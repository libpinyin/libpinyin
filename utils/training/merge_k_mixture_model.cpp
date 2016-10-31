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

void print_help(){
    printf("Usage: merge_k_mixture_model [--result-file <RESULT_FILENAME>]\n");
    printf("                             {<SOURCE_FILENAME>}+\n");
}

static const gchar * result_filename = NULL;

static GOptionEntry entries[] =
{
    {"result-file", 0, 0, G_OPTION_ARG_FILENAME, &result_filename, "merged result file", NULL},
    {NULL}
};

static bool merge_two_phrase_array( /* in */  FlexibleBigramPhraseArray first,
                             /* in */  FlexibleBigramPhraseArray second,
                             /* out */ FlexibleBigramPhraseArray & merged ){
    /* avoid to do empty merge. */
    assert( NULL != first && NULL != second && NULL != merged );

    /* merge two arrays. */
    guint first_index, second_index = first_index = 0;
    KMixtureModelArrayItemWithToken * first_item,
        * second_item = first_item = NULL;
    while ( first_index < first->len && second_index < second->len ){
        first_item = &g_array_index(first, KMixtureModelArrayItemWithToken,
                                    first_index);
        second_item = &g_array_index(second, KMixtureModelArrayItemWithToken,
                                     second_index);
        if ( first_item->m_token > second_item->m_token ) {
            g_array_append_val(merged, *second_item);
            second_index ++;
        } else if ( first_item->m_token < second_item->m_token ) {
            g_array_append_val(merged, *first_item);
            first_index ++;
        } else /* first_item->m_token == second_item->m_token */ {
            KMixtureModelArrayItemWithToken merged_item;
            memset(&merged_item, 0, sizeof(KMixtureModelArrayItemWithToken));
            merged_item.m_token = first_item->m_token;/* same as second_item */
            merged_item.m_item.m_WC = first_item->m_item.m_WC +
                second_item->m_item.m_WC;
            /* merged_item.m_item.m_T = first_item->m_item.m_T +
                   second_item->m_item.m_T; */
            merged_item.m_item.m_N_n_0 = first_item->m_item.m_N_n_0 +
                second_item->m_item.m_N_n_0;
            merged_item.m_item.m_n_1 = first_item->m_item.m_n_1 +
                second_item->m_item.m_n_1;
            merged_item.m_item.m_Mr = std_lite::max(first_item->m_item.m_Mr,
                                                    second_item->m_item.m_Mr);
            g_array_append_val(merged, merged_item);
            first_index ++; second_index ++;
        }
    }

    /* add remained items. */
    while ( first_index < first->len ){
        first_item = &g_array_index(first, KMixtureModelArrayItemWithToken,
                                    first_index);
        g_array_append_val(merged, *first_item);
        first_index++;
    }

    while ( second_index < second->len ){
        second_item = &g_array_index(second, KMixtureModelArrayItemWithToken,
                                     second_index);
        g_array_append_val(merged, *second_item);
        second_index++;
    }

    return true;
}

static bool merge_magic_header( /* in & out */ KMixtureModelBigram * target,
                                /* in */ KMixtureModelBigram * new_one ){

    KMixtureModelMagicHeader target_magic_header;
    KMixtureModelMagicHeader new_magic_header;
    KMixtureModelMagicHeader merged_magic_header;

    memset(&merged_magic_header, 0, sizeof(KMixtureModelMagicHeader));
    if (!target->get_magic_header(target_magic_header)) {
        memset(&target_magic_header, 0, sizeof(KMixtureModelMagicHeader));
    }
    assert(new_one->get_magic_header(new_magic_header));
    if ( target_magic_header.m_WC + new_magic_header.m_WC <
         std_lite::max( target_magic_header.m_WC, new_magic_header.m_WC ) ){
        fprintf(stderr, "the m_WC integer in magic header overflows.\n");
        return false;
    }
    if ( target_magic_header.m_total_freq + new_magic_header.m_total_freq <
         std_lite::max( target_magic_header.m_total_freq,
                        new_magic_header.m_total_freq ) ){
        fprintf(stderr, "the m_total_freq in magic header overflows.\n");
        return false;
    }

    merged_magic_header.m_WC = target_magic_header.m_WC +
        new_magic_header.m_WC;
    merged_magic_header.m_N = target_magic_header.m_N +
        new_magic_header.m_N;
    merged_magic_header.m_total_freq = target_magic_header.m_total_freq +
        new_magic_header.m_total_freq;

    assert(target->set_magic_header(merged_magic_header));
    return true;
}

static bool merge_array_items( /* in & out */ KMixtureModelBigram * target,
                               /* in */ KMixtureModelBigram * new_one ){

    GArray * new_items = g_array_new(FALSE, FALSE, sizeof(phrase_token_t));
    new_one->get_all_items(new_items);

    for ( size_t i = 0; i < new_items->len; ++i ){
        phrase_token_t * token = &g_array_index(new_items, phrase_token_t, i);
        KMixtureModelSingleGram * target_single_gram = NULL;
        KMixtureModelSingleGram * new_single_gram = NULL;

        assert(new_one->load(*token, new_single_gram));
        bool exists_in_target = target->load(*token, target_single_gram);
        if ( !exists_in_target ){
            target->store(*token, new_single_gram);
            delete new_single_gram;
            continue;
        }

        /* word count in array header in parallel with array items */
        KMixtureModelArrayHeader target_array_header;
        KMixtureModelArrayHeader new_array_header;
        KMixtureModelArrayHeader merged_array_header;

        assert(new_one->get_array_header(*token, new_array_header));
        assert(target->get_array_header(*token, target_array_header));
        memset(&merged_array_header, 0, sizeof(KMixtureModelArrayHeader));

        merged_array_header.m_WC = target_array_header.m_WC +
            new_array_header.m_WC;
        merged_array_header.m_freq = target_array_header.m_freq +
            new_array_header.m_freq;
        /* end of word count in array header computing. */

        assert(NULL != target_single_gram);
        KMixtureModelSingleGram * merged_single_gram =
            new KMixtureModelSingleGram;

        FlexibleBigramPhraseArray target_array =
            g_array_new(FALSE, FALSE, sizeof(KMixtureModelArrayItemWithToken));
        target_single_gram->retrieve_all(target_array);

        FlexibleBigramPhraseArray new_array =
            g_array_new(FALSE, FALSE, sizeof(KMixtureModelArrayItemWithToken));
        new_single_gram->retrieve_all(new_array);
        FlexibleBigramPhraseArray merged_array =
            g_array_new(FALSE, FALSE, sizeof(KMixtureModelArrayItemWithToken));

        assert(merge_two_phrase_array(target_array, new_array, merged_array));

        g_array_free(target_array, TRUE);
        g_array_free(new_array, TRUE);
        delete target_single_gram; delete new_single_gram;

        for ( size_t m = 0; m < merged_array->len; ++m ){
            KMixtureModelArrayItemWithToken * item =
                &g_array_index(merged_array,
                               KMixtureModelArrayItemWithToken, m);
            merged_single_gram->insert_array_item(item->m_token, item->m_item);
        }

        assert(merged_single_gram->set_array_header(merged_array_header));
        assert(target->store(*token, merged_single_gram));
        delete merged_single_gram;
        g_array_free(merged_array, TRUE);
    }

    g_array_free(new_items, TRUE);
    return true;
}

bool merge_two_k_mixture_model( /* in & out */ KMixtureModelBigram * target,
                                /* in */ KMixtureModelBigram * new_one ){
    assert(NULL != target);
    assert(NULL != new_one);
    return merge_array_items(target, new_one) &&
        merge_magic_header(target, new_one);
}

int main(int argc, char * argv[]){
    int i = 1;

    setlocale(LC_ALL, "");

    GError * error = NULL;
    GOptionContext * context;

    context = g_option_context_new("- merge k mixture model");
    g_option_context_add_main_entries(context, entries, NULL);
    if (!g_option_context_parse(context, &argc, &argv, &error)) {
        g_print("option parsing failed:%s\n", error->message);
        exit(EINVAL);
    }

    KMixtureModelBigram target(K_MIXTURE_MODEL_MAGIC_NUMBER);
    target.attach(result_filename, ATTACH_READWRITE|ATTACH_CREATE);

    while (i < argc){
        const char * new_filename = argv[i];
        KMixtureModelBigram new_one(K_MIXTURE_MODEL_MAGIC_NUMBER);
        new_one.attach(new_filename, ATTACH_READONLY);
        if ( !merge_two_k_mixture_model(&target, &new_one) )
            exit(EOVERFLOW);
        ++i;
    }

    return 0;
}
