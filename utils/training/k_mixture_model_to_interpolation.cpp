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

enum LINE_TYPE{
    BEGIN_LINE = 1,
    END_LINE,
    GRAM_1_LINE,
    GRAM_2_LINE,
    GRAM_1_ITEM_LINE,
    GRAM_2_ITEM_LINE
};

static int line_type = 0;
static GPtrArray * values = NULL;
static GHashTable * required = NULL;
/* variables for line buffer. */
static char * linebuf = NULL;
static size_t len = 0;

bool parse_unigram(FILE * input, FILE * output);

bool parse_bigram(FILE * input, FILE * output);

static ssize_t my_getline(FILE * input){
    ssize_t result = getline(&linebuf, &len, input);
    if ( result == -1 )
        return result;

    linebuf[strlen(linebuf) - 1] = '\0';
    return result;
}

bool parse_body(FILE * input, FILE * output){
    taglib_push_state();

    assert(taglib_add_tag(END_LINE, "\\end", 0, "", ""));
    assert(taglib_add_tag(GRAM_1_LINE, "\\1-gram", 0, "", ""));
    assert(taglib_add_tag(GRAM_2_LINE, "\\2-gram", 0, "", ""));

    do {
    retry:
        assert(taglib_read(linebuf, line_type, values, required));
        switch(line_type) {
        case END_LINE:
            fprintf(output, "\\end\n");
            goto end;
        case GRAM_1_LINE:
            fprintf(output, "\\1-gram\n");
            my_getline(input);
            parse_unigram(input, output);
            goto retry;
        case GRAM_2_LINE:
            fprintf(output, "\\2-gram\n");
            my_getline(input);
            parse_bigram(input, output);
            goto retry;
        default:
            assert(false);
        }
    } while (my_getline(input) != -1);

 end:
    taglib_pop_state();
    return true;
}

bool parse_unigram(FILE * input, FILE * output){
    taglib_push_state();

    assert(taglib_add_tag(GRAM_1_ITEM_LINE, "\\item", 1, "freq", "count"));

    do {
        assert(taglib_read(linebuf, line_type, values, required));
        switch(line_type) {
        case GRAM_1_ITEM_LINE: {
            /* handle \item in \1-gram */
            const char * string = (const char *) g_ptr_array_index(values, 0);
            /* remove the "<start>" in the uni-gram of interpolation model */
            if ( strcmp("<start>", string) == 0 )
                break;
            gpointer value = NULL;
            assert(g_hash_table_lookup_extended(required, "freq",
                                                NULL, &value));
            glong freq = atol ((const char *) value);
            /* ignore zero unigram freq item */
            if ( 0 != freq )
                fprintf(output, "\\item %s count %d\n", string, freq);
            break;
        }
        case END_LINE:
        case GRAM_1_LINE:
        case GRAM_2_LINE:
            goto end;
        default:
            assert(false);
        }
    } while (my_getline(input) != -1);

 end:
    taglib_pop_state();
    return true;
}

bool parse_bigram(FILE * input, FILE * output){
    taglib_push_state();

    assert(taglib_add_tag(GRAM_2_ITEM_LINE, "\\item", 2,
                          "count", "T:N_n_0:n_1:Mr"));

    do {
        assert(taglib_read(linebuf, line_type, values, required));
        switch (line_type) {
        case GRAM_2_ITEM_LINE:{
            /* handle \item in \2-gram */
            /* two strings */
            const char * string1 = (const char *) g_ptr_array_index(values, 0);
            const char * string2 = (const char *) g_ptr_array_index(values, 1);

            gpointer value = NULL;
            /* tag: count */
            assert(g_hash_table_lookup_extended(required, "count", NULL, &value));
            const char * count = (const char *)value;
            fprintf(output, "\\item %s %s count %s\n", string1, string2, count);
            break;
        }
        case END_LINE:
        case GRAM_1_LINE:
        case GRAM_2_LINE:
            goto end;
        default:
            assert(false);
        }
    } while (my_getline(input) != -1);

 end:
    taglib_pop_state();
    return true;
}

int main(int argc, char * argv[]){
    FILE * input = stdin;
    FILE * output = stdout;

    taglib_init();

    values = g_ptr_array_new();
    required = g_hash_table_new(g_str_hash, g_str_equal);

    //enter "\data" line
    assert(taglib_add_tag(BEGIN_LINE, "\\data", 0, "model",
                          "count:N:total_freq"));
    ssize_t result = my_getline(input);
    if ( result == -1 ) {
        fprintf(stderr, "empty file input.\n");
        exit(ENODATA);
    }

    //read "\data" line
    if ( !taglib_read(linebuf, line_type, values, required) ) {
        fprintf(stderr, "error: k mixture model expected.\n");
        exit(ENODATA);
    }

    assert(line_type == BEGIN_LINE);
    gpointer value = NULL;
    assert(g_hash_table_lookup_extended(required, "model", NULL, &value));
    const char * model = (const char *) value;
    if ( !( strcmp("k mixture model", model) == 0 ) ){
        fprintf(stderr, "error: k mixture model expected.\n");
        exit(ENODATA);
    }

    fprintf(output, "\\data model interpolation\n");

    result = my_getline(input);
    if ( result != -1 )
        parse_body(input, output);

    taglib_fini();

    return 0;
}
