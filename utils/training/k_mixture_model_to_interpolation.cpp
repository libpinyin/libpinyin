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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "pinyin_internal.h"
#include "utils_helper.h"

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

bool parse_headline(FILE * input, FILE * output);

bool parse_unigram(FILE * input, FILE * output);

bool parse_bigram(FILE * input, FILE * output);

static ssize_t my_getline(FILE * input){
    ssize_t result = getline(&linebuf, &len, input);
    if ( result == -1 )
        return result;

    linebuf[strlen(linebuf) - 1] = '\0';
    return result;
}

bool parse_headline(FILE * input, FILE * output) {
    /* enter "\data" line */
    assert(taglib_add_tag(BEGIN_LINE, "\\data", 0, "model",
                          "count:N:total_freq"));

    /* read "\data" line */
    if ( !taglib_read(linebuf, line_type, values, required) ) {
        fprintf(stderr, "error: k mixture model expected.\n");
        return false;
    }

    assert(line_type == BEGIN_LINE);
    TAGLIB_GET_TAGVALUE(const char *, model, (const char *));
    if ( !( strcmp("k mixture model", model) == 0 ) ){
        fprintf(stderr, "error: k mixture model expected.\n");
        return false;
    }

    /* print header */
    fprintf(output, "\\data model interpolation\n");

    return true;
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

    assert(taglib_add_tag(GRAM_1_ITEM_LINE, "\\item", 2, "freq", "count"));

    do {
        assert(taglib_read(linebuf, line_type, values, required));
        switch(line_type) {
        case GRAM_1_ITEM_LINE: {
            /* handle \item in \1-gram */
            TAGLIB_GET_TOKEN(token, 0);
            TAGLIB_GET_PHRASE_STRING(word, 1);

            /* remove the "<start>" in the uni-gram of interpolation model */
            if ( sentence_start == token )
                break;

            TAGLIB_GET_TAGVALUE(glong, freq, atol);

            /* ignore zero unigram freq item */
            if ( 0 != freq )
                fprintf(output, "\\item %d %s count %ld\n", token, word, freq);
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

    assert(taglib_add_tag(GRAM_2_ITEM_LINE, "\\item", 4,
                          "count", "T:N_n_0:n_1:Mr"));

    do {
        assert(taglib_read(linebuf, line_type, values, required));
        switch (line_type) {
        case GRAM_2_ITEM_LINE:{
            /* handle \item in \2-gram */
            /* two strings */
            TAGLIB_GET_TOKEN(token1, 0);
            TAGLIB_GET_PHRASE_STRING(word1, 1);

            TAGLIB_GET_TOKEN(token2, 2);
            TAGLIB_GET_PHRASE_STRING(word2, 3);

            TAGLIB_GET_TAGVALUE(glong, count, atol);
            fprintf(output, "\\item %d %s %d %s count %ld\n",
                    token1, word1, token2, word2, count);
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

    ssize_t result = my_getline(input);
    if ( result == -1 ) {
        fprintf(stderr, "empty file input.\n");
        exit(ENODATA);
    }

    if (!parse_headline(input, output))
        exit(ENODATA);

    result = my_getline(input);
    if ( result != -1 )
        parse_body(input, output);

    taglib_fini();

    return 0;
}
