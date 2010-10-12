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
#include <assert.h>
#include <glib.h>
#include "novel_types.h"
#include "memory_chunk.h"
#include "phrase_index.h"
#include "ngram.h"

/* export interpolation model as textual format */

void gen_unigram(FILE * output, FacadePhraseIndex * phrase_index);
void gen_bigram(FILE * output, FacadePhraseIndex * phrase_index, Bigram * bigram);

/* consider moving the following function to utils/storage/utility.h */
char * token_to_string(FacadePhraseIndex * phrase_index, phrase_token_t token);

void begin_data(FILE * file){
    fprintf(file, "\\data model interpolation\n");
}

void end_data(FILE * file){
    fprintf(file, "\\end\n");
}

int main(int argc, char * argv[]){
    FILE * output = stdout;
    const char * bigram_filename = "../../data/bigram.db";

    FacadePhraseIndex phrase_index;

    //gb_char binary file
    MemoryChunk * chunk = new MemoryChunk;
    chunk->load("../../data/gb_char.bin");
    phrase_index.load(1, chunk);

    //gbk_char binary file
    chunk = new MemoryChunk;
    chunk->load("../../data/gbk_char.bin");
    phrase_index.load(2, chunk);

    Bigram bigram;
    bigram.attach(bigram_filename, NULL);

    begin_data(output);

    gen_unigram(output, &phrase_index);
    gen_bigram(output, &phrase_index, &bigram);

    end_data(output);
    return 0;
}

void gen_unigram(FILE * output, FacadePhraseIndex * phrase_index) {
    fprintf(output, "\\1-gram\n");
    for ( size_t i = 0; i < PHRASE_INDEX_LIBRARY_COUNT; i++) {

        PhraseIndexRange range;
        int result = phrase_index->get_range(i, range);
        if ( result )
            continue;

        PhraseItem item;
        for ( size_t j = range.m_range_begin; j < range.m_range_end; j++) {
            int result = phrase_index->get_phrase_item(j, item);

            if ( result == ERROR_NO_ITEM )
                continue;
            assert( result == ERROR_OK);

            size_t freq = item.get_unigram_frequency();
            char * phrase = token_to_string(phrase_index, j);
            if ( phrase )
                fprintf(output, "\\item %s count %lu\n", phrase, freq);

            g_free(phrase);
        }
    }
}

void gen_bigram(FILE * output, FacadePhraseIndex * phrase_index, Bigram * bigram){
    fprintf(output, "\\2-gram\n");

    /* Retrieve all user items. */
    GArray * system_items = g_array_new(FALSE, FALSE, sizeof(phrase_token_t));
    GArray * user_items = g_array_new(FALSE, FALSE, sizeof(phrase_token_t));

    bigram->get_all_items(system_items, user_items);
    assert(0 == user_items->len);
    g_array_free(user_items, TRUE);

    PhraseItem item;

    for(size_t i = 0; i < system_items->len; i++){
        phrase_token_t token = g_array_index(system_items, phrase_token_t, i);
        SingleGram * system = NULL, * user = NULL;
        bigram->load(token, system, user);
        assert(NULL == user);

        BigramPhraseWithCountArray array = g_array_new(FALSE, FALSE, sizeof(BigramPhraseItemWithCount));
        system->retrieve_all(array);
        for(size_t j = 0; j < array->len; j++) {
            BigramPhraseItemWithCount * item = &g_array_index(array, BigramPhraseItemWithCount, j);

            char * word1 = token_to_string(phrase_index, token);
            char * word2 = token_to_string(phrase_index, item->m_token);
            guint32 freq = item->m_count;

            if ( word1 && word2)
                fprintf(output, "\\item %s %s count %d\n", word1, word2, freq);

            g_free(word1); g_free(word2);
        }

        g_array_free(array, TRUE);
    }

    g_array_free(system_items, TRUE);
}

static const char * special_token_to_string(phrase_token_t token){
    struct token_pair{
        phrase_token_t token;
        const char * string;
    };

    static const token_pair tokens [] = {
        {sentence_start, "<start>"},
        {0, NULL}
    };

    const token_pair * pair = tokens;
    while (pair->token) {
        if ( token == pair->token )
            return pair->string;
    }

    fprintf(stderr, "error: unknown token:%d.\n", token);
    return NULL;
}

char * token_to_string(FacadePhraseIndex * phrase_index, phrase_token_t token) {
    PhraseItem item;
    utf16_t buffer[MAX_PHRASE_LENGTH];

    gchar * phrase;
    /* deal with the special phrase index, for "<start>..." */
    if ( PHRASE_INDEX_LIBRARY_INDEX(token) == 0 ) {
        return g_strdup(special_token_to_string(token));
    }

    int result = phrase_index->get_phrase_item(token, item);
    if (result != ERROR_OK) {
        fprintf(stderr, "error: unknown token:%d.\n", token);
        return NULL;
    }

    item.get_phrase_string(buffer);
    guint8 length = item.get_phrase_length();
    phrase = g_utf16_to_utf8(buffer, length, NULL, NULL, NULL);
    return phrase;
}
