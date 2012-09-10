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

