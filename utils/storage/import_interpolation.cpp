/* 
 *  libpinyin
 *  Library to deal with pinyin.
 *  
 *  Copyright (C) 2010 Peng Wu
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

#include <stdio.h>
#include <glib.h>
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

bool parse_unigram(FILE * input, PhraseLargeTable * phrases,
                   FacadePhraseIndex * phrase_index);

bool parse_bigram(FILE * input, PhraseLargeTable * phrases,
                  FacadePhraseIndex * phrase_index,
                  Bigram * bigram);

static ssize_t my_getline(FILE * input){
    ssize_t result = getline(&linebuf, &len, input);
    if ( result == -1 )
        return result;

    linebuf[strlen(linebuf) - 1] = '\0';
    return result;
}

bool parse_body(FILE * input, PhraseLargeTable * phrases,
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
            parse_unigram(input, phrases, phrase_index);
            goto retry;
        case GRAM_2_LINE:
            my_getline(input);
            parse_bigram(input, phrases, phrase_index, bigram);
            goto retry;
        default:
            assert(false);
        }
    } while (my_getline(input) != -1) ;

 end:
    taglib_pop_state();
    return true;
}

bool parse_unigram(FILE * input, PhraseLargeTable * phrases,
                   FacadePhraseIndex * phrase_index){
    taglib_push_state();

    assert(taglib_add_tag(GRAM_1_ITEM_LINE, "\\item", 1, "count", ""));

    do {
        assert(taglib_read(linebuf, line_type, values, required));
        switch (line_type) {
        case GRAM_1_ITEM_LINE:{
            /* handle \item in \1-gram */
            const char * string = (const char *) g_ptr_array_index(values, 0);
            phrase_token_t token = taglib_string_to_token(phrases, string);
            gpointer value = NULL;
            assert(g_hash_table_lookup_extended(required, "count", NULL, &value));
            glong count = atol((const char *)value);
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

bool parse_bigram(FILE * input, PhraseLargeTable * phrases,
                  FacadePhraseIndex * phrase_index,
                  Bigram * bigram){
    taglib_push_state();

    assert(taglib_add_tag(GRAM_2_ITEM_LINE, "\\item", 2, "count", ""));

    phrase_token_t last_token = 0; SingleGram * last_single_gram = NULL;
    do {
        assert(taglib_read(linebuf, line_type, values, required));
        switch (line_type) {
        case GRAM_2_ITEM_LINE:{
            /* handle \item in \2-gram */
            /* two tokens */
            const char * string = (const char *) g_ptr_array_index(values, 0);
            phrase_token_t token1 = taglib_string_to_token(phrases, string);
            string = (const char *) g_ptr_array_index(values, 1);
            phrase_token_t token2 = taglib_string_to_token(phrases, string);

            gpointer value = NULL;
            /* tag: count */
            assert(g_hash_table_lookup_extended(required, "count", NULL, &value));
            glong count = atol((const char *)value);

            if ( last_token != token1 ) {
                if ( last_token && last_single_gram ) {
                    bigram->store(last_token, last_single_gram);
                    delete last_single_gram;
                    //safe guard
                    last_token = 0;
                    last_single_gram = NULL;
                }
                SingleGram * single_gram = NULL;
                bigram->load(token1, single_gram);

                //create the new single gram
                if ( single_gram == NULL )
                    single_gram = new SingleGram;
                last_token = token1;
                last_single_gram = single_gram;
            }
            //save the freq
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
    const char * bigram_filename = "bigram.db";

    PhraseLargeTable phrases;

    MemoryChunk * chunk = new MemoryChunk;
    chunk->load("phrase_index.bin");
    phrases.load(chunk);

    FacadePhraseIndex phrase_index;

    //gb_char binary file
    chunk = new MemoryChunk;
    chunk->load("gb_char.bin");
    phrase_index.load(1, chunk);

    //gbk_char binary file
    chunk = new MemoryChunk;
    chunk->load("gbk_char.bin");
    phrase_index.load(2, chunk);

    Bigram bigram;
    bigram.attach(bigram_filename, ATTACH_CREATE|ATTACH_READWRITE);

    taglib_init();

    values = g_ptr_array_new();
    required = g_hash_table_new(g_str_hash, g_str_equal);

    //enter "\data" line
    assert(taglib_add_tag(BEGIN_LINE, "\\data", 0, "model", ""));
    ssize_t result = my_getline(input);
    if ( result == -1 ) {
        fprintf(stderr, "empty file input.\n");
        exit(ENODATA);
    }

    //read "\data" line
    if ( !taglib_read(linebuf, line_type, values, required) ) {
        fprintf(stderr, "error: interpolation model expected.\n");
        exit(ENODATA);
    }

    assert(line_type == BEGIN_LINE);
    char * value = NULL;
    assert(g_hash_table_lookup_extended(required, "model", NULL, (gpointer *)&value));
    if ( !( strcmp("interpolation", value) == 0 ) ) {
        fprintf(stderr, "error: interpolation model expected.\n");
        exit(ENODATA);
    }

    result = my_getline(input);
    if ( result != -1 )
        parse_body(input, &phrases, &phrase_index, &bigram);

    taglib_fini();

    chunk = new MemoryChunk;
    phrase_index.store(1, chunk);
    chunk->save("gb_char.bin");
    phrase_index.load(1, chunk);

    chunk = new MemoryChunk;
    phrase_index.store(2, chunk);
    chunk->save("gbk_char.bin");
    phrase_index.load(2, chunk);

    return 0;
}
