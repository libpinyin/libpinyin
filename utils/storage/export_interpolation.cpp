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
#include <assert.h>
#include <glib.h>
#include "pinyin_internal.h"


/* export interpolation model as textual format */

bool gen_unigram(FILE * output, FacadePhraseIndex * phrase_index);
bool gen_bigram(FILE * output, FacadePhraseIndex * phrase_index, Bigram * bigram);

bool begin_data(FILE * output){
    fprintf(output, "\\data model interpolation\n");
    return true;
}

bool end_data(FILE * output){
    fprintf(output, "\\end\n");
    return true;
}

int main(int argc, char * argv[]){
    FILE * output = stdout;
    const char * bigram_filename = "bigram.db";

    FacadePhraseIndex phrase_index;

    //gb_char binary file
    MemoryChunk * chunk = new MemoryChunk;
    chunk->load("gb_char.bin");
    phrase_index.load(1, chunk);

    //gbk_char binary file
    chunk = new MemoryChunk;
    chunk->load("gbk_char.bin");
    phrase_index.load(2, chunk);

    Bigram bigram;
    bigram.attach(bigram_filename, ATTACH_READONLY);

    begin_data(output);

    gen_unigram(output, &phrase_index);
    gen_bigram(output, &phrase_index, &bigram);

    end_data(output);
    return 0;
}

bool gen_unigram(FILE * output, FacadePhraseIndex * phrase_index) {
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
            if ( 0 == freq )
                continue;
            char * phrase = taglib_token_to_string(phrase_index, j);
            if ( phrase )
                fprintf(output, "\\item %s count %ld\n", phrase, freq);

            g_free(phrase);
        }
    }
    return true;
}

bool gen_bigram(FILE * output, FacadePhraseIndex * phrase_index, Bigram * bigram){
    fprintf(output, "\\2-gram\n");

    /* Retrieve all user items. */
    GArray * items = g_array_new(FALSE, FALSE, sizeof(phrase_token_t));

    bigram->get_all_items(items);

    PhraseItem item;

    for(size_t i = 0; i < items->len; i++){
        phrase_token_t token = g_array_index(items, phrase_token_t, i);
        SingleGram * single_gram = NULL;
        bigram->load(token, single_gram);

        BigramPhraseWithCountArray array = g_array_new(FALSE, FALSE, sizeof(BigramPhraseItemWithCount));
        single_gram->retrieve_all(array);
        for(size_t j = 0; j < array->len; j++) {
            BigramPhraseItemWithCount * item = &g_array_index(array, BigramPhraseItemWithCount, j);

            char * word1 = taglib_token_to_string(phrase_index, token);
            char * word2 = taglib_token_to_string(phrase_index, item->m_token);
            guint32 freq = item->m_count;

            if ( word1 && word2)
                fprintf(output, "\\item %s %s count %d\n", word1, word2, freq);

            g_free(word1); g_free(word2);
        }

        g_array_free(array, TRUE);
    }

    g_array_free(items, TRUE);
    return true;
}
