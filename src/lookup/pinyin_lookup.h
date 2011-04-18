/* 
 *  libpinyin
 *  Library to deal with pinyin.
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

#ifndef PINYIN_LOOKUP_H
#define PINYIN_LOOKUP_H

#include <float.h>
#include <glib.h>
#include "novel_types.h"
#include "pinyin_base.h"
#include "lookup.h"

class WinnerTree;

/** @file pinyin_lookup.h
 *  @brief the definitions of pinyin lookup related classes and structs.
 */

enum constraint_type{NO_CONSTRAINT, CONSTRAINT_ONESTEP, CONSTRAINT_NOSEARCH };

struct lookup_constraint_t{
    /* current type of the step */
    constraint_type m_type;

    /* Note:
     *   value of m_type:
     *     NO_CONSTRAINT:
     *       no values in the below union.
     *       search all possible next words.
     *     CONSTRAINT_ONESTEP:
     *       m_token contains the next word.
     *       only one word can be used to search for the next step,
     *       use case for user selected candidates.
     *     CONSTRAINT_NOSEARCH:
     *       m_constraint_step contains the value
     *       which points back to the CONSTRAINT_ONESTEP step.
     *       no search is allowed for the current step.
     */

    union{
	phrase_token_t m_token;
	guint32 m_constraint_step; /* index of m_token */
    };
};

typedef GArray * CandidateConstraints; /* Array of lookup_constraint_t */

/* Note:
 *   winner tree for beam search.
 */
class IBranchIterator{
public:
  virtual ~IBranchIterator(){}
  virtual bool has_next() = 0;
  virtual lookup_value_t next() = 0;
  virtual lookup_value_t max() = 0;
};  

class PinyinLookup{
private:
    static const gfloat bigram_lambda = LAMBDA_PARAMETER;
    static const gfloat unigram_lambda = 1 - LAMBDA_PARAMETER;
    
    PhraseItem m_cache_phrase_item;
protected:
    //saved varibles
    CandidateConstraints m_constraints;
    PinyinKeyVector m_keys;
    
    PinyinLargeTable * m_pinyin_table;
    FacadePhraseIndex * m_phrase_index;
    PinyinCustomSettings * m_custom;
    Bigram * m_system_bigram;
    Bigram * m_user_bigram;
    
    //internal step data structure
    GPtrArray * m_steps_index;  
    /* Array of LookupStepIndex */
    GPtrArray * m_steps_content;
    /* Array of LookupStepContent */

    GArray * m_table_cache;
    /* Array of PhraseIndexRanges,
     *   PhraseIndexRanges is an array of GArray of PhraseIndexRange,
     *   indexed by phrase library (only contains enabled phrase libraries).
     */
    
    WinnerTree * m_winner_tree;

    size_t prepare_table_cache(int nstep, int total_pinyin);
    
    bool search_unigram(IBranchIterator * iter,  int nstep, int npinyin);
    bool search_bigram(IBranchIterator * iter,  int nstep, int npinyin);
    
    bool unigram_gen_next_step(int nstep, lookup_value_t * cur_step, phrase_token_t token);
    bool bigram_gen_next_step(int nstep, lookup_value_t * cur_step, phrase_token_t token, gfloat bigram_poss);
        
    bool save_next_step(int next_step_pos, lookup_value_t * cur_step, lookup_value_t * next_step);
    
    bool final_step(MatchResults & results);
public:
    PinyinLookup( PinyinCustomSettings * custom, PinyinLargeTable * pinyin_table, FacadePhraseIndex * phrase_index, Bigram * system_bigram, Bigram * user_bigram);

    ~PinyinLookup();

    bool get_best_match(PinyinKeyVector keys, CandidateConstraints constraints, MatchResults & results);
    
    bool train_result(PinyinKeyVector keys, CandidateConstraints constraints, MatchResults & results);

    bool convert_to_utf8(MatchResults results, /* out */ char * & result_string);

    bool add_constraint(CandidateConstraints constraints, size_t index, phrase_token_t token);

    bool clear_constraint(CandidateConstraints constraints, size_t index);

    bool validate_constraint(CandidateConstraints constraints, PinyinKeyVector m_parsed_keys);

    /* init pinyin table lookup array */
    bool prepare_pinyin_lookup(PhraseIndexRanges ranges);
    /* destroy pinyin table lookup array */
    bool destroy_pinyin_lookup(PhraseIndexRanges ranges);
};

#endif
