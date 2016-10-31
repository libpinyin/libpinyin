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

#include "pinyin_internal.h"
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

    guint32 expected_word_count = magic_header.m_WC;
    if ( 0 == expected_word_count ){
        fprintf(stderr, "word count in magic header is unexpected zero.\n");
        return false;
    }
    guint32 expected_total_freq = magic_header.m_total_freq;
    if ( 0 == expected_total_freq ){
        fprintf(stderr, "total freq in magic header is unexpected zero.\n");
        return false;
    }

    if ( expected_word_count != expected_total_freq ){
        fprintf(stderr, "the word count doesn't match the total freq.\n");
        return false;
    }
    
    GArray * items = g_array_new(FALSE, FALSE, sizeof(phrase_token_t));
    bigram->get_all_items(items);

    guint32 word_count = 0; guint32 total_freq = 0;
    for (size_t i = 0; i < items->len; ++i) {
        phrase_token_t * token = &g_array_index(items, phrase_token_t, i);
        KMixtureModelArrayHeader array_header;
        assert(bigram->get_array_header(*token, array_header));
        word_count += array_header.m_WC;
        total_freq += array_header.m_freq;
    }

    if ( word_count != expected_word_count ){
        fprintf(stderr, "word count in magic header:%d\n",
                expected_word_count);
        fprintf(stderr, "sum of word count in array headers:%d\n", word_count);
        fprintf(stderr, "the sum differs from word count.\n");
        return false;
    }
    if ( total_freq != expected_total_freq ){
        fprintf(stderr, "total freq in magic header:%d\n",
                expected_total_freq);
        fprintf(stderr, "sum of freqs in array headers:%d\n", total_freq);
        fprintf(stderr, "the total freq differs from sum of freqs.\n");
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
        guint32 freq = array_header.m_freq;
        if ( 0 == expected_sum ){
            if ( 0 != array->len ){
                fprintf(stderr, "in the array header of token %d:\n", *token);
                fprintf(stderr, "word count is zero but has array items.\n");
                result = false;
            }
            if ( 0 != freq ){
                delete single_gram;
                continue;
            } else {
                fprintf(stderr, "in the array header of token %d:\n", *token);
                fprintf(stderr, "both word count and freq are "
                        "unexpected zero.\n");
                result = false;
            }
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
        delete single_gram;
    }

    g_array_free(items, TRUE);
    return result;
}

int main(int argc, char * argv[]){

    GError * error = NULL;
    GOptionContext * context;

    context = g_option_context_new("- validate k mixture model");
    if (!g_option_context_parse(context, &argc, &argv, &error)) {
        g_print("option parsing failed:%s\n", error->message);
        exit(EINVAL);
    }

    if (2 != argc) {
        fprintf(stderr, "wrong arguments.\n");
        exit(EINVAL);
    }

    const char * k_mixture_model_filename = argv[1];

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
