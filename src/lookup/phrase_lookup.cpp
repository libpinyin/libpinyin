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

#include <math.h>
#include "stl_lite.h"
#include "novel_types.h"
#include "phrase_index.h"
#include "phrase_large_table.h"
#include "ngram.h"
#include "phrase_lookup.h"

using namespace pinyin;

const gfloat PhraseLookup::bigram_lambda = LAMBDA_PARAMETER;
const gfloat PhraseLookup::unigram_lambda = 1 - LAMBDA_PARAMETER;

PhraseLookup::PhraseLookup(PhraseLargeTable * phrase_table,
                           FacadePhraseIndex * phrase_index,
                           Bigram * system_bigram,
                           Bigram * user_bigram){
    m_phrase_table = phrase_table;
    m_phrase_index = phrase_index;
    m_system_bigram = system_bigram;
    m_user_bigram = user_bigram;

    m_steps_index = g_ptr_array_new();
    m_steps_content = g_ptr_array_new();
}

PhraseLookup::~PhraseLookup(){
    //free m_steps_index
    for ( size_t i = 0; i < m_steps_index->len; ++i){
        GHashTable * table = (GHashTable *) g_ptr_array_index(m_steps_index, i);
        g_hash_table_destroy(table);
    }
    g_ptr_array_free(m_steps_index, TRUE);
    m_steps_index = NULL;

    //free m_steps_content
    for ( size_t i = 0; i < m_steps_content->len; ++i){
        GArray * array = (GArray *) g_ptr_array_index(m_steps_content, i);
        g_array_free(array, TRUE);
    }
    g_ptr_array_free(m_steps_content, TRUE);
}

bool PhraseLookup::get_best_match(int sentence_length, utf16_t sentence[],
                                  MatchResults & results){
    m_sentence_length = sentence_length;
    m_sentence = sentence;
    int nstep = m_sentence_length + 1;

    //free m_steps_index
    for ( size_t i = 0; i < m_steps_index->len; ++i){
        GHashTable * table = (GHashTable *) g_ptr_array_index(m_steps_index, i);
        g_hash_table_destroy(table);
        g_ptr_array_index(m_steps_index, i) = NULL;
    }

    //free m_steps_content
    for ( size_t i = 0; i < m_steps_content->len; ++i){
        GArray * array = (GArray *) g_ptr_array_index(m_steps_content, i);
        g_array_free(array, TRUE);
        g_ptr_array_index(m_steps_content, i) = NULL;
    }

    //add null start step
    g_ptr_array_set_size(m_steps_index, nstep);
    g_ptr_array_set_size(m_steps_content, nstep);

    for ( int i = 0; i < nstep; ++i ){
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

    for ( int i = 0; i < nstep - 1; ++i ){
        for ( int m = i + 1; m < nstep; ++m ){
            phrase_token_t next_token = null_token;
            int result = m_phrase_table->search(m - i, sentence + i, next_token);
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

bool PhraseLookup::search_unigram(int nstep, phrase_token_t token){
    GArray * lookup_content = (GArray *) g_ptr_array_index(m_steps_content, nstep);
    if ( 0 == lookup_content->len )
        return false;

    lookup_value_t * max_value = &g_array_index(lookup_content, lookup_value_t, 0);
    /* find the maximum node */
    for ( size_t i = 1; i < lookup_content->len; ++i ){
        lookup_value_t * cur_value = &g_array_index(lookup_content, lookup_value_t, i);
        if ( cur_value->m_poss > max_value->m_poss )
            max_value = cur_value;
    }

    return unigram_gen_next_step(nstep, max_value, token);
}

bool PhraseLookup::search_bigram(int nstep, phrase_token_t token){
    bool found = false;
    GArray * lookup_content = (GArray *) g_ptr_array_index(m_steps_content, nstep);
    if ( 0 == lookup_content->len )
        return false;

    for ( size_t i = 0; i < lookup_content->len; ++i ){
        lookup_value_t * cur_value = &g_array_index(lookup_content, lookup_value_t, i);
        phrase_token_t index_token = cur_value->m_handles[1];
        SingleGram * system, * user;
        m_system_bigram->load(index_token, system);
        m_user_bigram->load(index_token, user);

        if ( system && user ){
            guint32 total_freq;
            assert(user->get_total_freq(total_freq));
            assert(system->set_total_freq(total_freq));
        }
        if ( system ){
            guint32 freq;
            if ( system->get_freq(token, freq) ){
                guint32 total_freq;
                system->get_total_freq(total_freq);
                gfloat bigram_poss = freq / (gfloat) total_freq;
                found = bigram_gen_next_step(nstep, cur_value, token, bigram_poss) || found;
            }
        }
        if ( user ){
            guint32 freq;
            if ( user->get_freq(token, freq) ){
                guint32 total_freq;
                user->get_total_freq(total_freq);
                gfloat bigram_poss = freq / (gfloat) total_freq;
                found = bigram_gen_next_step(nstep, cur_value, token, bigram_poss) || found;
            }
        }
    }

    return found;
}

bool PhraseLookup::unigram_gen_next_step(int nstep, lookup_value_t * cur_value,
phrase_token_t token){
    if ( m_phrase_index->get_phrase_item(token, m_cache_phrase_item))
        return false;
    size_t phrase_length = m_cache_phrase_item.get_phrase_length();
    gdouble elem_poss = m_cache_phrase_item.get_unigram_frequency() / (gdouble)
        m_phrase_index->get_phrase_index_total_freq();
    if ( elem_poss < DBL_EPSILON )
        return false;

    lookup_value_t next_value;
    next_value.m_handles[0] = cur_value->m_handles[1]; next_value.m_handles[1] = token;
    next_value.m_poss = cur_value->m_poss + log(elem_poss * unigram_lambda);
    next_value.m_last_step = nstep;

    return save_next_step(nstep + phrase_length, cur_value, &next_value);
}

bool PhraseLookup::bigram_gen_next_step(int nstep, lookup_value_t * cur_value, phrase_token_t token, gfloat bigram_poss){
    if ( m_phrase_index->get_phrase_item(token, m_cache_phrase_item))
        return false;
    size_t phrase_length = m_cache_phrase_item.get_phrase_length();
    gdouble unigram_poss = m_cache_phrase_item.get_unigram_frequency() /
        (gdouble) m_phrase_index->get_phrase_index_total_freq();

    if ( bigram_poss < FLT_EPSILON && unigram_poss < DBL_EPSILON )
        return false;

    lookup_value_t next_value;
    next_value.m_handles[0] = cur_value->m_handles[1]; next_value.m_handles[1] = token;
    next_value.m_poss = cur_value->m_poss +
        log( bigram_lambda * bigram_poss + unigram_lambda * unigram_poss );
    next_value.m_last_step = nstep;

    return save_next_step(nstep + phrase_length, cur_value, &next_value);
}

bool PhraseLookup::save_next_step(int next_step_pos, lookup_value_t * cur_value, lookup_value_t * next_value){
    lookup_key_t next_key = next_value->m_handles[1];
    GHashTable * next_lookup_index = (GHashTable *) g_ptr_array_index(m_steps_index, next_step_pos);
    GArray * next_lookup_content = (GArray *) g_ptr_array_index(m_steps_content, next_step_pos);

    gpointer key, value;
    gboolean lookup_result = g_hash_table_lookup_extended(next_lookup_index, GUINT_TO_POINTER(next_key), &key, &value);
    size_t step_index = GPOINTER_TO_UINT(value);
    if ( !lookup_result ){
        g_array_append_val(next_lookup_content, *next_value);
        g_hash_table_insert(next_lookup_index, GUINT_TO_POINTER(next_key), GUINT_TO_POINTER(next_lookup_content->len - 1));
        return true;
    }else{
        lookup_value_t * orig_next_value = &g_array_index(next_lookup_content, lookup_value_t, step_index);
        if ( orig_next_value->m_poss < next_value->m_poss ){
            orig_next_value->m_handles[0] = next_value->m_handles[0];
            assert(orig_next_value->m_handles[1] == next_value->m_handles[1]);
            orig_next_value->m_poss = next_value->m_poss;
            orig_next_value->m_last_step = next_value->m_last_step;
            return true;
        }
        return false;
    }
}

bool PhraseLookup::final_step(MatchResults & results ){
    //reset results
    g_array_set_size(results, m_steps_content->len);
    for ( size_t i = 0; i < results->len; ++i ){
        phrase_token_t * token = &g_array_index(results, phrase_token_t, i);
        *token = null_token;
    }

    //find max element
    size_t last_step_pos = m_steps_content->len - 1;
    GArray * last_step_content =  (GArray *) g_ptr_array_index(m_steps_content, last_step_pos);
    if ( last_step_content->len == 0 )
        return false;

    lookup_value_t * max_value = &g_array_index(last_step_content, lookup_value_t, 0);
    for ( size_t i = 1; i < last_step_content->len; ++i ){
        lookup_value_t * cur_value = &g_array_index(last_step_content, lookup_value_t, i);
        if ( cur_value->m_poss > max_value->m_poss )
            max_value = cur_value;
    }

    //backtracing
    while( true ){
        int cur_step_pos = max_value->m_last_step;
        if ( -1 == cur_step_pos )
            break;

        phrase_token_t * token = &g_array_index(results, phrase_token_t, cur_step_pos);
        *token = max_value->m_handles[1];

        phrase_token_t last_token = max_value->m_handles[0];
        GHashTable * lookup_step_index = (GHashTable *) g_ptr_array_index(m_steps_index, cur_step_pos);
        gpointer key, value;
        gboolean result = g_hash_table_lookup_extended(lookup_step_index, GUINT_TO_POINTER(last_token), &key, &value);
        if ( !result )
            return false;
        GArray * lookup_step_content = (GArray *) g_ptr_array_index(m_steps_content, cur_step_pos);
        max_value = &g_array_index(lookup_step_content, lookup_value_t, GPOINTER_TO_UINT(value));
    }

    //no need to reverse the result
    return true;
}
