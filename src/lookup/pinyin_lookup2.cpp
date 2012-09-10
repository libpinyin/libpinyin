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

const gfloat PinyinLookup2::bigram_lambda = LAMBDA_PARAMETER;
const gfloat PinyinLookup2::unigram_lambda = 1 - LAMBDA_PARAMETER;


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

    dump_max_value(candidates);

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

    dump_all_values(topresults);

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

        GArray * initial_step_content = (GArray *)
            g_ptr_array_index(steps_content, 0);
        initial_step_content = g_array_append_val
            (initial_step_content, initial_value);

        GHashTable * initial_step_index = (GHashTable *)
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
	/* initialize m_steps_index */
	g_ptr_array_index(steps_index, i) = g_hash_table_new(g_direct_hash, g_direct_equal);
	/* initialize m_steps_content */
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


PinyinLookup2::PinyinLookup2(pinyin_option_t options,
                             FacadeChewingTable * pinyin_table,
                             FacadePhraseIndex * phrase_index,
                             Bigram * system_bigram,
                             Bigram * user_bigram){
    m_options = options;
    m_pinyin_table = pinyin_table;
    m_phrase_index = phrase_index;
    m_system_bigram = system_bigram;
    m_user_bigram = user_bigram;

    m_steps_index = g_ptr_array_new();
    m_steps_content = g_ptr_array_new();
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

            populate_candidates(candidates, step);
            get_top_results(topresults, candidates);

            search_bigram2(topresults, i, ranges),
                search_unigram2(topresults, i, ranges);

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
    if (0 == topresults->len)
        return false;

    lookup_constraint_t* constraint =
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
