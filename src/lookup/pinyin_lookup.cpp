/* 
 *  novel-pinyin,
 *  A Simplified Chinese Sentence-Based Pinyin Input Method Engine
 *  Based On Markov Model.
 *  
 *  Copyright (C) 2006-2007 Peng Wu
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

#include <math.h>
#include <assert.h>
#include <iostream>
#include "stl_lite.h"
#include "novel_types.h"
#include "pinyin_base.h"
#include "pinyin_phrase.h"
#include "pinyin_large_table.h"
#include "phrase_index.h"
#include "ngram.h"
#include "lookup.h"
#include "winner_tree.h"

const gfloat PinyinLookup::bigram_lambda;
const gfloat PinyinLookup::unigram_lambda;

PinyinLookup::PinyinLookup(PinyinCustomSettings * custom, PinyinLargeTable * pinyin_table, FacadePhraseIndex * phrase_index, Bigram * bigram){
    m_custom = custom;
    m_pinyin_table = pinyin_table;
    m_phrase_index = phrase_index;
    m_bigram = bigram;
    m_winner_tree = new WinnerTree;
    m_steps_index = g_ptr_array_new();
    m_steps_content = g_ptr_array_new();
    m_table_cache = g_array_new(FALSE, TRUE, sizeof(PhraseIndexRanges));
    g_array_set_size(m_table_cache, 1);
}

PinyinLookup::~PinyinLookup(){
    if ( m_winner_tree )
	delete m_winner_tree;
    m_winner_tree = NULL;
    //free resources
    for ( size_t i = 0; i < m_table_cache->len; ++i){
	PhraseIndexRanges * ranges = &g_array_index(m_table_cache, PhraseIndexRanges, i);
	destroy_pinyin_lookup(*ranges);
    }
    //g_array_set_size(m_table_cache, 1);
    g_array_free(m_table_cache, TRUE);

    //free m_steps_index
    for ( size_t i = 0; i < m_steps_index->len; ++i){
	GHashTable * table = (GHashTable *) g_ptr_array_index(m_steps_index, i);
	g_hash_table_destroy(table);
	g_ptr_array_index(m_steps_index, i) = NULL;
    }
    g_ptr_array_free(m_steps_index, TRUE);

    //free m_steps_content
    for ( size_t i = 0; i < m_steps_content->len; ++i){
	GArray * array = (GArray *) g_ptr_array_index(m_steps_content, i);
	g_array_free(array, TRUE);
	g_ptr_array_index(m_steps_content, i) = NULL;
    }
    g_ptr_array_free(m_steps_content, TRUE);
        
}

bool PinyinLookup::prepare_pinyin_lookup(PhraseIndexRanges ranges){
    //memset(ranges, 0, sizeof(ranges));
    for ( size_t i = 0; i < PHRASE_INDEX_LIBRARY_COUNT; ++i ){
	GArray * & array = ranges[i];
	assert(NULL == array);
	if (m_phrase_index->m_sub_phrase_indices[i]){
	    array = g_array_new(FALSE, FALSE, sizeof (PhraseIndexRange));
	}
    }
	return true;
}

bool PinyinLookup::destroy_pinyin_lookup(PhraseIndexRanges ranges){
    for ( size_t i = 0; i < PHRASE_INDEX_LIBRARY_COUNT ; ++i){
	GArray * & array = ranges[i];
	if ( array )
	    g_array_free(array, TRUE);
	array = NULL;
    }
	return true;
}

size_t PinyinLookup::prepare_table_cache(int nstep, int total_pinyin){
    //free resources
    for ( size_t i = 0; i < m_table_cache->len; ++i){
	PhraseIndexRanges * ranges = &g_array_index(m_table_cache, PhraseIndexRanges, i);
	destroy_pinyin_lookup(*ranges);
    }
    //g_array_set_size(m_table_cache, 1);
    PinyinKey * pinyin_keys = (PinyinKey *)m_keys->data;
    pinyin_keys += nstep;
    //init resources
    g_array_set_size(m_table_cache, MAX_PHRASE_LENGTH + 1);
    size_t len;
    for ( len = 1; len <= total_pinyin && len <= MAX_PHRASE_LENGTH; ++len){
	PhraseIndexRanges * ranges = &g_array_index(m_table_cache, PhraseIndexRanges, len);
	prepare_pinyin_lookup(*ranges);
	int result = m_pinyin_table->search(len, pinyin_keys, *ranges);
	if (!( result & SEARCH_CONTINUED)){
	    ++len;
	    break;
	}
    }
    g_array_set_size(m_table_cache, std_lite::min(len, (size_t) MAX_PHRASE_LENGTH + 1));
    return m_table_cache->len - 1;
}

bool PinyinLookup::get_best_match(PinyinKeyVector keys, CandidateConstraints constraints, MatchResults & results){
    //g_array_set_size(results, 0);

    m_constraints = constraints;
    m_keys = keys;
    int nstep = keys->len + 1;

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

    for ( size_t i = 0 ; i < nstep; ++i ){
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

#if 0
    LookupStepContent tmp_step = (LookupStepContent) g_ptr_array_index(m_steps_content, 0);
    IBranchIterator * iter = m_winner_tree->get_iterator(tmp_step);
    size_t npinyin = prepare_table_cache(0, keys->len);
    search_unigram(iter, 0, npinyin);
    delete iter;
#endif

    for ( size_t i = 0 ; i < nstep - 1 ; ++i ){
	LookupStepContent tmp_step = (LookupStepContent) g_ptr_array_index(m_steps_content, i);
	IBranchIterator * iter = m_winner_tree->get_iterator(tmp_step);
	size_t npinyin = prepare_table_cache(i, keys->len - i);
	search_bigram(iter, i, npinyin),
	    search_unigram(iter, i, npinyin);
	delete iter;
    }
    return final_step(results);
}

bool PinyinLookup::search_unigram(IBranchIterator * iter, int nstep, int npinyin){
    lookup_constraint_t* constraint = &g_array_index(m_constraints, lookup_constraint_t, nstep);
    if ( CONSTRAINT_NOSEARCH == constraint->m_type )
	return false;
    GArray * lookup_content = (GArray *) g_ptr_array_index(m_steps_content, nstep);
    if ( 0 == lookup_content->len )
	return false;
    lookup_value_t max_step = iter->max();
    if ( CONSTRAINT_ONESTEP == constraint->m_type){
	    return unigram_gen_next_step(nstep, &max_step, constraint->m_token);
    }
    if ( NO_CONSTRAINT == constraint->m_type ){
	bool found = false;
	for ( size_t i = 1; i < m_table_cache->len && i <= MAX_PHRASE_LENGTH; ++i){
	lookup_constraint_t * constraint = &g_array_index(m_constraints, lookup_constraint_t, nstep + i - 1);
	if ( constraint->m_type != NO_CONSTRAINT )
	    continue;
	    PhraseIndexRanges * ranges = &g_array_index(m_table_cache,PhraseIndexRanges, i);
	    for ( size_t m = 0; m < PHRASE_INDEX_LIBRARY_COUNT; ++m){
		GArray * array = (*ranges)[m];
		if ( !array ) continue;
		for ( size_t n = 0; n < array->len; ++n){
		    PhraseIndexRange * range = &g_array_index(array, PhraseIndexRange, n);
		    for ( phrase_token_t token = range->m_range_begin; 
			  token != range->m_range_end; ++token){
			found = unigram_gen_next_step(nstep, &max_step, token)|| found;
		    }  
		}
	    }
	}
	return found;
    }
    return false;
}


bool PinyinLookup::search_bigram(IBranchIterator * iter, 
				 int nstep, int npinyin){
    lookup_constraint_t* constraint = &g_array_index(m_constraints, lookup_constraint_t, nstep);
    if ( CONSTRAINT_NOSEARCH == constraint->m_type )
	return false;
    GArray * lookup_content = (GArray *) g_ptr_array_index(m_steps_content, nstep);

    bool found = false;
    BigramPhraseArray bigram_phrase_items = g_array_new(FALSE, FALSE, 
					       sizeof(BigramPhraseItem));
    while ( iter->has_next() ){
	lookup_value_t cur_step = iter->next();
	//printf("token:%d\t%d\n", cur_step.m_handles[0], cur_step.m_handles[1]);
	phrase_token_t index_token = cur_step.m_handles[1];
	SingleGram * system, * user;
	m_bigram->load(index_token, system, user);
	if ( system && user ){
	    guint32 total_freq;
	    assert(user->get_total_freq(total_freq));
	    assert(system->set_total_freq(total_freq));
	}
	if ( CONSTRAINT_ONESTEP == constraint->m_type ){
	    phrase_token_t token = constraint->m_token;
	    if ( system ){
		guint32 freq;
		if( system->get_freq(token, freq) ){
		    guint32 total_freq;
		    system->get_total_freq(total_freq);
		    gfloat bigram_poss = freq / (gfloat) total_freq;
		    found =  bigram_gen_next_step(nstep, &cur_step, token, bigram_poss) || found;
		}
	    }
	    if ( user ){
		guint32 freq;
		if( user->get_freq(token, freq)){
		    guint32 total_freq;
		    user->get_total_freq(total_freq);
		    gfloat bigram_poss = freq / (gfloat) total_freq;
		    found = bigram_gen_next_step(nstep, &cur_step, token, bigram_poss) || found;
		}
	    }
	}

	if ( NO_CONSTRAINT == constraint->m_type ){
	    for ( size_t i = 1; i < m_table_cache->len 
		      && i <= MAX_PHRASE_LENGTH;++i ){
                lookup_constraint_t * constraint = &g_array_index(m_constraints, lookup_constraint_t, nstep + i - 1);
                if ( constraint->m_type != NO_CONSTRAINT )
                     continue;

		PhraseIndexRanges * ranges = &g_array_index(m_table_cache, PhraseIndexRanges, i);
		for( size_t m = 0; m < PHRASE_INDEX_LIBRARY_COUNT; ++m){
		    GArray * array = (*ranges)[m];
		    if ( !array ) continue;
		    for ( size_t n = 0; n < array->len; ++n){
			PhraseIndexRange * range = &g_array_index(array, PhraseIndexRange, n);
			if (system){
			    g_array_set_size(bigram_phrase_items, 0);
			    system->search(range, bigram_phrase_items);
			    for( size_t k = 0; k < bigram_phrase_items->len; 
				 ++k){
				BigramPhraseItem * item = &g_array_index(bigram_phrase_items, BigramPhraseItem, k);
				found = bigram_gen_next_step(nstep, &cur_step, item->m_token, item->m_freq) || found;
			    }
			}
			if (user){
			    g_array_set_size(bigram_phrase_items, 0);
			    user->search(range, bigram_phrase_items);
			    for( size_t k  = 0; k < bigram_phrase_items->len;
				 ++k){
				BigramPhraseItem * item = &g_array_index(bigram_phrase_items, BigramPhraseItem, k);
				found = bigram_gen_next_step(nstep, &cur_step, item->m_token, item->m_freq) || found;
			    }
			}
		    }
		}
	    }
	}
	if (system)
	    delete system;
	if (user)
	    delete user;
    }
    g_array_free(bigram_phrase_items, TRUE);
    return found;
}


bool PinyinLookup::unigram_gen_next_step(int nstep, lookup_value_t * cur_step, phrase_token_t token){
    PinyinKey * pinyinkeys = ((PinyinKey *)m_keys->data) + nstep;
    if (!m_phrase_index->get_phrase_item(token, m_cache_phrase_item))
	return false;
    size_t phrase_length = m_cache_phrase_item.get_phrase_length();
    gfloat elem_poss = m_cache_phrase_item.get_unigram_frequency() / (gfloat)
	m_phrase_index->get_phrase_index_total_freq();
    if ( elem_poss < FLT_EPSILON )
	return false;
    gfloat pinyin_poss = m_cache_phrase_item.get_pinyin_possibility(*m_custom, pinyinkeys);
    if (pinyin_poss < FLT_EPSILON )
	return false;
    lookup_value_t next_step;
    next_step.m_handles[0] = cur_step->m_handles[1]; next_step.m_handles[1] = token;
    next_step.m_poss = cur_step->m_poss + log(elem_poss * pinyin_poss * unigram_lambda);
    next_step.m_last_step = nstep;
    
    return save_next_step(nstep + phrase_length, cur_step, &next_step);
}

bool PinyinLookup::bigram_gen_next_step(int nstep, lookup_value_t * cur_step, phrase_token_t token, gfloat bigram_poss){
    PinyinKey * pinyinkeys = ((PinyinKey *)m_keys->data) + nstep;
    if (!m_phrase_index->get_phrase_item(token, m_cache_phrase_item))
	return false;
    size_t phrase_length = m_cache_phrase_item.get_phrase_length();
    gfloat unigram_poss = m_cache_phrase_item.get_unigram_frequency() / (gfloat)
	m_phrase_index->get_phrase_index_total_freq();
    if ( bigram_poss < FLT_EPSILON && unigram_poss < FLT_EPSILON )
	return false;
    gfloat pinyin_poss = m_cache_phrase_item.get_pinyin_possibility(*m_custom, pinyinkeys);
    if ( pinyin_poss < FLT_EPSILON )
	return false;
    lookup_value_t next_step;
    next_step.m_handles[0] = cur_step->m_handles[1]; next_step.m_handles[1] = token;
    next_step.m_poss = cur_step->m_poss + 
	log(( bigram_lambda * bigram_poss + unigram_lambda * unigram_poss) *pinyin_poss);
    next_step.m_last_step = nstep;
    
    return save_next_step(nstep + phrase_length, cur_step, &next_step);
}

bool PinyinLookup::save_next_step(int next_step_pos, lookup_value_t * cur_step, lookup_value_t * next_step){
    lookup_key_t next_key = next_step->m_handles[1];
    GHashTable * next_lookup_index = (GHashTable *) g_ptr_array_index(m_steps_index, next_step_pos);
    GArray * next_lookup_content = (GArray *) g_ptr_array_index(m_steps_content, next_step_pos);
    
    gpointer key, value;
    gboolean lookup_result = g_hash_table_lookup_extended(next_lookup_index, GUINT_TO_POINTER(next_key), &key, &value);
    size_t step_index = GPOINTER_TO_UINT(value);
    if ( !lookup_result ){
	g_array_append_val(next_lookup_content, *next_step);
	g_hash_table_insert(next_lookup_index, GUINT_TO_POINTER(next_key), GUINT_TO_POINTER(next_lookup_content->len - 1));
	return true;
    }else{
	lookup_value_t * orig_next_value = &g_array_index(next_lookup_content, lookup_value_t,step_index);
	if ( orig_next_value->m_poss < next_step->m_poss) {
	    orig_next_value->m_handles[0] = next_step->m_handles[0];
	    assert(orig_next_value->m_handles[1] == next_step->m_handles[1]);
	    orig_next_value->m_poss = next_step->m_poss;
	    orig_next_value->m_last_step = next_step->m_last_step;
	    return true;
	}
	return false;
    }
}

bool PinyinLookup::final_step(MatchResults & results){
    //reset results
    g_array_set_size(results, m_steps_content->len);
    for ( size_t i = 0 ; i < m_steps_content->len ; ++i){
	phrase_token_t * token = &g_array_index(results, phrase_token_t, i);
	*token = NULL;
    }
    //find max element
    size_t last_step_pos = m_steps_content->len - 1;
    
    GArray * last_step_array = (GArray *)g_ptr_array_index(m_steps_content, last_step_pos);
    if ( last_step_array->len == 0 )
	return false;
    lookup_value_t * max_value = &g_array_index(last_step_array, lookup_value_t, 0);
    for ( size_t i = 1; i < last_step_array->len; ++i){
	lookup_value_t * cur_value = &g_array_index(last_step_array, lookup_value_t, i);
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
	
	
	GHashTable * lookup_step_index = (GHashTable *)g_ptr_array_index(m_steps_index, cur_step_pos);
	gpointer key, value;
	gboolean result = g_hash_table_lookup_extended(lookup_step_index, GUINT_TO_POINTER(last_token), &key, &value);
	if (!result)
	    return false;
	GArray * lookup_step_content = (GArray *)g_ptr_array_index(m_steps_content, cur_step_pos);

	max_value = &g_array_index(lookup_step_content, lookup_value_t, GPOINTER_TO_UINT(value));
    }
    
    //no need to reverse the result
    
    return true;
}

bool PinyinLookup::train_result(PinyinKeyVector keys, CandidateConstraints constraints, MatchResults & results){
    bool train_next = false;
    PinyinKey * pinyin_keys = (PinyinKey *)keys->data;
    //TODO: verify the new training method.
    phrase_token_t last_token = sentence_start;
    // constraints->len + 1 == results->len
    guint32 train_factor = 23;
    for ( size_t i = 0; i < constraints->len; ++i){
	phrase_token_t * token = &g_array_index(results, phrase_token_t, i);
	if ( *token == NULL )
	    continue;
	lookup_constraint_t * constraint = &g_array_index(constraints, lookup_constraint_t, i);
	if (train_next || CONSTRAINT_ONESTEP == constraint->m_type ){
	    if (CONSTRAINT_ONESTEP == constraint->m_type){
		assert(*token == constraint->m_token);
		train_next = true;
	    }else{
		train_next = false;
	    }
	    //add pi-gram frequency
	    //std::cout<<"i:"<<i<<"last_token:"<<last_token<<"\ttoken:"<<*token<<std::endl;
	    m_phrase_index->get_phrase_item(*token, m_cache_phrase_item);
	    m_cache_phrase_item.increase_pinyin_possibility(*m_custom, pinyin_keys + i, train_factor);
	    m_phrase_index->add_unigram_frequency(*token, train_factor);
	    if ( last_token ){
		SingleGram * system, *user;
		m_bigram->load(last_token, system, user);
		guint32 total_freq;
		if ( !user ){
		    total_freq = 0;
		    if ( system )
			assert(system->get_total_freq(total_freq));
		    user = new SingleGram;
		    user->set_total_freq(total_freq);
		}
		guint32 freq = 0;
		if ( !user->get_freq(*token, freq)){
		    if (system) system->get_freq(*token, freq);
		    user->set_freq(*token, freq);
		}
		assert(user->get_total_freq(total_freq));
		//protect against total_freq overflow.
		if ( train_factor > 0 && total_freq > total_freq + train_factor)
		    goto next;
		assert(user->set_total_freq(total_freq + train_factor));
		assert(user->get_freq(*token, freq));
		//if total_freq is not overflow, then freq won't overflow.
		assert(user->set_freq(*token, freq + train_factor));
		assert(m_bigram->store(last_token, user));
	    next:
		if (system) delete system;
		if (user) delete user;
	    }
	}
	last_token = *token;
    }
    return true;
}

bool PinyinLookup::convert_to_utf8(MatchResults results, /* out */ char * & result_string){
    result_string = g_strdup("");
    for ( size_t i = 0; i < results->len; ++i){
	phrase_token_t * token = &g_array_index(results, phrase_token_t, i);
	if ( NULL == *token )
	    continue;
	m_phrase_index->get_phrase_item(*token, m_cache_phrase_item);
	utf16_t buffer[MAX_PHRASE_LENGTH];
	m_cache_phrase_item.get_phrase_string(buffer);
	guint8 length = m_cache_phrase_item.get_phrase_length();
	gchar * phrase = g_utf16_to_utf8(buffer, length, NULL, NULL, NULL);
	char * tmp = result_string;
	result_string = g_strconcat(result_string, phrase, NULL);
	g_free(tmp); g_free(phrase);
    }
    return true;
}

bool PinyinLookup::add_constraint(CandidateConstraints constraints, size_t index, phrase_token_t token){
    if ( !m_phrase_index->get_phrase_item(token, m_cache_phrase_item) )
	return false;

    size_t phrase_length = m_cache_phrase_item.get_phrase_length();
    if ( index + phrase_length > constraints->len )
	return false;

    for ( size_t i = index; i < index + phrase_length ; ++i ){
	clear_constraint(constraints, i);
    }

    lookup_constraint_t * constraint = &g_array_index(constraints, lookup_constraint_t, index);
    constraint->m_type = CONSTRAINT_ONESTEP;
    constraint->m_token = token;
    
    for (size_t i = 1; i < phrase_length; ++i){
	constraint = &g_array_index(constraints, lookup_constraint_t, index + i);
	constraint->m_type = CONSTRAINT_NOSEARCH;
	constraint->m_constraint_step = index;
    }
	return true;
}

bool PinyinLookup::clear_constraint(CandidateConstraints constraints, size_t index){
    if ( index < 0 || index >= constraints->len )
	return false;
    lookup_constraint_t * constraint = &g_array_index(constraints, lookup_constraint_t, index);
    if (constraint->m_type == NO_CONSTRAINT)
	return false;
    if (constraint->m_type == CONSTRAINT_NOSEARCH){
	index = constraint->m_constraint_step;
	constraint = &g_array_index(constraints, lookup_constraint_t, index);
    }
    
    assert(constraint->m_type == CONSTRAINT_ONESTEP);    

    phrase_token_t token = constraint->m_token;
    if (!m_phrase_index->get_phrase_item(token, m_cache_phrase_item))
	return false;

    size_t phrase_length = m_cache_phrase_item.get_phrase_length();
    for ( size_t i = 0; i < phrase_length; ++i){
	if ( index + i >= constraints->len )
	    continue;
	constraint = &g_array_index(constraints, lookup_constraint_t, index + i);
	constraint->m_type = NO_CONSTRAINT;
    }
	return true;
}

bool PinyinLookup::validate_constraint(CandidateConstraints constraints, PinyinKeyVector m_parsed_keys){
    //resize constraints array
    size_t constraints_length = constraints->len;
    if ( m_parsed_keys->len > constraints_length ){
	g_array_set_size(constraints, m_parsed_keys->len);
	//initialize new element
	for( size_t i = constraints_length; i < m_parsed_keys->len; ++i){
	    lookup_constraint_t * constraint = &g_array_index(constraints, lookup_constraint_t, i);
	    constraint->m_type = NO_CONSTRAINT;
	}
    }else if (m_parsed_keys->len < constraints_length ){
	g_array_set_size(constraints, m_parsed_keys->len);
    }
    
    PinyinKey * pinyin_keys = (PinyinKey *)m_parsed_keys->data;
    
    for ( size_t i = 0; i < constraints->len; ++i){
	lookup_constraint_t * constraint = &g_array_index(constraints, lookup_constraint_t, i);
	if ( constraint->m_type == CONSTRAINT_ONESTEP ){
	    phrase_token_t token = constraint->m_token;
	    m_phrase_index->get_phrase_item(token, m_cache_phrase_item);
	    size_t phrase_length = m_cache_phrase_item.get_phrase_length();
	    //clear too long constraint
	    if ( i + phrase_length > constraints->len ){
		clear_constraint(constraints, i);
		continue;
	    }
	    //clear invalidated pinyin
	    gfloat pinyin_poss = m_cache_phrase_item.get_pinyin_possibility(*m_custom, pinyin_keys + i);
	    if ( pinyin_poss < FLT_EPSILON ){
		clear_constraint(constraints, i);
	    }
	}
    }
    return true;
}
