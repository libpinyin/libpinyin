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

#include "pinyin_internal.h"
#include "k_mixture_model.h"

void print_help(){
    printf("Usage: export_k_mixture_model [--k-mixture-model-file <FILENAME>]\n");
}

bool print_k_mixture_model_magic_header(FILE * output,
                                        KMixtureModelBigram * bigram){
    KMixtureModelMagicHeader magic_header;
    if ( !bigram->get_magic_header(magic_header) ){
        fprintf(stderr, "no magic header in k mixture model.\n");
        exit(ENODATA);
    }
    fprintf(output, "\\data model \"k mixture model\" count %d N %d "
            "total_freq %d\n", magic_header.m_WC, magic_header.m_N,
            magic_header.m_total_freq);
    return true;
}

bool print_k_mixture_model_array_headers(FILE * output,
                                         KMixtureModelBigram * bigram,
                                         FacadePhraseIndex * phrase_index){
    fprintf(output, "\\1-gram\n");
    GArray * items = g_array_new(FALSE, FALSE, sizeof(phrase_token_t));
    bigram->get_all_items(items);

    for (size_t i = 0; i < items->len; ++i) {
        phrase_token_t * token = &g_array_index(items, phrase_token_t, i);
        KMixtureModelArrayHeader array_header;
        assert(bigram->get_array_header(*token, array_header));
        char * phrase = taglib_token_to_string(phrase_index, *token);
        if ( phrase )
            fprintf(output, "\\item %s count %d freq %d\n",
                    phrase, array_header.m_WC, array_header.m_freq);

        g_free(phrase);
    }
    return true;
}

bool print_k_mixture_model_array_items(FILE * output,
                                       KMixtureModelBigram * bigram,
                                       FacadePhraseIndex * phrase_index){
    fprintf(output, "\\2-gram\n");
    GArray * items = g_array_new(FALSE, FALSE, sizeof(phrase_token_t));
    bigram->get_all_items(items);

    for (size_t i = 0; i < items->len; ++i) {
        phrase_token_t * token = &g_array_index(items, phrase_token_t, i);
        KMixtureModelSingleGram * single_gram = NULL;
        assert(bigram->load(*token, single_gram));
        FlexibleBigramPhraseArray array = g_array_new
            (FALSE, FALSE, sizeof(KMixtureModelArrayItemWithToken));
        single_gram->retrieve_all(array);

        for (size_t m = 0; m < array->len; ++m){
            KMixtureModelArrayItemWithToken * item = &g_array_index(array, KMixtureModelArrayItemWithToken, m);
            char * word1 = taglib_token_to_string(phrase_index, *token);
            char * word2 = taglib_token_to_string(phrase_index, item->m_token);

            if (word1 && word2)
                fprintf(output, "\\item %s %s count %d T %d N_n_0 %d n_1 %d Mr %d\n",
                        word1, word2, item->m_item.m_WC, item->m_item.m_WC,
                        item->m_item.m_N_n_0, item->m_item.m_n_1,
                        item->m_item.m_Mr);

            g_free(word1); g_free(word2);
        }

        g_array_free(array, TRUE);
        delete single_gram;
    }

    g_array_free(items, TRUE);
    return true;
}

bool end_data(FILE * output){
    fprintf(output, "\\end\n");
    return true;
}

int main(int argc, char * argv[]){
    int i = 1;
    const char * k_mixture_model_filename = NULL;
    FILE * output = stdout;

    while ( i < argc ){
        if ( strcmp ("--help", argv[i]) == 0 ){
            print_help();
            exit(0);
        } else if ( strcmp ("--k-mixture-model-file", argv[i]) == 0 ){
            if ( ++i > argc ){
                print_help();
                exit(EINVAL);
            }
            k_mixture_model_filename = argv[i];
        } else {
            print_help();
            exit(EINVAL);
        }
        ++i;
    }

    FacadePhraseIndex phrase_index;

    //gb_char binary file
    MemoryChunk * chunk = new MemoryChunk;
    chunk->load("gb_char.bin");
    phrase_index.load(1, chunk);

    //gbk_char binary file
    chunk = new MemoryChunk;
    chunk->load("gbk_char.bin");
    phrase_index.load(2, chunk);

    KMixtureModelBigram bigram(K_MIXTURE_MODEL_MAGIC_NUMBER);
    bigram.attach(k_mixture_model_filename, ATTACH_READONLY);

    print_k_mixture_model_magic_header(output, &bigram);
    print_k_mixture_model_array_headers(output, &bigram, &phrase_index);
    print_k_mixture_model_array_items(output, &bigram, &phrase_index);

    end_data(output);

    return 0;
}
