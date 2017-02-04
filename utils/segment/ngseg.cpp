/* 
 *  libpinyin
 *  Library to deal with pinyin.
 *  
 *  Copyright (C) 2010 Peng Wu
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <locale.h>
#include "pinyin_internal.h"
#include "utils_helper.h"


void print_help(){
    printf("Usage: ngseg [--generate-extra-enter]  [-o outputfile] [inputfile]\n");
}


static gboolean gen_extra_enter = FALSE;
static gchar * outputfile = NULL;

static GOptionEntry entries[] =
{
    {"outputfile", 'o', 0, G_OPTION_ARG_FILENAME, &outputfile, "output", "filename"},
    {"generate-extra-enter", 0, 0, G_OPTION_ARG_NONE, &gen_extra_enter, "generate ", NULL},
    {NULL}
};


/* n-gram based sentence segment. */

/* Note:
 * Currently libpinyin supports ucs4 characters.
 * This is a pre-processor tool for raw corpus,
 * and skips non-Chinese characters.
 */

/* TODO:
 * Try to add punctuation mark and english support,
 * such as ',', '.', '?', '!', <english>, and other punctuations.
 */

enum CONTEXT_STATE{
    CONTEXT_INIT,
    CONTEXT_SEGMENTABLE,
    CONTEXT_UNKNOWN
};

bool deal_with_segmentable(PhraseLookup * phrase_lookup,
                           GArray * current_ucs4,
                           FILE * output){
    char * result_string = NULL;
    MatchResult result = g_array_new(FALSE, FALSE, sizeof(phrase_token_t));
    phrase_lookup->get_best_match(current_ucs4->len,
                                  (ucs4_t *) current_ucs4->data, result);

    phrase_lookup->convert_to_utf8(result, result_string);

    if (result_string) {
        fprintf(output, "%s\n", result_string);
    } else {
        char * tmp_string = g_ucs4_to_utf8
            ( (ucs4_t *) current_ucs4->data, current_ucs4->len,
              NULL, NULL, NULL);
        fprintf(stderr, "Un-segmentable sentence encountered:%s\n",
                tmp_string);
        g_array_free(result, TRUE);
        return false;
    }
    g_array_free(result, TRUE);
    g_free(result_string);
    return true;
}

bool deal_with_unknown(GArray * current_ucs4, FILE * output){
    char * result_string = g_ucs4_to_utf8
        ( (ucs4_t *) current_ucs4->data, current_ucs4->len,
          NULL, NULL, NULL);
    fprintf(output, "%d %s\n", null_token, result_string);
    g_free(result_string);
    return true;
}


int main(int argc, char * argv[]){
    FILE * input = stdin;
    FILE * output = stdout;

    setlocale(LC_ALL, "");

    GError * error = NULL;
    GOptionContext * context;

    context = g_option_context_new("- n-gram segment");
    g_option_context_add_main_entries(context, entries, NULL);
    if (!g_option_context_parse(context, &argc, &argv, &error)) {
        g_print("option parsing failed:%s\n", error->message);
        exit(EINVAL);
    }

    if (outputfile) {
        output = fopen(outputfile, "w");
        if (NULL == output) {
            perror("open file failed");
            exit(EINVAL);
        }
    }

    if (argc > 2) {
        fprintf(stderr, "too many arguments.\n");
        exit(EINVAL);
    }

    if (2 == argc) {
        input = fopen(argv[1], "r");
        if (NULL == input) {
            perror("open file failed");
            exit(EINVAL);
        }
    }

    SystemTableInfo2 system_table_info;

    bool retval = system_table_info.load(SYSTEM_TABLE_INFO);
    if (!retval) {
        fprintf(stderr, "load table.conf failed.\n");
        exit(ENOENT);
    }

    /* init phrase table */
    FacadePhraseTable3 phrase_table;
    phrase_table.load(SYSTEM_PHRASE_INDEX, NULL);

    /* init phrase index */
    FacadePhraseIndex phrase_index;

    const pinyin_table_info_t * phrase_files =
        system_table_info.get_default_tables();

    if (!load_phrase_index(phrase_files, &phrase_index))
        exit(ENOENT);

    /* init bi-gram */
    Bigram system_bigram;
    system_bigram.attach(SYSTEM_BIGRAM, ATTACH_READONLY);
    Bigram user_bigram;

    gfloat lambda = system_table_info.get_lambda();

    /* init phrase lookup */
    PhraseLookup phrase_lookup(lambda,
                               &phrase_table, &phrase_index,
                               &system_bigram, &user_bigram);


    CONTEXT_STATE state, next_state;
    GArray * current_ucs4 = g_array_new(TRUE, TRUE, sizeof(ucs4_t));

    PhraseTokens tokens;
    memset(tokens, 0, sizeof(PhraseTokens));
    phrase_index.prepare_tokens(tokens);

    /* split the sentence */
    char * linebuf = NULL; size_t size = 0; ssize_t read;
    while( (read = getline(&linebuf, &size, input)) != -1 ){
        if ( '\n' ==  linebuf[strlen(linebuf) - 1] ) {
            linebuf[strlen(linebuf) - 1] = '\0';
        }

        /* check non-ucs4 characters */
        const glong num_of_chars = g_utf8_strlen(linebuf, -1);
        glong len = 0;
        ucs4_t * sentence = g_utf8_to_ucs4(linebuf, -1, NULL, &len, NULL);
        if ( len != num_of_chars ) {
            fprintf(stderr, "non-ucs4 characters encountered:%s.\n", linebuf);
            fprintf(output, "%d \n", null_token);
            continue;
        }

        /* only new-line persists. */
        if ( 0  == num_of_chars ) {
            fprintf(output, "%d \n", null_token);
            continue;
        }

        state = CONTEXT_INIT;
        int result = phrase_table.search( 1, sentence, tokens);
        g_array_append_val( current_ucs4, sentence[0]);
        if ( result & SEARCH_OK )
            state = CONTEXT_SEGMENTABLE;
        else
            state = CONTEXT_UNKNOWN;

        for ( int i = 1; i < num_of_chars; ++i) {
            int result = phrase_table.search( 1, sentence + i, tokens);
            if ( result & SEARCH_OK )
                next_state = CONTEXT_SEGMENTABLE;
            else
                next_state = CONTEXT_UNKNOWN;

            if ( state == next_state ){
                g_array_append_val(current_ucs4, sentence[i]);
                continue;
            }

            assert ( state != next_state );
            if ( state == CONTEXT_SEGMENTABLE )
                deal_with_segmentable(&phrase_lookup, current_ucs4, output);

            if ( state == CONTEXT_UNKNOWN )
                deal_with_unknown(current_ucs4, output);

            /* save the current character */
            g_array_set_size(current_ucs4, 0);
            g_array_append_val(current_ucs4, sentence[i]);
            state = next_state;
        }

        if ( current_ucs4->len ) {
            /* this seems always true. */
            if ( state == CONTEXT_SEGMENTABLE )
                deal_with_segmentable(&phrase_lookup, current_ucs4, output);

            if ( state == CONTEXT_UNKNOWN )
                deal_with_unknown(current_ucs4, output);
            g_array_set_size(current_ucs4, 0);
        }

        /* print extra enter */
        if ( gen_extra_enter )
            fprintf(output, "%d \n", null_token);

        g_free(sentence);
    }
    phrase_index.destroy_tokens(tokens);

    /* print enter at file tail */
    fprintf(output, "%d \n", null_token);
    g_array_free(current_ucs4, TRUE);
    free(linebuf);
    fclose(input);
    fclose(output);
    return 0;
}
