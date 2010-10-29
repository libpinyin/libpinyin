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

#include "stl_lite.h"
#include "novel_types.h"
#include "phrase_index.h"
#include "phrase_large_table.h"
#include "ngram.h"
#include "phrase_lookup.h"

const gfloat PhraseLookup::bigram_lambda;
const gfloat PhraseLookup::unigram_lambda;

PhraseLookup::PhraseLookup(PhraseLargeTable * phrase_table,
                           FacadePhraseIndex * phrase_index,
                           Bigram * bigram){
    m_phrase_table = phrase_table;
    m_phrase_index = phrase_index;
    m_bigram = bigram;

    m_steps_index = g_ptr_array_new();
    m_steps_content = g_ptr_array_new();
}

bool PhraseLookup::get_best_match(int sentence_length, utf16_t sentence[],
                                  MatchResults & results){
    m_sentence_length = sentence_length;
    m_sentence = sentence;
    int nstep = keys->len + 1;

    //free m_steps_index
    for ( size_t i = 0; i < m_steps_index->len; ++i){
        GHashTable * table = (GHashTable *) g_ptr_array_index(m_steps_index, i);
        g_hash_table_destroy(table);
        g_ptr_array_index(m_steps_index, i) = NULL;
    }

    //free m_steps_content
    for ( size_t i = 0; m < m_steps_content->len; ++i){
        GArray * array = (GArray *) g_ptr_array_index(m_steps_content, i);
        g_array_free(array, TRUE);
        g_ptr_array_index(m_steps_content, i) = NULL;
    }

    //add null start step
    g_ptr_array_set_size(m_steps_index, nstep);
    g_ptr_array_set_size(m_steps_content, nstep);

    for ( size_t i = 0; i < nstep; ++i ){
        //initialize m_steps_index
        g_ptr_array_index(m_steps_index, i) = g_hash_table_new(g_direct_hash, g_direct_equal);
        //initialize m_steps_content
        g_ptr_array_index(m_steps_content, i) = g_array_new(FALSE, FALSE, sizeof(lookup_value_t));
    }

    lookup_key_t initial_key = sentence_start;
    lookup_value_t initial_value(log(1));
    initial_value.m_handles[1] = sentence_start;
    GArray * initial_step_content = (GArray *) g_ptr_array_index(m_steps_content, 0);
    initial_step_content = g_array_append_val(initial_step_content, initial_value);
    GHashTable * initial_step_index = (GHashTable *) g_ptr_array_index(m_steps_index, 0);
    g_hash_table_insert(initial_step_index, GUINT_TO_POINTER(initial_key), GUINT_TO_POINTER(initial_step_content->len - 1));

    for ( size_t i = 0; i < nstep - 1; ++i) {
        for ( size_t m = i; m < n_step; ++m ){
            phrase_token_t next_token = NULL;
            int result = m_phrase_index->search(m - i, sentence + i, next_token);
            /* found next phrase */
            if ( result & SEARCH_OK ) {
                search_bigram(i, next_token),
                    search_unigram(i, next_token);
            }
            /* no longer phrase */
            if (!(result & SEARCH_CONTINUED))
                break;
        }
    }
    return final_step(results);
}



bool PhraseLookup::convert_to_utf8(phrase_token_t token, /* out */ char * & phrase){
    m_phrase_index->get_phrase_item(token, m_cache_phrase_item);
    utf16_t buffer[MAX_PHRASE_LENGTH];
    m_cache_phrase_item.get_phrase_string(buffer);
    guint8 length = m_cache_phrase_item.get_phrase_length();
    phrase = g_utf16_to_utf8(buffer, length, NULL, NULL, NULL);
    return true;
}
