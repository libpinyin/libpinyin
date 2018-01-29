/* 
 *  libpinyin
 *  Library to deal with pinyin.
 *  
 *  Copyright (C) 2017 Peng Wu <alexepico@gmail.com>
 *  
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef PHONETIC_LOOKUP_H
#define PHONETIC_LOOKUP_H


#include "novel_types.h"
#include <limits.h>
#include <math.h>
#include "phonetic_key_matrix.h"
#include "ngram.h"
#include "lookup.h"

namespace pinyin{


/* internal definition */
static const size_t nbeam = 32;

#define LONG_SENTENCE_PENALTY log(1.2f)

struct trellis_value_t {
    phrase_token_t m_handles[2];
    // the character length of the final sentence.
    gint32 m_sentence_length;
    gfloat m_poss;
    // the m_last_step and m_sub_index points to this trellis.
    gint32 m_last_step;
    // the m_sub_index points to the inside of last trellis_node.
    gint32 m_sub_index;
    // the current index in this trellis_node.
    // only initialized in the get_candidates method.
    gint32 m_current_index;

    trellis_value_t(gfloat poss = FLT_MAX){
        m_handles[0] = null_token;
        m_handles[1] = null_token;
        m_sentence_length = 0;
        m_poss = poss;
        m_last_step = -1;
        m_sub_index = -1;
        m_current_index = -1;
    }
};

template <gint32 nstore>
static bool inline trellis_value_less_than(const trellis_value_t * item_lhs,
                                           const trellis_value_t * item_rhs) {
#if 1
    if (nstore > 1) {
        /* allow longer sentence */
        if (item_lhs->m_sentence_length + 1 == item_rhs->m_sentence_length &&
            item_lhs->m_poss + LONG_SENTENCE_PENALTY < item_rhs->m_poss)
            return true;

        if (item_lhs->m_sentence_length == item_rhs->m_sentence_length + 1 &&
            item_lhs->m_poss < item_rhs->m_poss + LONG_SENTENCE_PENALTY)
            return true;
    }
#endif

    /* the same length but better possibility */
    if (item_lhs->m_sentence_length == item_rhs->m_sentence_length &&
        item_lhs->m_poss < item_rhs->m_poss)
        return true;

    /* shorter sentence */
    if (item_lhs->m_sentence_length > item_rhs->m_sentence_length)
        return true;

    return false;
}

#if 0
struct matrix_value_t {
    phrase_token_t m_cur_token;
    /* just propagate the value from tail matrix step. */
    gfloat m_poss;
    // the below information for recovering the final phrase array.
    // the m_next_step and m_next_index points to this matrix.
    gint32 m_next_step;
    // the m_next_index points to the last matrix_step.
    gint32 m_next_index;

    matrix_value_t(){
        m_cur_token = null_token;
        m_poss = FLT_MAX;
        m_next_step = -1;
        m_next_index = -1;
    }
};
#endif

#if 0
/* for debug purpose */
#include "phonetic_lookup_linear.h"
#else
/* for optimization */
#include "phonetic_lookup_heap.h"
#endif

struct trellis_constraint_t {
    /* the constraint type */
    constraint_type m_type;

    // expand the previous union into struct to catch some errors.
    /* for CONSTRAINT_ONESTEP type:
       the token of the word. */
    phrase_token_t m_token;
    /* for CONSTRAINT_ONESTEP type:
       the index of the next word.
       for CONSTRAINT_NOSEARCH type:
       the index of the previous onestep constraint. */
    guint32 m_constraint_step;

    trellis_constraint_t(){
        m_type = NO_CONSTRAINT;
    }
};


/* use maximum heap to get the topest results. */
template<gint32 nstore>
bool get_top_results(size_t num,
                     /* out */ GPtrArray * topresults,
                     /* in */ GPtrArray * candidates) {
    g_ptr_array_set_size(topresults, 0);

    if (0 == candidates->len)
        return false;

    trellis_value_t ** begin =
        (trellis_value_t **) &g_ptr_array_index(candidates, 0);
    trellis_value_t ** end =
        (trellis_value_t **) &g_ptr_array_index(candidates, candidates->len);

    std_lite::make_heap(begin, end, trellis_value_less_than<nstore>);

    while (end != begin) {
        trellis_value_t * one = *begin;
        g_ptr_array_add(topresults, one);

        std_lite::pop_heap(begin, end, trellis_value_less_than<nstore>);
        --end;

        if (topresults->len >= num)
            break;
    }

    /* dump_all_values(topresults); */

    return true;
}

static gint trellis_value_compare(const trellis_value_t ** lhs,
                                  const trellis_value_t ** rhs) {
    /* in descending order */
    return -((*lhs)->m_poss - (*rhs)->m_poss);
}

template <gint32 nstore, gint32 nbest>
class ForwardPhoneticTrellis {
private:
    /* Array of LookupStepIndex */
    GPtrArray * m_steps_index;
    /* Array of LookupStepContent */
    GPtrArray * m_steps_content;

public:
    ForwardPhoneticTrellis() {
        m_steps_index = g_ptr_array_new();
        m_steps_content = g_ptr_array_new();
    }

    ~ForwardPhoneticTrellis() {
        clear();
        g_ptr_array_free(m_steps_index, TRUE);
        m_steps_index = NULL;
        g_ptr_array_free(m_steps_content, TRUE);
        m_steps_content = NULL;
    }

public:
    size_t size() const {
        assert(m_steps_index->len == m_steps_content->len);
        return m_steps_index->len;
    }

    bool clear() {
        /* clear m_steps_index */
        for ( size_t i = 0; i < m_steps_index->len; ++i){
            LookupStepIndex step_index = (LookupStepIndex) g_ptr_array_index(m_steps_index, i);
            g_hash_table_destroy(step_index);
            g_ptr_array_index(m_steps_index, i) = NULL;
        }
        g_ptr_array_set_size(m_steps_index, 0);

        /* clear m_steps_content */
        for ( size_t i = 0; i < m_steps_content->len; ++i){
            LookupStepContent step_content = (LookupStepContent) g_ptr_array_index(m_steps_content, i);
            g_array_free(step_content, TRUE);
            g_ptr_array_index(m_steps_content, i) = NULL;
        }
        g_ptr_array_set_size(m_steps_content, 0);

        return true;
    }

    bool prepare(gint32 nstep) {
        /* add null start step */
        g_ptr_array_set_size(m_steps_index, nstep);
        g_ptr_array_set_size(m_steps_content, nstep);

        for (int i = 0; i < nstep; ++i) {
            /* initialize m_steps_index */
            g_ptr_array_index(m_steps_index, i) = g_hash_table_new(g_direct_hash, g_direct_equal);
            /* initialize m_steps_content */
            g_ptr_array_index(m_steps_content, i) = g_array_new(FALSE, FALSE, sizeof(trellis_node<nstore>));
        }

        return true;
    }

    /* Array of phrase_token_t */
    bool fill_prefixes(/* in */ TokenVector prefixes) {
        assert(prefixes->len > 0);

        for (size_t i = 0; i < prefixes->len; ++i) {
            phrase_token_t token = g_array_index(prefixes, phrase_token_t, i);
            lookup_key_t initial_key = token;
            trellis_value_t initial_value(log(1.f));
            initial_value.m_handles[1] = token;

            trellis_node<nstore> initial_node;
            assert(initial_node.eval_item(&initial_value));

            LookupStepContent initial_step_content = (LookupStepContent)
                g_ptr_array_index(m_steps_content, 0);
            initial_step_content = g_array_append_val
                (initial_step_content, initial_node);

            LookupStepIndex initial_step_index = (LookupStepIndex)
                g_ptr_array_index(m_steps_index, 0);
            g_hash_table_insert(initial_step_index,
                                GUINT_TO_POINTER(initial_key),
                                GUINT_TO_POINTER(initial_step_content->len - 1));
        }

        return true;
    }

    /* PtrArray of trellis_value_t pointer */
    bool get_candidates(/* in */ gint32 index,
                        /* out */ GPtrArray * candidates) const {
        LookupStepContent step = (LookupStepContent)
            g_ptr_array_index(m_steps_content, index);

        g_ptr_array_set_size(candidates, 0);

        if (0 == step->len)
            return false;

        for (size_t i = 0; i < step->len; ++i) {
            trellis_node<nstore> * node = &g_array_index
                (step, trellis_node<nstore>, i);

            // only initialized in the get_candidates method.
            node->number();

            const trellis_value_t * value = node->begin();
            for (; value < node->end(); ++value) {
                g_ptr_array_add(candidates, (trellis_value_t *)value);
            }
        }

        /* dump_max_value(candidates); */

        return true;
    }

    /* insert candidate */
    bool insert_candidate(gint32 index, lookup_key_t token,
                          const trellis_value_t * candidate) {
        LookupStepIndex step_index = (LookupStepIndex) g_ptr_array_index(m_steps_index, index);
        LookupStepContent step_content = (LookupStepContent) g_ptr_array_index(m_steps_content, index);

        gpointer key = NULL, value = NULL;
        gboolean lookup_result = g_hash_table_lookup_extended
            (step_index, GUINT_TO_POINTER(token), &key, &value);

        if (!lookup_result) {
            trellis_node<nstore> node;
            assert(node.eval_item(candidate));

            g_array_append_val(step_content, node);
            g_hash_table_insert(step_index, GUINT_TO_POINTER(token), GUINT_TO_POINTER(step_content->len - 1));
            return true;
        } else {
            size_t node_index = GPOINTER_TO_UINT(value);
            trellis_node<nstore> * node = &g_array_index
                (step_content, trellis_node<nstore>, node_index);

            return node->eval_item(candidate);
        }

        assert(FALSE);
    }

    /* get tails */
    /* Array of trellis_value_t * */
    bool get_tails(/* out */ GPtrArray * tails) const {
        gint32 tail_index = size() - 1;

        GPtrArray * candidates = g_ptr_array_new();
        get_candidates(tail_index, candidates);
        get_top_results<nstore>(nbest, tails, candidates);

        g_ptr_array_sort(tails, (GCompareFunc)trellis_value_compare);

        g_ptr_array_free(candidates, TRUE);
        return true;
    }

    /* get candidate */
    bool get_candidate(gint32 index, lookup_key_t token, gint32 sub_index,
                       const trellis_value_t * & candidate) const {
        LookupStepIndex step_index = (LookupStepIndex) g_ptr_array_index(m_steps_index, index);
        LookupStepContent step_content = (LookupStepContent) g_ptr_array_index(m_steps_content, index);

        gpointer key = NULL, value = NULL;
        gboolean lookup_result = g_hash_table_lookup_extended
            (step_index, GUINT_TO_POINTER(token), &key, &value);

        if (!lookup_result)
            return false;

        size_t node_index = GPOINTER_TO_UINT(value);
        trellis_node<nstore> * node = &g_array_index
            (step_content, trellis_node<nstore>, node_index);

        if (sub_index >= node->length())
            return false;

        candidate = node->begin() + sub_index;

        return true;
    }
};

template <gint32 nstore, gint32 nbest>
bool extract_result(const ForwardPhoneticTrellis<nstore, nbest> * trellis,
                    const trellis_value_t * tail,
                    /* out */ MatchResult & result) {
    /* reset result */
    g_array_set_size(result, trellis->size());
    for (size_t i = 0; i < result->len; ++i){
        phrase_token_t * token = &g_array_index(result, phrase_token_t, i);
        *token = null_token;
    }

    /* backtracing */
    while( true ){
        int index = tail->m_last_step;
        if ( -1 == index )
            break;

        phrase_token_t * token = &g_array_index
            (result, phrase_token_t, index);
        *token = tail->m_handles[1];

        phrase_token_t last_token = tail->m_handles[0];
        int sub_index = tail->m_sub_index;
        assert(trellis->get_candidate(index, last_token, sub_index, tail));
    }

    /* no need to reverse the result */
    return true;
}

#if 0
template <gint32 nstore>
class BackwardPhoneticMatrix {
private:
    /* Array of matrix_step */
    GArray * m_steps_matrix;

public:
    /* set tail node */
    bool set_tail(const matrix_step<nstore> * tail);
    /* back trace */
    /* always assume/assert matrix_step.eval_item(...) return true? */
    bool back_trace(const ForwardPhoneticTrellis * trellis);
    /* extract results */
    int extract(/* out */ GPtrArray * arrays);
};
#endif

class ForwardPhoneticConstraints {
private:
    /* Array of trellis_constraint_t */
    GArray * m_constraints;

protected:
    FacadePhraseIndex * m_phrase_index;

public:
    ForwardPhoneticConstraints(FacadePhraseIndex * phrase_index) {
        m_phrase_index = phrase_index;
        m_constraints = g_array_new(TRUE, TRUE, sizeof(trellis_constraint_t));
    }

    ~ForwardPhoneticConstraints() {
        m_phrase_index = NULL;
        g_array_free(m_constraints, TRUE);
        m_constraints = NULL;
    }

public:
    bool clear() {
        g_array_set_size(m_constraints, 0);
        return true;
    }

    int add_constraint(size_t start, size_t end, phrase_token_t token);
    bool clear_constraint(size_t index);
    bool validate_constraint(PhoneticKeyMatrix * matrix);

    size_t length() const { return m_constraints->len; }
    bool get_constraint(size_t index,
                        const trellis_constraint_t * & constraint) const {
        if (index >= m_constraints->len)
            return false;

        constraint = &g_array_index(m_constraints, trellis_constraint_t, index);

        return true;
    }

    bool diff_result(MatchResult best, MatchResult other);
};


class NBestMatchResults {
private:
    /* Array of MatchResult */
    GPtrArray * m_results;

public:
    NBestMatchResults() {
        m_results = g_ptr_array_new();
    }

    ~NBestMatchResults() {
        clear();
        g_ptr_array_free(m_results, TRUE);
        m_results = NULL;
    }

public:
    size_t size() const {
        return m_results->len;
    }

    bool get_result(size_t index, MatchResult & result) const {
        if (index >= m_results->len)
            return false;

        result = (MatchResult) g_ptr_array_index(m_results, index);
        return true;
    }

    bool clear() {
        /* free m_results */
        for (size_t i = 0; i < m_results->len; ++i) {
            MatchResult array =
                (MatchResult) g_ptr_array_index(m_results, i);
            g_array_free(array, TRUE);
        }
        g_ptr_array_set_size(m_results, 0);

        return true;
    }

    /* copy result here */
    bool add_result(MatchResult result) {
        MatchResult array = g_array_new
            (TRUE, TRUE, sizeof(phrase_token_t));

        g_array_append_vals(array, result->data, result->len);

        g_ptr_array_add(m_results, array);
        return true;
    }
};

template <gint32 nstore, gint32 nbest>
class PhoneticLookup {
private:
    const gfloat bigram_lambda;
    const gfloat unigram_lambda;

    /* memory cache */
    GArray * m_cached_keys;
    PhraseItem m_cached_phrase_item;
    SingleGram m_merged_single_gram;

protected:
    ForwardPhoneticTrellis<nstore, nbest> m_trellis;

protected:
    /* saved varibles */
    const ForwardPhoneticConstraints * m_constraints;
    const PhoneticKeyMatrix * m_matrix;

    FacadeChewingTable2 * m_pinyin_table;
    FacadePhraseIndex * m_phrase_index;
    Bigram * m_system_bigram;
    Bigram * m_user_bigram;

protected:
    bool search_unigram2(GPtrArray * topresults,
                         int start, int end,
                         PhraseIndexRanges ranges) {
        if (0 == topresults->len)
            return false;

        trellis_value_t * max = (trellis_value_t *)
            g_ptr_array_index(topresults, 0);

        const trellis_constraint_t * constraint = NULL;
        assert(m_constraints->get_constraint(start, constraint));

        if (CONSTRAINT_ONESTEP == constraint->m_type) {
            return unigram_gen_next_step(start, constraint->m_constraint_step,
                                         max, constraint->m_token);
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
                        found = unigram_gen_next_step(start, end, max, token) ||
                            found;
                    }
                }
            }
        }

        return found;
    }

    bool search_bigram2(GPtrArray * topresults,
                        int start, int end,
                        PhraseIndexRanges ranges) {
        const trellis_constraint_t * constraint = NULL;
        assert(m_constraints->get_constraint(start, constraint));

        bool found = false;
        BigramPhraseArray bigram_phrase_items = g_array_new
            (FALSE, FALSE, sizeof(BigramPhraseItem));

        for (size_t i = 0; i < topresults->len; ++i) {
            trellis_value_t * value = (trellis_value_t *)
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
                    found = bigram_gen_next_step(start,
                                                 constraint->m_constraint_step,
                                                 value, token, bigram_poss) || found;
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
                            found = bigram_gen_next_step(start, end, value, item->m_token, item->m_freq) || found;
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

    bool unigram_gen_next_step(int start, int end,
                               trellis_value_t * cur_step,
                               phrase_token_t token) {
        if (m_phrase_index->get_phrase_item(token, m_cached_phrase_item))
            return false;

        size_t phrase_length = m_cached_phrase_item.get_phrase_length();
        gdouble elem_poss = m_cached_phrase_item.get_unigram_frequency() /
            (gdouble) m_phrase_index->get_phrase_index_total_freq();
        if ( elem_poss < DBL_EPSILON )
            return false;

        gfloat pinyin_poss = compute_pronunciation_possibility
            (m_matrix, start, end, m_cached_keys, m_cached_phrase_item);
        if (pinyin_poss < FLT_EPSILON )
            return false;

        trellis_value_t next_step;
        next_step.m_handles[0] = cur_step->m_handles[1]; next_step.m_handles[1] = token;
        next_step.m_sentence_length = cur_step->m_sentence_length + phrase_length;
        next_step.m_poss = cur_step->m_poss + log(elem_poss * pinyin_poss * unigram_lambda);
        next_step.m_last_step = start;
        next_step.m_sub_index = cur_step->m_current_index;

        return save_next_step(end, &next_step);
    }

    bool bigram_gen_next_step(int start, int end,
                              trellis_value_t * cur_step,
                              phrase_token_t token,
                              gfloat bigram_poss) {
        if (m_phrase_index->get_phrase_item(token, m_cached_phrase_item))
            return false;

        size_t phrase_length = m_cached_phrase_item.get_phrase_length();
        gdouble unigram_poss = m_cached_phrase_item.get_unigram_frequency() /
            (gdouble) m_phrase_index->get_phrase_index_total_freq();
        if ( bigram_poss < FLT_EPSILON && unigram_poss < DBL_EPSILON )
            return false;

        gfloat pinyin_poss = compute_pronunciation_possibility
                           (m_matrix, start, end,
                            m_cached_keys, m_cached_phrase_item);
        if ( pinyin_poss < FLT_EPSILON )
            return false;

        trellis_value_t next_step;
        next_step.m_handles[0] = cur_step->m_handles[1]; next_step.m_handles[1] = token;
        next_step.m_sentence_length = cur_step->m_sentence_length + phrase_length;
        next_step.m_poss = cur_step->m_poss +
            log((bigram_lambda * bigram_poss + unigram_lambda * unigram_poss) * pinyin_poss);
        next_step.m_last_step = start;
        next_step.m_sub_index = cur_step->m_current_index;

        return save_next_step(end, &next_step);
    }

    bool save_next_step(int index, trellis_value_t * candidate) {
        lookup_key_t token = candidate->m_handles[1];
        return m_trellis.insert_candidate(index, token, candidate);
    }

public:

    PhoneticLookup(const gfloat lambda,
                   FacadeChewingTable2 * pinyin_table,
                   FacadePhraseIndex * phrase_index,
                   Bigram * system_bigram,
                   Bigram * user_bigram)
        : bigram_lambda(lambda),
          unigram_lambda(1. - lambda)
    {
        assert(nstore <= nbest);

        /* store the pointer. */
        m_pinyin_table = pinyin_table;
        m_phrase_index = phrase_index;
        m_system_bigram = system_bigram;
        m_user_bigram = user_bigram;

        m_cached_keys = g_array_new(TRUE, TRUE, sizeof(ChewingKey));

        /* the member variables below are saved in get_nbest_match call. */
        m_matrix = NULL;
        m_constraints = NULL;
    }

    ~PhoneticLookup(){
        g_array_free(m_cached_keys, TRUE);
        m_cached_keys = NULL;
    }


    bool get_nbest_match(TokenVector prefixes,
                         const PhoneticKeyMatrix * matrix,
                         const ForwardPhoneticConstraints * constraints,
                         NBestMatchResults * results) {
        m_constraints = constraints;
        m_matrix = matrix;

        int nstep = m_matrix->size();
        if (0 == nstep)
            return false;

        /* free results */
        results->clear();

        m_trellis.clear();
        m_trellis.prepare(nstep);

        m_trellis.fill_prefixes(prefixes);

        PhraseIndexRanges ranges;
        memset(ranges, 0, sizeof(PhraseIndexRanges));
        m_phrase_index->prepare_ranges(ranges);

        GPtrArray * candidates = g_ptr_array_new();
        GPtrArray * topresults = g_ptr_array_new();

        /* begin the viterbi beam search. */
        for ( int i = 0; i < nstep - 1; ++i ){
            const trellis_constraint_t * cur_constraint = NULL;
            assert(m_constraints->get_constraint(i, cur_constraint));

            if (CONSTRAINT_NOSEARCH == cur_constraint->m_type)
                continue;

            m_trellis.get_candidates(i, candidates);
            get_top_results<nstore>(nbeam, topresults, candidates);

            if (0 == topresults->len)
                continue;

            if (CONSTRAINT_ONESTEP == cur_constraint->m_type) {
                int m = cur_constraint->m_constraint_step;

                m_phrase_index->clear_ranges(ranges);

                /* do one pinyin table search. */
                int retval = search_matrix(m_pinyin_table, m_matrix,
                                           i, m, ranges);

                if (retval & SEARCH_OK) {
                    /* assume topresults always contains items. */
                    search_bigram2(topresults, i, m, ranges),
                        search_unigram2(topresults, i, m, ranges);
                }

                continue;
            }

            for ( int m = i + 1; m < nstep; ++m ){
                const trellis_constraint_t * next_constraint = NULL;
                assert(m_constraints->get_constraint(m, next_constraint));

                if (CONSTRAINT_NOSEARCH == next_constraint->m_type)
                    break;

                m_phrase_index->clear_ranges(ranges);

                /* do one pinyin table search. */
                int retval = search_matrix(m_pinyin_table, m_matrix,
                                           i, m, ranges);

                if (retval & SEARCH_OK) {
                    /* assume topresults always contains items. */
                    search_bigram2(topresults, i, m, ranges),
                        search_unigram2(topresults, i, m, ranges);
                }

                /* no longer pinyin */
                if (!(retval & SEARCH_CONTINUED))
                    break;
            }
        }

        m_phrase_index->destroy_ranges(ranges);

        g_ptr_array_free(candidates, TRUE);
        g_ptr_array_free(topresults, TRUE);

        /* extract every result. */
        GPtrArray * tails = g_ptr_array_new();
        m_trellis.get_tails(tails);

        MatchResult result = g_array_new
            (TRUE, TRUE, sizeof(phrase_token_t));
        for (size_t i = 0; i < tails->len; ++i) {
            const trellis_value_t * tail = (const trellis_value_t *)
                g_ptr_array_index(tails, i);

            assert(extract_result<nstore>(&m_trellis, tail, result));
            results->add_result(result);
        }

        g_array_free(result, TRUE);
        g_ptr_array_free(tails, TRUE);

        return true;
    }

    bool train_result3(const PhoneticKeyMatrix * matrix,
                       const ForwardPhoneticConstraints * constraints,
                       MatchResult result) {
        const guint32 initial_seed = 23 * 3;
        const guint32 expand_factor = 2;
        const guint32 unigram_factor = 7;
        const guint32 pinyin_factor = 1;
        const guint32 ceiling_seed = 23 * 15 * 64;

        /* begin training based on constraints and result. */
        bool train_next = false;

        phrase_token_t last_token = sentence_start;

        for (size_t i = 0; i < constraints->length(); ++i) {
            phrase_token_t token = g_array_index(result, phrase_token_t, i);
            if (null_token == token)
                continue;

            const trellis_constraint_t * constraint = NULL;
            assert(constraints->get_constraint(i, constraint));

            if (train_next || CONSTRAINT_ONESTEP == constraint->m_type) {
                if (CONSTRAINT_ONESTEP == constraint->m_type) {
                    assert(token == constraint->m_token);
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
                    if (!user->get_freq(token, freq)) {
                        assert(user->insert_freq(token, 0));
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
                    assert(user->set_freq(token, freq + seed));
                    assert(m_user_bigram->store(last_token, user));
                next:
                    assert(NULL != user);
                    if (user)
                        delete user;
                }

                /* compute the position of next token. */
                size_t next_pos = i + 1;
                for (; next_pos < constraints->length(); ++next_pos) {
                    phrase_token_t next_token = g_array_index
                        (result, phrase_token_t, next_pos);

                    if (null_token != next_token)
                        break;
                }
                /* safe guard for last token. */
                next_pos = std_lite::min(next_pos, constraints->length() - 1);

                /* train uni-gram */
                m_phrase_index->get_phrase_item(token, m_cached_phrase_item);
                increase_pronunciation_possibility
                    (matrix, i, next_pos,
                     m_cached_keys, m_cached_phrase_item, seed * pinyin_factor);
                m_phrase_index->add_unigram_frequency
                    (token, seed * unigram_factor);
            }

            last_token = token;
        }

        return true;
    }

    bool convert_to_utf8(MatchResult result,
                         /* out */ char * & result_string)
    {
        return pinyin::convert_to_utf8(m_phrase_index, result,
                                       NULL, false, result_string);
    }

};

};

#endif
