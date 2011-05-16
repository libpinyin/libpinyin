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
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */



#include <errno.h>
#include <locale.h>
#include "pinyin.h"
#include "k_mixture_model.h"

static guint32 g_prune_k = 3;
static parameter_t g_prune_poss = 0.99;

void print_help(){
    printf("Usage: prune_k_mixture_model -k <INT> --CDF <FLOAT>  <FILENAME>\n");
}

bool prune_k_mixture_model(KMixtureModelMagicHeader * magic_header,
                           KMixtureModelSingleGram * & bigram){
    bool success;

    FlexibleBigramPhraseArray array = g_array_new(FALSE, FALSE, sizeof(KMixtureModelArrayItemWithToken));
    bigram->retrieve_all(array);
    for ( size_t i = 0; i < array->len; ++i) {
        KMixtureModelArrayItemWithToken * item = &g_array_index(array, KMixtureModelArrayItemWithToken, i);
        phrase_token_t token = item->m_token;
        parameter_t remained_poss = 1;
        for ( size_t k = 0; k < g_prune_k; ++k){
            remained_poss -= compute_Pr_G_3_with_count
                (k, magic_header->m_N, item->m_item.m_WC,
                 magic_header->m_N - item->m_item.m_N_n_0,
                 item->m_item.m_n_1);
        }

        assert(remained_poss >= 0);
        if ( remained_poss < g_prune_poss ) {
            /* prune this word or phrase. */
            KMixtureModelArrayItem removed_item;
            bigram->remove_array_item(token, removed_item);
            assert( memcmp(&removed_item, &(item->m_item),
                           sizeof(KMixtureModelArrayItem)) == 0 );
            KMixtureModelArrayHeader header;
            bigram->get_array_header(header);
            guint32 removed_count = removed_item.m_WC;
            header.m_WC -= removed_count;
            bigram->set_array_header(header);
            magic_header->m_WC -= removed_count;
        }
    }

    KMixtureModelArrayHeader header;
    bigram->get_array_header(header);

    if ( 0 == header.m_WC ){
        delete bigram;
        bigram = NULL;
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

    for ( size_t i; i < items->len; ++i ){
        phrase_token_t * token = &g_array_index(items, phrase_token_t, i);
        KMixtureModelSingleGram * single_gram = NULL;
        bigram.load(*token, single_gram);

        prune_k_mixture_model(&magic_header, single_gram);

        if ( NULL == single_gram )
            bigram.remove(*token);
        else bigram.store(*token, single_gram);
    }

    bigram.set_magic_header(magic_header);
    return 0;
}
