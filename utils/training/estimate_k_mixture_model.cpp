/* 
 *  libpinyin
 *  Library to deal with pinyin.
 *  
 *  Copyright (C) 2011 Peng Wu <alexepico@gmail.com>
 *  
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <locale.h>
#include "pinyin_internal.h"
#include "k_mixture_model.h"

static const gchar * bigram_filename = "k_mixture_model_ngram.db";
static const gchar * deleted_bigram_filename = "k_mixture_model_deleted_ngram.db";

static GOptionEntry entries[] =
{
    {"bigram-file", 0, 0, G_OPTION_ARG_FILENAME, &bigram_filename, "the bigram file", NULL},
    {"deleted-bigram-file", 0, 0, G_OPTION_ARG_FILENAME, &deleted_bigram_filename, "the deleted bigram file", NULL},
    {NULL}
};


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
    setlocale(LC_ALL, "");

    GError * error = NULL;
    GOptionContext * context;

    context = g_option_context_new("- estimate k mixture model");
    g_option_context_add_main_entries(context, entries, NULL);
    if (!g_option_context_parse(context, &argc, &argv, &error)) {
        g_print("option parsing failed:%s\n", error->message);
        exit(EINVAL);
    }

    /* magic header signature check here. */
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
        bigram.load(*token, single_gram, true);

        KMixtureModelSingleGram * deleted_single_gram = NULL;
        deleted_bigram.load(*token, deleted_single_gram);

        KMixtureModelArrayHeader array_header;
        if (single_gram)
            assert(single_gram->get_array_header(array_header));
        KMixtureModelArrayHeader deleted_array_header;
        assert(deleted_single_gram->get_array_header(deleted_array_header));

        if ( 0 != deleted_array_header.m_WC ) {
            parameter_t lambda = compute_interpolation(deleted_single_gram, &bigram, single_gram);

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
