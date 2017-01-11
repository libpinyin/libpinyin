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

namespace pinyin{

struct trellis_value_t {
    phrase_token_t m_handles[2];
    // the character length of the final sentence.
    gint32 m_sentence_length;
    gfloat m_poss;
    // the m_last_step and m_last_index points to this trellis.
    gint32 m_last_step;
    // the m_last_index points to the last trellis_node.
    gint32 m_last_index;
    // the current index in this trellis_node.
    // only initialized in the get_candidates method.
    gint32 m_current_index;

    trellis_value_t(gfloat poss = FLT_MAX){
        m_handles[0] = null_token;
        m_handles[1] = null_token;
        m_sentence_length = 0;
        m_poss = poss;
        m_last_step = -1;
        m_last_index = -1;
        m_current_index = -1;
    }
};

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

#if 1
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

typedef phrase_token_t lookup_key_t;
/* Key: lookup_key_t, Value: int m, index to m_steps_content[i][m] */
typedef GHashTable * LookupStepIndex;
 /* Array of trellis_node */
typedef GArray * LookupStepContent;

template <gint32 nbest>
class ForwardPhoneticTrellis {
private:
    /* Array of LookupStepIndex */
    GPtrArray * m_steps_index;
    /* Array of LookupStepContent */
    GPtrArray * m_steps_content;

public:
    bool clear();
    bool prepare(gint32 nstep);

    /* Array of phrase_token_t */
    bool fill_prefixes(/* in */ TokenVector prefixes);
    /* Array of trellis_value_t */
    bool get_candidates(/* in */ gint32 index,
                        /* out */ GArray * candidates) const;
    /* insert candidate */
    bool insert_candidate(gint32 index, phrase_token_t token,
                          const trellis_value_t * candidate);
    /* get tails */
    /* Array of trellis_value_t */
    bool get_tails(/* out */ GArray * tails) const;
    /* get candidate */
    bool get_candidate(gint32 index, phrase_token_t token,
                       gint32 last_index, trellis_value_t * candidate) const;
};

template <gint32 nbest>
bool extract_result(const ForwardPhoneticTrellis<nbest> * trellis,
                    const trellis_value_t * tail,
                    /* out */ MatchResults & result);

#if 0
template <gint32 nbest>
class BackwardPhoneticMatrix {
private:
    /* Array of matrix_step */
    GArray * m_steps_matrix;

public:
    /* set tail node */
    bool set_tail(const matrix_step<nbest> * tail);
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

#if 0
    /* pre-mature optimazition? */
    GArray * m_cached_keys;
    PhraseItem m_cached_phrase_item;
#endif

public:
    int add_constraint(size_t start, size_t end, phrase_token_t token);
    bool clear_constraint(size_t index);
    bool validate_constraint(PhoneticKeyMatrix * matrix);
};

/* use maximum heap to get the topest results. */
static bool get_top_results(/* out */ GPtrArray * topresults,
                            /* in */ GPtrArray * candidates);


/* Array of MatchResults */
typedef GPtrArray * NBestMatchResults;

template <gint32 nbest>
class PhoneticLookup {
private:
    const gfloat bigram_lambda;
    const gfloat unigram_lambda;

protected:
    ForwardPhoneticTrellis<nbest> m_forward_trellis;
    BackwardPhoneticMatrix<nbest> m_backward_matrix;

protected:
    /* saved varibles */
    ForwardPhoneticConstraints m_constraints;
    PhoneticKeyMatrix * m_matrix;

    FacadeChewingTable2 * m_pinyin_table;
    FacadePhraseIndex * m_phrase_index;
    Bigram * m_system_bigram;
    Bigram * m_user_bigram;

protected:
    bool search_unigram2(GPtrArray * topresults,
                         int start, int end,
                         PhraseIndexRanges ranges);
    bool search_bigram2(GPtrArray * topresults,
                        int start, int end,
                        PhraseIndexRanges ranges);

    bool unigram_gen_next_step(int start, int end,
                               lookup_value_t * cur_step,
                               phrase_token_t token);
    bool bigram_gen_next_step(int start, int end,
                              lookup_value_t * cur_step,
                              phrase_token_t token,
                              gfloat bigram_poss);

    bool save_next_step(int next_step_pos, trellis_value_t * cur_step, trellis_value_t * next_step);

public:

    bool get_best_match(TokenVector prefixes,
                        PhoneticKeyMatrix * matrix,
                        ForwardPhoneticConstraints constraints,
                        NBestMatchResults & results);

};

};

#endif
