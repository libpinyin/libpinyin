/* 
 *  libpinyin
 *  Library to deal with pinyin.
 *  
 *  Copyright (C) 2010,2013 Peng Wu
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
#include <string.h>
#include <locale.h>
#include <glib.h>
#include "pinyin_internal.h"
#include "utils_helper.h"


void print_help(){
    printf("Usage: spseg [--generate-extra-enter] [-o outputfile] [inputfile]\n");
}

static gboolean gen_extra_enter = FALSE;
static gchar * outputfile = NULL;

static GOptionEntry entries[] =
{
    {"outputfile", 'o', 0, G_OPTION_ARG_FILENAME, &outputfile, "output", "filename"},
    {"generate-extra-enter", 0, 0, G_OPTION_ARG_NONE, &gen_extra_enter, "generate ", NULL},
    {NULL}
};


/* graph shortest path sentence segment. */

/* Note:
 * Currently libpinyin only supports ucs4 characters, as this is a
 * pre-processor tool for raw corpus, it will skip all sentences
 * which contains non-ucs4 characters.
 */

enum CONTEXT_STATE{
    CONTEXT_INIT,
    CONTEXT_SEGMENTABLE,
    CONTEXT_UNKNOWN
};

struct SegmentStep{
    phrase_token_t m_handle;
    ucs4_t * m_phrase;
    size_t m_phrase_len;
    //use formula W = number of words. Zero handle means one word.
    guint m_nword;
    //backtrace information, -1 one step backward.
    gint m_backward_nstep;
public:
    SegmentStep(){
        m_handle = null_token;
        m_phrase = NULL;
        m_phrase_len = 0;
        m_nword = UINT_MAX;
        m_backward_nstep = -0;
    }
};

bool backtrace(GArray * steps, glong phrase_len, GArray * strings);

/* Note: do not free phrase, as it is used by strings (array of segment). */
bool segment(FacadePhraseTable3 * phrase_table,
             FacadePhraseIndex * phrase_index,
             GArray * current_ucs4,
             GArray * strings /* Array of SegmentStep. */){
    ucs4_t * phrase = (ucs4_t *)current_ucs4->data;
    guint phrase_len = current_ucs4->len;

    /* Prepare for shortest path segment dynamic programming. */
    GArray * steps = g_array_new(TRUE, TRUE, sizeof(SegmentStep));
    SegmentStep step;
    for ( glong i = 0; i < phrase_len + 1; ++i ){
        g_array_append_val(steps, step);
    }

    SegmentStep * first_step = &g_array_index(steps, SegmentStep, 0);
    first_step->m_nword = 0;

    PhraseTokens tokens;
    memset(tokens, 0, sizeof(PhraseTokens));
    phrase_index->prepare_tokens(tokens);

    for ( glong i = 0; i < phrase_len + 1; ++i ) {
        SegmentStep * step_begin = &g_array_index(steps, SegmentStep, i);
        size_t nword = step_begin->m_nword;
        for ( glong k = i + 1; k < phrase_len + 1; ++k ) {
            size_t len = k - i;
            ucs4_t * cur_phrase = phrase + i;

            phrase_token_t token = null_token;
            phrase_index->clear_tokens(tokens);
            int result = phrase_table->search(len, cur_phrase, tokens);
            int num = get_first_token(tokens, token);

            if ( !(result & SEARCH_OK) ){
                token = null_token;
                if ( 1 != len )
                    continue;
            }
            ++nword;

            SegmentStep * step_end = &g_array_index(steps, SegmentStep, k);
            if ( nword < step_end->m_nword ) {
                step_end->m_handle = token;
                step_end->m_phrase = cur_phrase;
                step_end->m_phrase_len = len;
                step_end->m_nword = nword;
                step_end->m_backward_nstep = i - k;
            }
            if ( !(result & SEARCH_CONTINUED) )
                break;
        }
    }
    phrase_index->destroy_tokens(tokens);

    return backtrace(steps, phrase_len, strings);
}

bool backtrace(GArray * steps, glong phrase_len, GArray * strings){
    /* backtracing to get the result. */
    size_t cur_step = phrase_len;
    g_array_set_size(strings, 0);
    while ( cur_step ){
        SegmentStep * step = &g_array_index(steps, SegmentStep, cur_step);
        g_array_append_val(strings, *step);
        cur_step = cur_step + step->m_backward_nstep;
        /* intended to avoid leaking internal informations. */
        step->m_nword = 0; step->m_backward_nstep = 0;
    }

    /* reverse the strings. */
    for ( size_t i = 0; i < strings->len / 2; ++i ) {
        SegmentStep * head, * tail;
        head = &g_array_index(strings, SegmentStep, i);
        tail = &g_array_index(strings, SegmentStep, strings->len - 1 - i );
        SegmentStep tmp;
        tmp = *head;
        *head = *tail;
        *tail = tmp;
    }

    g_array_free(steps, TRUE);
    return true;
}

bool deal_with_segmentable(FacadePhraseTable3 * phrase_table,
                           FacadePhraseIndex * phrase_index,
                           GArray * current_ucs4,
                           FILE * output){

    /* do segment stuff. */
    GArray * strings = g_array_new(TRUE, TRUE, sizeof(SegmentStep));
    segment(phrase_table, phrase_index, current_ucs4, strings);

    /* print out the split phrase. */
    for ( glong i = 0; i < strings->len; ++i ) {
        SegmentStep * step = &g_array_index(strings, SegmentStep, i);
        char * string = g_ucs4_to_utf8( step->m_phrase, step->m_phrase_len, NULL, NULL, NULL);
        fprintf(output, "%d %s\n", step->m_handle, string);
        g_free(string);
    }

    g_array_free(strings, TRUE);
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

    context = g_option_context_new("- shortest path segment");
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

    CONTEXT_STATE state, next_state;
    GArray * current_ucs4 = g_array_new(TRUE, TRUE, sizeof(ucs4_t));

    PhraseTokens tokens;
    memset(tokens, 0, sizeof(PhraseTokens));
    phrase_index.prepare_tokens(tokens);

    char * linebuf = NULL; size_t size = 0; ssize_t read;
    while( (read = getline(&linebuf, &size, input)) != -1 ){
        if ( '\n' ==  linebuf[strlen(linebuf) - 1] ) {
            linebuf[strlen(linebuf) - 1] = '\0';
        }

        /* check non-ucs4 characters. */
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
                deal_with_segmentable(&phrase_table, &phrase_index,
                                      current_ucs4, output);

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
                deal_with_segmentable(&phrase_table, &phrase_index,
                                      current_ucs4, output);

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
