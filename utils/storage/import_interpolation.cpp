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
#include <locale.h>
#include <glib.h>
#include "pinyin_internal.h"
#include "utils_helper.h"


static const gchar * table_dir = ".";

static GOptionEntry entries[] =
{
    {"table-dir", 0, 0, G_OPTION_ARG_FILENAME, &table_dir, "table directory", NULL},
    {NULL}
};


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

bool parse_headline();

bool parse_unigram(FILE * input, PhraseLargeTable3 * phrase_table,
                   FacadePhraseIndex * phrase_index);

bool parse_bigram(FILE * input, PhraseLargeTable3 * phrase_table,
                  FacadePhraseIndex * phrase_index,
                  Bigram * bigram);

static ssize_t my_getline(FILE * input){
    ssize_t result = getline(&linebuf, &len, input);
    if ( result == -1 )
        return result;

    if ( '\n' == linebuf[strlen(linebuf) - 1] ) {
        linebuf[strlen(linebuf) - 1] = '\0';
    }
    return result;
}

bool parse_headline(){
    /* enter "\data" line */
    assert(taglib_add_tag(BEGIN_LINE, "\\data", 0, "model", ""));

    /* read "\data" line */
    if ( !taglib_read(linebuf, line_type, values, required) ) {
        fprintf(stderr, "error: interpolation model expected.\n");
        return false;
    }

    assert(line_type == BEGIN_LINE);
    /* check header */
    TAGLIB_GET_TAGVALUE(const char *, model, (const char *));
    if ( !( strcmp("interpolation", model) == 0 ) ) {
        fprintf(stderr, "error: interpolation model expected.\n");
        return false;
    }
    return true;
}

bool parse_body(FILE * input, PhraseLargeTable3 * phrase_table,
                FacadePhraseIndex * phrase_index,
                Bigram * bigram){
    taglib_push_state();

    assert(taglib_add_tag(END_LINE, "\\end", 0, "", ""));
    assert(taglib_add_tag(GRAM_1_LINE, "\\1-gram", 0, "", ""));
    assert(taglib_add_tag(GRAM_2_LINE, "\\2-gram", 0, "", ""));

    do {
    retry:
        assert(taglib_read(linebuf, line_type, values, required));
        switch(line_type) {
        case END_LINE:
            goto end;
        case GRAM_1_LINE:
            my_getline(input);
            parse_unigram(input, phrase_table, phrase_index);
            goto retry;
        case GRAM_2_LINE:
            my_getline(input);
            parse_bigram(input, phrase_table, phrase_index, bigram);
            goto retry;
        default:
            assert(false);
        }
    } while (my_getline(input) != -1) ;

 end:
    taglib_pop_state();
    return true;
}

bool parse_unigram(FILE * input, PhraseLargeTable3 * phrase_table,
                   FacadePhraseIndex * phrase_index){
    taglib_push_state();

    assert(taglib_add_tag(GRAM_1_ITEM_LINE, "\\item", 2, "count", ""));

    do {
        assert(taglib_read(linebuf, line_type, values, required));
        switch (line_type) {
        case GRAM_1_ITEM_LINE:{
            /* handle \item in \1-gram */
            TAGLIB_GET_TOKEN(token, 0);
            TAGLIB_GET_PHRASE_STRING(word, 1);
            assert(taglib_validate_token_with_string
                   (phrase_index, token, word));

            TAGLIB_GET_TAGVALUE(glong, count, atol);
            phrase_index->add_unigram_frequency(token, count);
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

bool parse_bigram(FILE * input, PhraseLargeTable3 * phrase_table,
                  FacadePhraseIndex * phrase_index,
                  Bigram * bigram){
    taglib_push_state();

    assert(taglib_add_tag(GRAM_2_ITEM_LINE, "\\item", 4, "count", ""));

    phrase_token_t last_token = 0; SingleGram * last_single_gram = NULL;
    do {
        assert(taglib_read(linebuf, line_type, values, required));
        switch (line_type) {
        case GRAM_2_ITEM_LINE:{
            /* handle \item in \2-gram */
            /* two tokens */
            TAGLIB_GET_TOKEN(token1, 0);
            TAGLIB_GET_PHRASE_STRING(word1, 1);
            assert(taglib_validate_token_with_string
                   (phrase_index, token1, word1));

            TAGLIB_GET_TOKEN(token2, 2);
            TAGLIB_GET_PHRASE_STRING(word2, 3);
            assert(taglib_validate_token_with_string
                   (phrase_index, token2, word2));

            TAGLIB_GET_TAGVALUE(glong, count, atol);

            if ( last_token != token1 ) {
                if ( last_token && last_single_gram ) {
                    bigram->store(last_token, last_single_gram);
                    delete last_single_gram;

                    /* safe guard */
                    last_token = null_token;
                    last_single_gram = NULL;
                }
                SingleGram * single_gram = NULL;
                bigram->load(token1, single_gram);

                /* create the new single gram */
                if ( single_gram == NULL )
                    single_gram = new SingleGram;
                last_token = token1;
                last_single_gram = single_gram;
            }

            /* save the freq */
            assert(NULL != last_single_gram);
            guint32 total_freq = 0;
            assert(last_single_gram->get_total_freq(total_freq));
            assert(last_single_gram->insert_freq(token2, count));
            total_freq += count;
            assert(last_single_gram->set_total_freq(total_freq));
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
    if ( last_token && last_single_gram ) {
        bigram->store(last_token, last_single_gram);
        delete last_single_gram;
        //safe guard
        last_token = 0;
        last_single_gram = NULL;
    }

    taglib_pop_state();
    return true;
}

int main(int argc, char * argv[]){
    FILE * input = stdin;
    const char * bigram_filename = SYSTEM_BIGRAM;

    setlocale(LC_ALL, "");

    GError * error = NULL;
    GOptionContext * context;

    context = g_option_context_new("- import interpolation model");
    g_option_context_add_main_entries(context, entries, NULL);
    if (!g_option_context_parse(context, &argc, &argv, &error)) {
        g_print("option parsing failed:%s\n", error->message);
        exit(EINVAL);
    }

    SystemTableInfo2 system_table_info;

    const gchar * filename = SYSTEM_TABLE_INFO;
    bool retval = system_table_info.load(filename);
    if (!retval) {
        fprintf(stderr, "load table.conf failed.\n");
        exit(ENOENT);
    }

    PhraseLargeTable3 phrase_table;

    retval = phrase_table.attach(SYSTEM_PHRASE_INDEX, ATTACH_READONLY);
    if (!retval) {
        fprintf(stderr, "open %s failed!\n", SYSTEM_PHRASE_INDEX);
        exit(ENOENT);
    }

    FacadePhraseIndex phrase_index;

    const pinyin_table_info_t * phrase_files =
        system_table_info.get_default_tables();

    if (!load_phrase_index(phrase_files, &phrase_index))
        exit(ENOENT);

    Bigram bigram;
    retval = bigram.attach(bigram_filename, ATTACH_CREATE|ATTACH_READWRITE);
    if (!retval) {
        fprintf(stderr, "open %s failed!\n", bigram_filename);
        exit(ENOENT);
    }

    taglib_init();

    values = g_ptr_array_new();
    required = g_hash_table_new(g_str_hash, g_str_equal);

    /* read first line */
    ssize_t result = my_getline(input);
    if ( result == -1 ) {
        fprintf(stderr, "empty file input.\n");
        exit(ENODATA);
    }

    if (!parse_headline())
        exit(ENODATA);

    result = my_getline(input);
    if ( result != -1 )
        parse_body(input, &phrase_table, &phrase_index, &bigram);

    taglib_fini();

    if (!save_phrase_index(phrase_files, &phrase_index))
        exit(ENOENT);

    return 0;
}
