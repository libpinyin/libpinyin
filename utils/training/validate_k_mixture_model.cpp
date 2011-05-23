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

#include "pinyin.h"
#include "k_mixture_model.h"

void print_help(){
    printf("Usage: validate_k_mixture_model <FILENAME>\n");
}

bool validate_unigram(KMixtureModelBigram * bigram){
    KMixtureModelMagicHeader magic_header;
    if( !bigram->get_magic_header(magic_header) ){
        fprintf(stderr, "no magic header in k mixture model.\n");
        return false;
    }

    guint32 expected_sum = magic_header.m_WC;
    if ( 0 == expected_sum ){
        fprintf(stderr, "word count in magic header is unexpected zero.\n");
        return false;
    }
    
    GArray * items = g_array_new(FALSE, FALSE, sizeof(phrase_token_t));
    bigram->get_all_items(items);

    guint32 sum = 0;
    for (size_t i = 0; i < items->len; ++i) {
        phrase_token_t * token = &g_array_index(items, phrase_token_t, i);
        KMixtureModelArrayHeader array_header;
        assert(bigram->get_array_header(*token, array_header));
        sum += array_header.m_WC;
    }

    if ( sum != expected_sum ){
        fprintf(stderr, "word count in magic header:%d\n", expected_sum);
        fprintf(stderr, "sum of word count in array headers:%d\n", sum);
        fprintf(stderr, "the sum differs from word count.\n");
        return false;
    }

    g_array_free(items, TRUE);
    return true;
}

bool validate_bigram(KMixtureModelBigram * bigram){
    bool result = true;

    GArray * items = g_array_new(FALSE, FALSE, sizeof(phrase_token_t));
    bigram->get_all_items(items);

    for (size_t i = 0; i < items->len; ++i) {
        phrase_token_t * token = &g_array_index(items, phrase_token_t, i);
        KMixtureModelSingleGram * single_gram = NULL;
        assert(bigram->load(*token, single_gram));
        FlexibleBigramPhraseArray array = g_array_new
            (FALSE, FALSE, sizeof(KMixtureModelArrayItemWithToken));
        single_gram->retrieve_all(array);

        KMixtureModelArrayHeader array_header;
        assert(single_gram->get_array_header(array_header));

        guint32 expected_sum = array_header.m_WC;
        if ( 0 == expected_sum ){
            fprintf(stderr, "in the array header of token %d:\n", *token);
            fprintf(stderr, "word count is unexpected zero.\n");
            result = false;
        }

        guint32 sum = 0;
        for (size_t m = 0; m< array->len; ++m){
            KMixtureModelArrayItemWithToken * item = &g_array_index(array, KMixtureModelArrayItemWithToken, m);

            sum += item->m_item.m_WC;
        }

        if ( sum != expected_sum ){
            fprintf(stderr, "word count in array header:%d\n", expected_sum);
            fprintf(stderr, "sum of word count in array items:%d\n", sum);
            fprintf(stderr, "the sum differs from word count.\n");
            result = false;
        }

        g_array_free(array, TRUE);
    }

    g_array_free(items, TRUE);
    return result;
}

int main(int argc, char * argv[]){
    int i = 1;
    const char * k_mixture_model_filename = NULL;

    while ( i < argc ){
        if ( strcmp ("--help", argv[i]) == 0 ){
            print_help();
            exit(0);
        } else {
            k_mixture_model_filename = argv[i];
        }
        ++i;
    }

    KMixtureModelBigram bigram(K_MIXTURE_MODEL_MAGIC_NUMBER);
    bigram.attach(k_mixture_model_filename, ATTACH_READONLY);

    if (!validate_unigram(&bigram)) {
        fprintf(stderr, "k mixture model validation failed.\n");
        exit(ENODATA);
    }

    if (!validate_bigram(&bigram)) {
        fprintf(stderr, "k mixture model validation failed.\n");
        exit(ENODATA);
    }

    return 0;
}
