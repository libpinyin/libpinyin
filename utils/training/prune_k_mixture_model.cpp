/* 
 *  libpinyin
 *  Library to deal with pinyin.
 *  
 *  Copyright (C) 2011 Peng Wu <alexepico@gmail.com>
 *  
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 * 
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *  
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */



#include <errno.h>
#include <locale.h>
#include "pinyin_internal.h"
#include "k_mixture_model.h"

static guint32 g_prune_k = 3;
static parameter_t g_prune_poss = 0.99;

void print_help(){
    printf("Usage: prune_k_mixture_model -k <INT> --CDF <FLOAT>  <FILENAME>\n");
}

bool prune_k_mixture_model(KMixtureModelMagicHeader * magic_header,
                           KMixtureModelSingleGram * & bigram,
                           FlexibleBigramPhraseArray removed_array){
    bool success;

    FlexibleBigramPhraseArray array = g_array_new(FALSE, FALSE, sizeof(KMixtureModelArrayItemWithToken));
    bigram->retrieve_all(array);

    for ( size_t i = 0; i < array->len; ++i) {
        KMixtureModelArrayItemWithToken * item = &g_array_index(array, KMixtureModelArrayItemWithToken, i);
        phrase_token_t token = item->m_token;
        parameter_t remained_poss = 1; parameter_t one_poss = 0;
        bool errors = false;
        for ( size_t k = 0; k < g_prune_k; ++k){
            one_poss = compute_Pr_G_3_with_count
                (k, magic_header->m_N, item->m_item.m_WC,
                 magic_header->m_N - item->m_item.m_N_n_0,
                 item->m_item.m_n_1);
            if ( !(0 <= one_poss && one_poss <= 1) )
                errors = true;
            remained_poss -= one_poss;
        }

        if ( fabs(remained_poss) < DBL_EPSILON )
            remained_poss = 0.;

        /* some wrong possibility. */
        if ( errors || !(0 <= remained_poss && remained_poss <= 1) ) {
            fprintf(stderr, "some wrong possibility is encountered:%f.\n",
                    remained_poss);
            fprintf(stderr, "k:%d N:%d WC:%d n_0:%d n_1:%d\n",
                    g_prune_k, magic_header->m_N, item->m_item.m_WC,
                    magic_header->m_N - item->m_item.m_N_n_0,
                    item->m_item.m_n_1);
            exit(EDOM);
        }

        if ( remained_poss < g_prune_poss ) {
            /* prune this word or phrase. */
            KMixtureModelArrayItem removed_item;
            bigram->remove_array_item(token, removed_item);
            assert( memcmp(&removed_item, &(item->m_item),
                           sizeof(KMixtureModelArrayItem)) == 0 );

            KMixtureModelArrayItemWithToken removed_item_with_token;
            removed_item_with_token.m_token = token;
            removed_item_with_token.m_item = removed_item;
            g_array_append_val(removed_array, removed_item_with_token);

            KMixtureModelArrayHeader array_header;
            bigram->get_array_header(array_header);
            guint32 removed_count = removed_item.m_WC;
            array_header.m_WC -= removed_count;
            bigram->set_array_header(array_header);
            magic_header->m_WC -= removed_count;
            magic_header->m_total_freq -= removed_count;
        }
    }

    return true;
}

int main(int argc, char * argv[]){
    int i = 1;
    const char * bigram_filename = NULL;

    setlocale(LC_ALL, "");
    while ( i < argc ){
        if ( strcmp("--help", argv[i]) == 0 ){
            print_help();
            exit(0);
        } else if ( strcmp("-k", argv[i]) == 0 ){
            if ( ++i >= argc ){
                print_help();
                exit(EINVAL);
            }
            g_prune_k = atoi(argv[i]);
        } else if ( strcmp("--CDF", argv[i]) == 0 ){
            if ( ++i >= argc ){
                print_help();
                exit(EINVAL);
            }
            g_prune_poss = atof(argv[i]);
        } else {
            bigram_filename = argv[i];
        }
        ++i;
    }

    /* TODO: magic header signature check here. */
    KMixtureModelBigram bigram(K_MIXTURE_MODEL_MAGIC_NUMBER);
    bigram.attach(bigram_filename, ATTACH_READWRITE);

    KMixtureModelMagicHeader magic_header;
    bigram.get_magic_header(magic_header);
    GArray * items = g_array_new(FALSE, FALSE, sizeof(phrase_token_t));
    bigram.get_all_items(items);

    /* print prune progress */
    size_t progress = 0; size_t onestep = items->len / 20;
    for ( size_t i = 0; i < items->len; ++i ){
        if ( progress >= onestep ) {
            progress = 0; fprintf(stderr, "*");
        }
        progress ++;

        phrase_token_t * token = &g_array_index(items, phrase_token_t, i);
        KMixtureModelSingleGram * single_gram = NULL;
        bigram.load(*token, single_gram);

        FlexibleBigramPhraseArray removed_array = g_array_new(FALSE, FALSE, sizeof(KMixtureModelArrayItemWithToken));

        prune_k_mixture_model(&magic_header, single_gram, removed_array);
        bigram.store(*token, single_gram);

        delete single_gram;

        /* post processing for unigram reduce */
        for (size_t m = 0; m < removed_array->len; ++m ){
            KMixtureModelArrayItemWithToken * item =
                &g_array_index(removed_array,
                              KMixtureModelArrayItemWithToken, m);
            KMixtureModelArrayHeader array_header;
            assert(bigram.get_array_header(item->m_token, array_header));
            array_header.m_freq -= item->m_item.m_WC;
            assert(array_header.m_freq >= 0);
            assert(bigram.set_array_header(item->m_token, array_header));
        }

        g_array_free(removed_array, TRUE);
        removed_array = NULL;
    }

    fprintf(stderr, "\n");

    bigram.set_magic_header(magic_header);

    /* post processing clean up zero items */
    KMixtureModelArrayHeader array_header;
    for ( size_t i = 0; i < items->len; ++i ){
        phrase_token_t * token = &g_array_index(items, phrase_token_t, i);
        assert(bigram.get_array_header(*token, array_header));
        if ( 0 == array_header.m_WC && 0 == array_header.m_freq )
            assert(bigram.remove(*token));
    }

    g_array_free(items, TRUE);

    return 0;
}
