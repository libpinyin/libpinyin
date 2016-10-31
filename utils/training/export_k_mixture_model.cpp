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
#include "utils_helper.h"

static const gchar * k_mixture_model_filename = NULL;

static GOptionEntry entries[] =
{
    {"k-mixture-model-file", 0, 0, G_OPTION_ARG_FILENAME, &k_mixture_model_filename, "k mixture model file", NULL},
    {NULL}
};


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
        phrase_token_t token = g_array_index(items, phrase_token_t, i);
        KMixtureModelArrayHeader array_header;
        assert(bigram->get_array_header(token, array_header));
        char * phrase = taglib_token_to_string(phrase_index, token);
        if ( phrase )
            fprintf(output, "\\item %d %s count %d freq %d\n",
                    token, phrase, array_header.m_WC, array_header.m_freq);

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
        phrase_token_t token = g_array_index(items, phrase_token_t, i);
        KMixtureModelSingleGram * single_gram = NULL;
        assert(bigram->load(token, single_gram));
        FlexibleBigramPhraseArray array = g_array_new
            (FALSE, FALSE, sizeof(KMixtureModelArrayItemWithToken));
        single_gram->retrieve_all(array);

        for (size_t m = 0; m < array->len; ++m){
            KMixtureModelArrayItemWithToken * item = &g_array_index(array, KMixtureModelArrayItemWithToken, m);
            char * word1 = taglib_token_to_string(phrase_index, token);
            char * word2 = taglib_token_to_string(phrase_index, item->m_token);

            if (word1 && word2)
                fprintf(output, "\\item %d %s %d %s count %d T %d N_n_0 %d n_1 %d Mr %d\n",
                        token, word1, item->m_token, word2,
                        item->m_item.m_WC, item->m_item.m_WC,
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
    FILE * output = stdout;
    setlocale(LC_ALL, "");

    GError * error = NULL;
    GOptionContext * context;

    context = g_option_context_new("- export k mixture model");
    g_option_context_add_main_entries(context, entries, NULL);
    if (!g_option_context_parse(context, &argc, &argv, &error)) {
        g_print("option parsing failed:%s\n", error->message);
        exit(EINVAL);
    }

    SystemTableInfo2 system_table_info;

    bool retval = system_table_info.load(SYSTEM_TABLE_INFO);
    if (!retval) {
        fprintf(stderr, "load table.conf failed.\n");
        exit(ENOENT);
    }

    FacadePhraseIndex phrase_index;

    const pinyin_table_info_t * phrase_files =
        system_table_info.get_default_tables();

    if (!load_phrase_index(phrase_files, &phrase_index))
        exit(ENOENT);

    KMixtureModelBigram bigram(K_MIXTURE_MODEL_MAGIC_NUMBER);
    if (!bigram.attach(k_mixture_model_filename, ATTACH_READONLY)) {
        fprintf(stderr, "open %s failed.\n", k_mixture_model_filename);
        exit(ENOENT);
    }

    print_k_mixture_model_magic_header(output, &bigram);
    print_k_mixture_model_array_headers(output, &bigram, &phrase_index);
    print_k_mixture_model_array_items(output, &bigram, &phrase_index);

    end_data(output);

    return 0;
}
