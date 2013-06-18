/* 
 *  libpinyin
 *  Library to deal with pinyin.
 *  
 *  Copyright (C) 2012 Peng Wu <alexepico@gmail.com>
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
#include "facade_chewing_table.h"
#include "pinyin_lookup2.h"
#include "stl_lite.h"

using namespace pinyin;

/*
const gfloat PinyinLookup2::bigram_lambda = lambda;
const gfloat PinyinLookup2::unigram_lambda = 1 - lambda;
*/

/* internal definition */
static const size_t nbeam = 32;

static bool dump_max_value(GPtrArray * values){
    if (0 == values->len)
        return false;

    const lookup_value_t * max =
        (const lookup_value_t *) g_ptr_array_index(values, 0);

    for (size_t i = 1; i < values->len; ++i) {
        const lookup_value_t * cur =
            (const lookup_value_t *) g_ptr_array_index(values, i);

        if (cur->m_poss > max->m_poss)
            max = cur;
    }

    printf("max value: %f\n", max->m_poss);

    return true;
}

static bool dump_all_values(GPtrArray * values) {
    if (0 == values->len)
        return false;

    printf("values:");
    for (size_t i = 0; i < values->len; ++i) {
        const lookup_value_t * cur =
            (const lookup_value_t *) g_ptr_array_index(values, i);

        printf("%f\t", cur->m_poss);
    }
    printf("\n");

    return true;
}

/* populate the candidates. */
static bool populate_candidates(/* out */ GPtrArray * candidates,
                                /* in */ LookupStepContent step) {
    g_ptr_array_set_size(candidates, 0);

    if (0 == step->len)
        return false;

    for (size_t i = 0; i < step->len; ++i) {
        lookup_value_t * value = &g_array_index
            (step, lookup_value_t, i);

        g_ptr_array_add(candidates, value);
    }

    /* dump_max_value(candidates); */

    return true;
}

static bool lookup_value_less_than(lookup_value_t * lhs, lookup_value_t * rhs){
    return lhs->m_poss < rhs->m_poss;
}

/* use maximum heap to get the topest results. */
static bool get_top_results(/* out */ GPtrArray * topresults,
                            /* in */ GPtrArray * candidates) {
    g_ptr_array_set_size(topresults, 0);

    if (0 == candidates->len)
        return false;

    lookup_value_t ** begin =
        (lookup_value_t **) &g_ptr_array_index(candidates, 0);
    lookup_value_t ** end =
        (lookup_value_t **) &g_ptr_array_index(candidates, candidates->len);

    std_lite::make_heap(begin, end, lookup_value_less_than);

    while (end != begin) {
        lookup_value_t * one = *begin;
        g_ptr_array_add(topresults, one);

        std_lite::pop_heap(begin, end, lookup_value_less_than);
        --end;

        if (topresults->len >= nbeam)
            break;
    }

    /* dump_all_values(topresults); */

    return true;
}

static bool populate_prefixes(GPtrArray * steps_index,
                              GPtrArray * steps_content,
                              TokenVector prefixes) {
    assert(prefixes->len > 0);

    for (size_t i = 0; i < prefixes->len; ++i) {
        phrase_token_t token = g_array_index(prefixes, phrase_token_t, i);
        lookup_key_t initial_key = token;
        lookup_value_t initial_value(log(1));
        initial_value.m_handles[1] = token;

        LookupStepContent initial_step_content = (LookupStepContent)
            g_ptr_array_index(steps_content, 0);
        initial_step_content = g_array_append_val
            (initial_step_content, initial_value);

        LookupStepIndex initial_step_index = (LookupStepIndex)
            g_ptr_array_index(steps_index, 0);
        g_hash_table_insert(initial_step_index,
                            GUINT_TO_POINTER(initial_key),
                            GUINT_TO_POINTER(initial_step_content->len - 1));
    }

    return true;
}

static bool init_steps(GPtrArray * steps_index,
                       GPtrArray * steps_content,
                       int nstep){
    /* add null start step */
    g_ptr_array_set_size(steps_index, nstep);
    g_ptr_array_set_size(steps_content, nstep);

    for (int i = 0; i < nstep; ++i) {
	/* initialize steps_index */
	g_ptr_array_index(steps_index, i) = g_hash_table_new(g_direct_hash, g_direct_equal);
	/* initialize steps_content */
	g_ptr_array_index(steps_content, i) = g_array_new(FALSE, FALSE, sizeof(lookup_value_t));
    }

    return true;
}

static void clear_steps(GPtrArray * steps_index, GPtrArray * steps_content){
    /* clear steps_index */
    for ( size_t i = 0; i < steps_index->len; ++i){
	GHashTable * table = (GHashTable *) g_ptr_array_index(steps_index, i);
	g_hash_table_destroy(table);
	g_ptr_array_index(steps_index, i) = NULL;
    }

    /* clear steps_content */
    for ( size_t i = 0; i < steps_content->len; ++i){
	GArray * array = (GArray *) g_ptr_array_index(steps_content, i);
	g_array_free(array, TRUE);
	g_ptr_array_index(steps_content, i) = NULL;
    }
}


PinyinLookup2::PinyinLookup2(const gfloat lambda,
                             pinyin_option_t options,
                             FacadeChewingTable * pinyin_table,
                             FacadePhraseIndex * phrase_index,
                             Bigram * system_bigram,
                             Bigram * user_bigram)
    : bigram_lambda(lambda),
      unigram_lambda(1. - lambda)
{
    m_options = options;
    m_pinyin_table = pinyin_table;
    m_phrase_index = phrase_index;
    m_system_bigram = system_bigram;
    m_user_bigram = user_bigram;

    m_steps_index = g_ptr_array_new();
    m_steps_content = g_ptr_array_new();

    /* the member variables below are saved in get_best_match call. */
    m_keys = NULL;
    m_constraints = NULL;
}

PinyinLookup2::~PinyinLookup2(){
    clear_steps(m_steps_index, m_steps_content);
    g_ptr_array_free(m_steps_index, TRUE);
    g_ptr_array_free(m_steps_content, TRUE);
}


bool PinyinLookup2::get_best_match(TokenVector prefixes,
                                   ChewingKeyVector keys,
                                   CandidateConstraints constraints,
                                   MatchResults & results){
    m_constraints = constraints;
    m_keys = keys;
    int nstep = keys->len + 1;

    clear_steps(m_steps_index, m_steps_content);

    init_steps(m_steps_index, m_steps_content, nstep);

    populate_prefixes(m_steps_index, m_steps_content, prefixes);

    PhraseIndexRanges ranges;
    memset(ranges, 0, sizeof(PhraseIndexRanges));
    m_phrase_index->prepare_ranges(ranges);

    GPtrArray * candidates = g_ptr_array_new();
    GPtrArray * topresults = g_ptr_array_new();

    /* begin the viterbi beam search. */
    for ( int i = 0; i < nstep - 1; ++i ){
        lookup_constraint_t * cur_constraint = &g_array_index
            (m_constraints, lookup_constraint_t, i);

        if (CONSTRAINT_NOSEARCH == cur_constraint->m_type)
            continue;

        LookupStepContent step = (LookupStepContent)
            g_ptr_array_index(m_steps_content, i);

        populate_candidates(candidates, step);
        get_top_results(topresults, candidates);

        if (0 == topresults->len)
            continue;

        for ( int m = i + 1; m < nstep; ++m ){
            const int len = m - i;
            if (len > MAX_PHRASE_LENGTH)
                break;

            lookup_constraint_t * next_constraint = &g_array_index
                (m_constraints, lookup_constraint_t, m - 1);

            if (CONSTRAINT_NOSEARCH == next_constraint->m_type)
                break;

            ChewingKey * pinyin_keys = (ChewingKey *)m_keys->data;
            /* do one pinyin table search. */
            int result = m_pinyin_table->search(len, pinyin_keys + i, ranges);

            if (result & SEARCH_OK) {
                /* assume topresults always contains items. */
                search_bigram2(topresults, i, ranges),
                    search_unigram2(topresults, i, ranges);
            }

            /* poke the next constraint. */
            ++ next_constraint;
            if (CONSTRAINT_ONESTEP == next_constraint->m_type)
                break;

            /* no longer pinyin */
            if (!(result & SEARCH_CONTINUED))
                break;
        }
    }

    m_phrase_index->destroy_ranges(ranges);

    g_ptr_array_free(candidates, TRUE);
    g_ptr_array_free(topresults, TRUE);

    return final_step(results);
}

bool PinyinLookup2::search_unigram2(GPtrArray * topresults, int nstep,
                                    PhraseIndexRanges ranges) {

    if (0 == topresults->len)
        return false;

    lookup_value_t * max = (lookup_value_t *)
        g_ptr_array_index(topresults, 0);

    lookup_constraint_t * constraint =
        &g_array_index(m_constraints, lookup_constraint_t, nstep);

    if (CONSTRAINT_ONESTEP == constraint->m_type) {
        return unigram_gen_next_step(nstep, max, constraint->m_token);
    }

    bool found = false;

    if (NO_CONSTRAINT == constraint->m_type) {
        for ( size_t m = 0; m < PHRASE_INDEX_LIBRARY_COUNT; ++m){
            GArray * array = ranges[m];
            if ( !array ) continue;

            for ( size_t n = 0; n < array->len; ++n){
                PhraseIndexRange * range = &g_array_index(array, PhraseIndexRange, n);
                for ( phrase_token_t token = range->m_range_begin;
                      token != range->m_range_end; ++token){
                    found = unigram_gen_next_step(nstep, max, token)|| found;
                }
            }
        }
    }

    return found;
}

bool PinyinLookup2::search_bigram2(GPtrArray * topresults, int nstep,
                                   PhraseIndexRanges ranges) {

    lookup_constraint_t * constraint =
        &g_array_index(m_constraints, lookup_constraint_t, nstep);

    bool found = false;
    BigramPhraseArray bigram_phrase_items = g_array_new
        (FALSE, FALSE, sizeof(BigramPhraseItem));

    for (size_t i = 0; i < topresults->len; ++i) {
        lookup_value_t * value = (lookup_value_t *)
            g_ptr_array_index(topresults, i);

        phrase_token_t index_token = value->m_handles[1];

	SingleGram * system = NULL, * user = NULL;
	m_system_bigram->load(index_token, system);
        m_user_bigram->load(index_token, user);

        if ( !merge_single_gram(&m_merged_single_gram, system, user) )
            continue;

	if ( CONSTRAINT_ONESTEP == constraint->m_type ){
	    phrase_token_t token = constraint->m_token;

            guint32 freq;
            if( m_merged_single_gram.get_freq(token, freq) ){
                guint32 total_freq;
                m_merged_single_gram.get_total_freq(total_freq);
                gfloat bigram_poss = freq / (gfloat) total_freq;
                found = bigram_gen_next_step(nstep, value, token, bigram_poss) || found;
            }
	}

        if (NO_CONSTRAINT == constraint->m_type) {
            for( size_t m = 0; m < PHRASE_INDEX_LIBRARY_COUNT; ++m){
                GArray * array = ranges[m];
                if ( !array ) continue;

                for ( size_t n = 0; n < array->len; ++n){
                    PhraseIndexRange * range =
                        &g_array_index(array, PhraseIndexRange, n);

                    g_array_set_size(bigram_phrase_items, 0);
                    m_merged_single_gram.search(range, bigram_phrase_items);
                    for( size_t k = 0; k < bigram_phrase_items->len; ++k) {
                        BigramPhraseItem * item = &g_array_index(bigram_phrase_items, BigramPhraseItem, k);
                        found = bigram_gen_next_step(nstep, value, item->m_token, item->m_freq) || found;
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


bool PinyinLookup2::unigram_gen_next_step(int nstep,
                                          lookup_value_t * cur_step,
                                          phrase_token_t token) {

    if (m_phrase_index->get_phrase_item(token, m_cache_phrase_item))
	return false;

    size_t phrase_length = m_cache_phrase_item.get_phrase_length();
    gdouble elem_poss = m_cache_phrase_item.get_unigram_frequency() / (gdouble)
	m_phrase_index->get_phrase_index_total_freq();
    if ( elem_poss < DBL_EPSILON )
	return false;

    ChewingKey * pinyin_keys = ((ChewingKey *)m_keys->data) + nstep;
    gfloat pinyin_poss = m_cache_phrase_item.get_pronunciation_possibility(m_options, pinyin_keys);
    if (pinyin_poss < FLT_EPSILON )
	return false;

    lookup_value_t next_step;
    next_step.m_handles[0] = cur_step->m_handles[1]; next_step.m_handles[1] = token;
    next_step.m_poss = cur_step->m_poss + log(elem_poss * pinyin_poss * unigram_lambda);
    next_step.m_last_step = nstep;

    return save_next_step(nstep + phrase_length, cur_step, &next_step);
}

bool PinyinLookup2::bigram_gen_next_step(int nstep,
                                         lookup_value_t * cur_step,
                                         phrase_token_t token,
                                         gfloat bigram_poss) {

    if (m_phrase_index->get_phrase_item(token, m_cache_phrase_item))
	return false;

    size_t phrase_length = m_cache_phrase_item.get_phrase_length();
    gdouble unigram_poss = m_cache_phrase_item.get_unigram_frequency() /
        (gdouble) m_phrase_index->get_phrase_index_total_freq();
    if ( bigram_poss < FLT_EPSILON && unigram_poss < DBL_EPSILON )
	return false;

    ChewingKey * pinyin_keys = ((ChewingKey *)m_keys->data) + nstep;
    gfloat pinyin_poss = m_cache_phrase_item.get_pronunciation_possibility(m_options, pinyin_keys);
    if ( pinyin_poss < FLT_EPSILON )
	return false;

    lookup_value_t next_step;
    next_step.m_handles[0] = cur_step->m_handles[1]; next_step.m_handles[1] = token;
    next_step.m_poss = cur_step->m_poss +
	log((bigram_lambda * bigram_poss + unigram_lambda * unigram_poss) * pinyin_poss);
    next_step.m_last_step = nstep;

    return save_next_step(nstep + phrase_length, cur_step, &next_step);
}

bool PinyinLookup2::save_next_step(int next_step_pos,
                                   lookup_value_t * cur_step,
                                   lookup_value_t * next_step){

    lookup_key_t next_key = next_step->m_handles[1];
    LookupStepIndex next_lookup_index = (LookupStepIndex)
        g_ptr_array_index(m_steps_index, next_step_pos);
    LookupStepContent next_lookup_content = (LookupStepContent)
        g_ptr_array_index(m_steps_content, next_step_pos);

    gpointer key = NULL, value = NULL;
    gboolean lookup_result = g_hash_table_lookup_extended
        (next_lookup_index, GUINT_TO_POINTER(next_key), &key, &value);

    if ( !lookup_result ){
	g_array_append_val(next_lookup_content, *next_step);
	g_hash_table_insert(next_lookup_index, GUINT_TO_POINTER(next_key), GUINT_TO_POINTER(next_lookup_content->len - 1));
	return true;
    }else{
        size_t step_index = GPOINTER_TO_UINT(value);
	lookup_value_t * orig_next_value = &g_array_index
            (next_lookup_content, lookup_value_t, step_index);

	if ( orig_next_value->m_poss < next_step->m_poss) {
            /* found better result. */
	    orig_next_value->m_handles[0] = next_step->m_handles[0];
	    assert(orig_next_value->m_handles[1] == next_step->m_handles[1]);
	    orig_next_value->m_poss = next_step->m_poss;
	    orig_next_value->m_last_step = next_step->m_last_step;
	    return true;
	}

	return false;
    }
}

bool PinyinLookup2::final_step(MatchResults & results){

    /* reset results */
    g_array_set_size(results, m_steps_content->len - 1);
    for (size_t i = 0; i < results->len; ++i){
	phrase_token_t * token = &g_array_index(results, phrase_token_t, i);
	*token = null_token;
    }

    /* find max element */
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

    /* backtracing */
    while( true ){
	int cur_step_pos = max_value->m_last_step;
	if ( -1 == cur_step_pos )
	    break;

	phrase_token_t * token = &g_array_index
            (results, phrase_token_t, cur_step_pos);
	*token = max_value->m_handles[1];

	phrase_token_t last_token = max_value->m_handles[0];
	LookupStepIndex lookup_step_index = (LookupStepIndex)
            g_ptr_array_index(m_steps_index, cur_step_pos);

	gpointer key = NULL, value = NULL;
	gboolean result = g_hash_table_lookup_extended
            (lookup_step_index, GUINT_TO_POINTER(last_token), &key, &value);
	if (!result)
	    return false;

	LookupStepContent lookup_step_content = (LookupStepContent)
            g_ptr_array_index(m_steps_content, cur_step_pos);
	max_value = &g_array_index
            (lookup_step_content, lookup_value_t, GPOINTER_TO_UINT(value));
    }

    /* no need to reverse the result */
    return true;
}


bool PinyinLookup2::train_result2(ChewingKeyVector keys,
                                  CandidateConstraints constraints,
                                  MatchResults results) {
    const guint32 initial_seed = 23 * 3;
    const guint32 expand_factor = 2;
    const guint32 unigram_factor = 7;
    const guint32 pinyin_factor = 1;
    const guint32 ceiling_seed = 23 * 15 * 64;

    /* begin training based on constraints and results. */
    bool train_next = false;
    ChewingKey * pinyin_keys = (ChewingKey *) keys->data;

    phrase_token_t last_token = sentence_start;
    /* constraints->len + 1 == results->len */
    for (size_t i = 0; i < constraints->len; ++i) {
        phrase_token_t * token = &g_array_index(results, phrase_token_t, i);
        if (null_token == *token)
            continue;

        lookup_constraint_t * constraint = &g_array_index
            (constraints, lookup_constraint_t, i);
        if (train_next || CONSTRAINT_ONESTEP == constraint->m_type) {
            if (CONSTRAINT_ONESTEP == constraint->m_type) {
                assert(*token == constraint->m_token);
                train_next = true;
            } else {
                train_next = false;
            }

            guint32 seed = initial_seed;
            /* train bi-gram first, and get train seed. */
            if (last_token) {
                SingleGram * user = NULL;
                m_user_bigram->load(last_token, user);

                guint32 total_freq = 0;
                if (!user) {
                    user = new SingleGram;
                }
                assert(user->get_total_freq(total_freq));

                guint32 freq = 0;
                /* compute train factor */
                if (!user->get_freq(*token, freq)) {
                    assert(user->insert_freq(*token, 0));
                    seed = initial_seed;
                } else {
                    seed = std_lite::max(freq, initial_seed);
                    seed *= expand_factor;
                    seed = std_lite::min(seed, ceiling_seed);
                }

                /* protect against total_freq overflow */
                if (seed > 0 && total_freq > total_freq + seed)
                    goto next;

                assert(user->set_total_freq(total_freq + seed));
                /* if total_freq is not overflow, then freq won't overflow. */
                assert(user->set_freq(*token, freq + seed));
                assert(m_user_bigram->store(last_token, user));
            next:
                assert(NULL != user);
                if (user)
                    delete user;
            }

            /* train uni-gram */
	    m_phrase_index->get_phrase_item(*token, m_cache_phrase_item);
	    m_cache_phrase_item.increase_pronunciation_possibility
                (m_options, pinyin_keys + i, seed * pinyin_factor);
	    m_phrase_index->add_unigram_frequency
                (*token, seed * unigram_factor);
        }
        last_token = *token;
    }
    return true;
}


int PinyinLookup2::add_constraint(CandidateConstraints constraints,
                                  size_t index,
                                  phrase_token_t token) {

    if (m_phrase_index->get_phrase_item(token, m_cache_phrase_item))
	return 0;

    size_t phrase_length = m_cache_phrase_item.get_phrase_length();
    if ( index + phrase_length > constraints->len )
	return 0;

    for (size_t i = index; i < index + phrase_length; ++i){
	clear_constraint(constraints, i);
    }

    /* store one step constraint */
    lookup_constraint_t * constraint = &g_array_index
        (constraints, lookup_constraint_t, index);
    constraint->m_type = CONSTRAINT_ONESTEP;
    constraint->m_token = token;

    /* propagate no search constraint */
    for (size_t i = 1; i < phrase_length; ++i){
	constraint = &g_array_index(constraints, lookup_constraint_t, index + i);
	constraint->m_type = CONSTRAINT_NOSEARCH;
	constraint->m_constraint_step = index;
    }

    return phrase_length;
}

bool PinyinLookup2::clear_constraint(CandidateConstraints constraints,
                                    int index) {
    if (index < 0 || index >= constraints->len)
	return false;

    lookup_constraint_t * constraint = &g_array_index
        (constraints, lookup_constraint_t, index);

    if (NO_CONSTRAINT == constraint->m_type)
	return false;

    if (CONSTRAINT_NOSEARCH == constraint->m_type){
	index = constraint->m_constraint_step;
	constraint = &g_array_index(constraints, lookup_constraint_t, index);
    }

    /* now var constraint points to the one step constraint. */
    assert(constraint->m_type == CONSTRAINT_ONESTEP);

    phrase_token_t token = constraint->m_token;
    if (m_phrase_index->get_phrase_item(token, m_cache_phrase_item))
	return false;

    size_t phrase_length = m_cache_phrase_item.get_phrase_length();
    for ( size_t i = 0; i < phrase_length; ++i){
	if (index + i >= constraints->len)
	    continue;

	constraint = &g_array_index
            (constraints, lookup_constraint_t, index + i);
	constraint->m_type = NO_CONSTRAINT;
    }

    return true;
}

bool PinyinLookup2::validate_constraint(CandidateConstraints constraints,
                                        ChewingKeyVector keys) {
    /* resize constraints array first */
    size_t constraints_length = constraints->len;

    if ( keys->len > constraints_length ){
	g_array_set_size(constraints, keys->len);

	/* initialize new element */
	for( size_t i = constraints_length; i < keys->len; ++i){
	    lookup_constraint_t * constraint = &g_array_index(constraints, lookup_constraint_t, i);
	    constraint->m_type = NO_CONSTRAINT;
	}

    }else if (keys->len < constraints_length ){
        /* just shrink it */
	g_array_set_size(constraints, keys->len);
    }

    for ( size_t i = 0; i < constraints->len; ++i){
	lookup_constraint_t * constraint = &g_array_index
            (constraints, lookup_constraint_t, i);

        /* handle one step constraint */
	if ( constraint->m_type == CONSTRAINT_ONESTEP ){

	    phrase_token_t token = constraint->m_token;
	    m_phrase_index->get_phrase_item(token, m_cache_phrase_item);
	    size_t phrase_length = m_cache_phrase_item.get_phrase_length();

	    /* clear too long constraint */
	    if (i + phrase_length > constraints->len){
		clear_constraint(constraints, i);
		continue;
	    }

            ChewingKey * pinyin_keys = (ChewingKey *)keys->data;
	    /* clear invalid pinyin */
	    gfloat pinyin_poss = m_cache_phrase_item.get_pronunciation_possibility(m_options, pinyin_keys + i);
	    if (pinyin_poss < FLT_EPSILON)
		clear_constraint(constraints, i);
	}
    }
    return true;
}
