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
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

#include <stdio.h>
#include <glib.h>
#include "novel_types.h"
#include "memory_chunk.h"
#include "phrase_index.h"
#include "ngram.h"
#include "phrase_large_table.h"
#include "tag_utility.h"

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

bool parse_body(FILE * input, PhraseLargeTable * phrases,
                FacadePhraseIndex * phrase_index,
                Bigram * bigram){
    taglib_push_state();

    assert(taglib_add_tag(END_LINE, "\\end", 0, "", ""));
    assert(taglib_add_tag(GRAM_1_LINE, "\\1-gram", 0, "", ""));
    assert(taglib_add_tag(GRAM_2_LINE, "\\2-gram", 0, "", ""));

    do{
    retry:
        assert(taglib_read(linebuf, line_type, values, required));
        switch(line_type) {
        case END_LINE:
            goto end;
            break;
        case GRAM_1_LINE:
            parse_unigram(input, phrases, phrase_index);
            goto retry;
            break;
        case GRAM_2_LINE:
            parse_bigram(input, phrases, phrase_index, bigram);
            goto retry;
            break;
        default:
            assert(false);
        }
    } while (getline(&linebuf, &len, input) != -1) ;

 end:
    taglib_pop_state();
    return true;
}

bool parse_unigram(FILE * input, PhraseLargeTable * phrases,
                   FacadePhraseIndex * phrase_index){

    return true;
}

bool parse_bigram(FILE * input, PhraseLargeTable * phrases,
                  FacadePhraseIndex * phrase_index,
                  Bigram * bigram){

    return true;
}

int main(int argc, char * argv[]){
    FILE * input = stdin;
    const char * bigram_filename = "../../data/bigram.db";

    PhraseLargeTable phrases;

    MemoryChunk * chunk = new MemoryChunk;
    chunk->load("../../data/phrase_index.bin");
    phrases.load(chunk);

    FacadePhraseIndex phrase_index;

    //gb_char binary file
    chunk = new MemoryChunk;
    chunk->load("../../data/gb_char.bin");
    phrase_index.load(1, chunk);

    //gbk_char binary file
    chunk = new MemoryChunk;
    chunk->load("../../data/gbk_char.bin");
    phrase_index.load(2, chunk);

    Bigram bigram;
    bigram.attach(NULL, bigram_filename);

    taglib_init();

    values = g_ptr_array_new();
    required = g_hash_table_new(g_str_hash, g_str_equal);

    assert(taglib_add_tag(BEGIN_LINE, "\\data", 0, "model", ""));
    getline(&linebuf, &len, input);
    linebuf[strlen(linebuf) - 1] = '\0';

    assert(taglib_read(linebuf, line_type, values, required));
    assert(line_type == BEGIN_LINE);
    char * value = NULL;
    assert(g_hash_table_lookup_extended(required, "model", NULL, (gpointer *)&value));
    assert(strcmp("interpolation", value) == 0);

    getline(&linebuf, &len, input);
    linebuf[strlen(linebuf) - 1] = '\0';
    parse_body(input, &phrases, &phrase_index, &bigram);

    taglib_fini();

    return 0;
}
