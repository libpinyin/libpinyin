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

#include <locale.h>
#include "pinyin_internal.h"
#include "k_mixture_model.h"

void print_help(){
    printf("Usage: estimate_k_mixture_model [--bigram-file <FILENAME>]\n");
    printf("                                [--deleted-bigram-file <FILENAME]\n");
}

parameter_t compute_interpolation(KMixtureModelSingleGram * deleted_bigram,
                                  KMixtureModelBigram * unigram,
                                  KMixtureModelSingleGram * bigram){
    bool success;
    parameter_t lambda = 0, next_lambda = 0.6;
    parameter_t epsilon = 0.001;

    KMixtureModelMagicHeader magic_header;
    assert(unigram->get_magic_header(magic_header));
    assert(0 != magic_header.m_total_freq);

    while (fabs(lambda - next_lambda) > epsilon){
        lambda = next_lambda;
        next_lambda = 0;
        parameter_t numerator = 0;
        parameter_t part_of_denominator = 0;

        FlexibleBigramPhraseArray array = g_array_new(FALSE, FALSE, sizeof(KMixtureModelArrayItemWithToken));
        deleted_bigram->retrieve_all(array);

        for ( size_t i = 0; i < array->len; ++i){
            KMixtureModelArrayItemWithToken * item = &g_array_index(array, KMixtureModelArrayItemWithToken, i);
            //get the phrase token
            phrase_token_t token = item->m_token;
            guint32 deleted_count = item->m_item.m_WC;

            {
                parameter_t elem_poss = 0;
                KMixtureModelArrayHeader array_header;
                KMixtureModelArrayItem array_item;
                if ( bigram && bigram->get_array_item(token, array_item) ){
                    assert(bigram->get_array_header(array_header));
                    assert(0 != array_header.m_WC);
                    elem_poss = array_item.m_WC / (parameter_t) array_header.m_WC;
                }
                numerator = lambda * elem_poss;
            }

            {
                parameter_t elem_poss = 0;
                KMixtureModelArrayHeader array_header;
                if (unigram->get_array_header(token, array_header)){
                    elem_poss = array_header.m_freq / (parameter_t) magic_header.m_total_freq;
                }
                part_of_denominator = (1 - lambda) * elem_poss;
            }
            if (0 == (numerator + part_of_denominator))
                continue;

            next_lambda += deleted_count * (numerator / (numerator + part_of_denominator));
        }
        KMixtureModelArrayHeader header;
        assert(deleted_bigram->get_array_header(header));
        assert(0 != header.m_WC);
        next_lambda /= header.m_WC;

        g_array_free(array, TRUE);
    }
    lambda = next_lambda;
    return lambda;
}

int main(int argc, char * argv[]){
    int i = 1;
    const char * bigram_filename = "k_mixture_model_ngram.db";
    const char * deleted_bigram_filename = "k_mixture_model_deleted_ngram.db";

    setlocale(LC_ALL, "");
    while ( i < argc ){
        if ( strcmp("--help", argv[i] ) == 0 ){
            print_help();
            exit(0);
        } else if ( strcmp("--bigram-file", argv[i]) == 0 ){
            if ( ++i >= argc ) {
                print_help();
                exit(EINVAL);
            }
            bigram_filename = argv[i];
        } else if ( strcmp("--deleted-bigram-file", argv[i]) == 0){
            if ( ++i >= argc ) {
                print_help();
                exit(EINVAL);
            }
            deleted_bigram_filename = argv[i];
        } else{
            print_help();
            exit(EINVAL);
        }
        ++i;
    }

    /* TODO: magic header signature check here. */
    KMixtureModelBigram unigram(K_MIXTURE_MODEL_MAGIC_NUMBER);
    unigram.attach(bigram_filename, ATTACH_READONLY);

    KMixtureModelBigram bigram(K_MIXTURE_MODEL_MAGIC_NUMBER);
    bigram.attach(bigram_filename, ATTACH_READONLY);

    KMixtureModelBigram deleted_bigram(K_MIXTURE_MODEL_MAGIC_NUMBER);
    deleted_bigram.attach(deleted_bigram_filename, ATTACH_READONLY);

    GArray * deleted_items = g_array_new(FALSE, FALSE, sizeof(phrase_token_t));
    deleted_bigram.get_all_items(deleted_items);

    parameter_t lambda_sum = 0;
    int lambda_count = 0;

    for( size_t i = 0; i < deleted_items->len; ++i ){
        phrase_token_t * token = &g_array_index(deleted_items, phrase_token_t, i);
        KMixtureModelSingleGram * single_gram = NULL;
        bigram.load(*token, single_gram);

        KMixtureModelSingleGram * deleted_single_gram = NULL;
        deleted_bigram.load(*token, deleted_single_gram);

        KMixtureModelArrayHeader array_header;
        if (single_gram)
            assert(single_gram->get_array_header(array_header));
        KMixtureModelArrayHeader deleted_array_header;
        assert(deleted_single_gram->get_array_header(deleted_array_header));

        if ( 0 != deleted_array_header.m_WC ) {
            parameter_t lambda = compute_interpolation(deleted_single_gram, &unigram, single_gram);

            printf("token:%d lambda:%f\n", *token, lambda);

            lambda_sum += lambda;
            lambda_count ++;
        }

        if (single_gram)
            delete single_gram;
        delete deleted_single_gram;
    }

    printf("average lambda:%f\n", (lambda_sum/lambda_count));
    g_array_free(deleted_items, TRUE);
    return 0;
}
