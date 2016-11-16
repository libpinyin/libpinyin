/* 
 *  libpinyin
 *  Library to deal with pinyin.
 *  
 *  Copyright (C) 2013 Peng Wu <alexepico@gmail.com>
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
#include <locale.h>
#include <string.h>
#include <glib.h>
#include "pinyin_internal.h"
#include "utils_helper.h"


void print_help(){
    printf("Usage: mergeseq [-o outputfile] [inputfile]\n");
}


static gchar * outputfile = NULL;

static GOptionEntry entries[] =
{
    {"outputfile", 'o', 0, G_OPTION_ARG_FILENAME, &outputfile, "output", "filename"},
    {NULL}
};


/* data structure definition. */
typedef struct{
    phrase_token_t m_token;
    gint m_token_len;
} TokenInfo;


/* GArray of ucs4 characters. */
typedef GArray * UnicodeCharVector;
/* GArray of TokenInfo. */
typedef GArray * TokenInfoVector;

gint calculate_sequence_length(TokenInfoVector tokeninfos) {
    gint len = 0;

    size_t i = 0;
    for (i = 0; i < tokeninfos->len; ++i) {
        TokenInfo * token_info = &g_array_index(tokeninfos, TokenInfo, i);
        len += token_info->m_token_len;
    }

    return len;
}

/* if merge sequence found, merge and output it,
 *   if not, just output the first token;
 * pop the first token or sequence.
 */
bool merge_sequence(FacadePhraseTable3 * phrase_table,
                    FacadePhraseIndex * phrase_index,
                    UnicodeCharVector unichars,
                    TokenInfoVector tokeninfos) {
    assert(tokeninfos->len > 0);

    bool found = false;
    TokenInfo * token_info = NULL;
    phrase_token_t token = null_token;

    ucs4_t * ucs4_str = (ucs4_t *) unichars->data;

    PhraseTokens tokens;
    memset(tokens, 0, sizeof(PhraseTokens));
    phrase_index->prepare_tokens(tokens);

    /* search the merge sequence. */
    size_t index = tokeninfos->len;
    gint seq_len = calculate_sequence_length(tokeninfos);
    while (seq_len > 0) {
        /* do phrase table search. */
        int retval = phrase_table->search(seq_len, ucs4_str, tokens);

        if (retval & SEARCH_OK) {
            int num = get_first_token(tokens, token);
            found = true;
            break;
        }

        --index;
        token_info = &g_array_index(tokeninfos, TokenInfo, index);
        seq_len -= token_info->m_token_len;
    }

    phrase_index->destroy_tokens(tokens);

    /* push the merged sequence back. */
    if (found) {
        /* pop up the origin sequence. */
        g_array_remove_range(tokeninfos, 0, index);

        TokenInfo info;
        info.m_token = token;
        info.m_token_len = seq_len;
        g_array_prepend_val(tokeninfos, info);
    }

    return found;
}

bool pop_first_token(UnicodeCharVector unichars,
                     TokenInfoVector tokeninfos,
                     FILE * output) {
    ucs4_t * ucs4_str = (ucs4_t *) unichars->data;

    /* pop it. */
    TokenInfo * token_info = &g_array_index(tokeninfos, TokenInfo, 0);
    phrase_token_t token = token_info->m_token;
    gint token_len = token_info->m_token_len;

    glong read = 0;
    gchar * utf8_str = g_ucs4_to_utf8(ucs4_str, token_len, &read, NULL, NULL);
    assert(read == token_len);
    fprintf(output, "%d %s\n", token, utf8_str);
    g_free(utf8_str);

    g_array_remove_range(unichars, 0, token_len);
    g_array_remove_index(tokeninfos, 0);

    return true;
}

bool feed_line(FacadePhraseTable3 * phrase_table,
               FacadePhraseIndex * phrase_index,
               UnicodeCharVector unichars,
               TokenInfoVector tokeninfos,
               const char * linebuf,
               FILE * output) {

    TAGLIB_PARSE_SEGMENTED_LINE(phrase_index, token, linebuf);

    if (null_token == token) {
        /* empty the queue. */
        while (0 != tokeninfos->len) {
            merge_sequence(phrase_table, phrase_index, unichars, tokeninfos);
            pop_first_token(unichars, tokeninfos, output);
        }

        assert(0 == unichars->len);
        assert(0 == tokeninfos->len);

        /* restore the null token line. */
        fprintf(output, "%s\n", linebuf);

        return false;
    }

    PhraseItem item;
    phrase_index->get_phrase_item(token, item);
    gint len = item.get_phrase_length();

    TokenInfo info;
    info.m_token = token;
    info.m_token_len = len;
    g_array_append_val(tokeninfos, info);

    ucs4_t buffer[MAX_PHRASE_LENGTH];
    item.get_phrase_string(buffer);
    g_array_append_vals(unichars, buffer, len);

    /* probe merge sequence. */
    len = calculate_sequence_length(tokeninfos);
    while (len >= MAX_PHRASE_LENGTH) {
        merge_sequence(phrase_table, phrase_index, unichars, tokeninfos);
        pop_first_token(unichars, tokeninfos, output);
        len = calculate_sequence_length(tokeninfos);
    }

    return true;
}


int main(int argc, char * argv[]){
    FILE * input = stdin;
    FILE * output = stdout;

    setlocale(LC_ALL, "");

    GError * error = NULL;
    GOptionContext * context;

    context = g_option_context_new("- merge word sequence");
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

    GArray * unichars = g_array_new(TRUE, TRUE, sizeof(ucs4_t));
    GArray * tokeninfos = g_array_new(TRUE, TRUE, sizeof(TokenInfo));

    char * linebuf = NULL; size_t size = 0; ssize_t read;
    while( (read = getline(&linebuf, &size, input)) != -1 ){
        if ( '\n' ==  linebuf[strlen(linebuf) - 1] ) {
            linebuf[strlen(linebuf) - 1] = '\0';
        }

        if (0 == strlen(linebuf))
            continue;

        feed_line(&phrase_table, &phrase_index,
                  unichars, tokeninfos,
                  linebuf, output);
    }

    /* append one null token for EOF. */
    feed_line(&phrase_table, &phrase_index,
              unichars, tokeninfos,
              "0 ", output);

    g_array_free(unichars, TRUE);
    g_array_free(tokeninfos, TRUE);
    free(linebuf);
    fclose(input);
    fclose(output);
    return 0;
}
