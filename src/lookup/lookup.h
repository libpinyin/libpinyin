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

#ifndef LOOKUP_H
#define LOOKUP_H

#include <float.h>
#include <glib.h>
#include "novel_types.h"
#include "pinyin_base.h"

class WinnerTree;

/** @file lookup.h
 *  @brief the definitions of lookup related classes and structs.
 *         Currently only contains pinyin lookup.
 */

typedef phrase_token_t lookup_key_t;

struct lookup_value_t{
    phrase_token_t m_handles[2];
    gfloat m_poss;
    gint32 m_last_step;
    lookup_value_t(gfloat poss = FLT_MAX){
	m_handles[0] = NULL; m_handles[1] = NULL;
	m_poss = poss;
	m_last_step = -1;
    }
};

enum constraint_type{NO_CONSTRAINT, CONSTRAINT_ONESTEP, CONSTRAINT_NOSEARCH };

struct lookup_constraint_t{
    constraint_type m_type;
    union{
	phrase_token_t m_token;
	guint32 m_constraint_step; /* index of m_token */
    };
};

typedef GArray * CandidateConstraints; /* Array of lookup_constraint_t */
typedef GArray * MatchResults;         /* Array of phrase_token_t */

namespace novel{
class PinyinLargeTable;
class FacadePhraseIndex;
class Bigram;
};

typedef GHashTable * LookupStepIndex;
/* Key: lookup_key_t, Value: int m, index to m_steps_content[i][m] */
typedef GArray * LookupStepContent; /* array of lookup_value_t */


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
    
    novel::PinyinLargeTable * m_pinyin_table;
    novel::FacadePhraseIndex * m_phrase_index;
    novel::PinyinCustomSettings * m_custom;
    novel::Bigram * m_bigram;
    
    //internal step data structure
    GPtrArray * m_steps_index;  
    /* Array of LookupStepIndex */
    GPtrArray * m_steps_content;
    /* Array of LookupStepContent */

    GArray * m_table_cache;
    /* Array of PhraseIndexRanges */
    
    WinnerTree * m_winner_tree;

    size_t prepare_table_cache(int nstep, int total_pinyin);
    
    bool search_unigram(IBranchIterator * iter,  int nstep, int npinyin);
    bool search_bigram(IBranchIterator * iter,  int nstep, int npinyin);
    
    bool unigram_gen_next_step(int nstep, lookup_value_t * cur_step, phrase_token_t token);
    bool bigram_gen_next_step(int nstep, lookup_value_t * cur_step, phrase_token_t token, gfloat bigram_poss);
        
    bool save_next_step(int next_step_pos, lookup_value_t * cur_step, lookup_value_t * next_step);
    
    bool final_step(MatchResults & results);
public:
    PinyinLookup( PinyinCustomSettings * custom, PinyinLargeTable * pinyin_table, FacadePhraseIndex * phrase_index, Bigram * bigram);

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
