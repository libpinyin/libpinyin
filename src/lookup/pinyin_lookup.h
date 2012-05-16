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
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef PINYIN_LOOKUP_H
#define PINYIN_LOOKUP_H


#include <float.h>
#include <glib.h>
#include "novel_types.h"
#include "chewing_key.h"
#include "phrase_index.h"
#include "ngram.h"
#include "lookup.h"


namespace pinyin{

class WinnerTree;

/**
 * pinyin_lookup.h
 *
 * The definitions of pinyin lookup related classes and structs.
 *
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


/**
 * IBranchIterator:
 *
 * The iterator to get the 32 highest values.
 *
 * Note: The winner tree for Viterbi beam search.
 *
 */
class IBranchIterator{
public:
  virtual ~IBranchIterator(){}
  virtual bool has_next() = 0;
  virtual lookup_value_t next() = 0;
  virtual lookup_value_t max() = 0;
};

/**
 * PinyinLookup:
 *
 * The pinyin lookup class to convert pinyin keys to guessed sentence.
 *
 */
class PinyinLookup{
private:
    static const gfloat bigram_lambda;
    static const gfloat unigram_lambda;
    
    PhraseItem m_cache_phrase_item;
    SingleGram m_merged_single_gram;
protected:
    //saved varibles
    CandidateConstraints m_constraints;
    ChewingKeyVector m_keys;
    
    FacadeChewingTable * m_pinyin_table;
    FacadePhraseIndex * m_phrase_index;
    pinyin_option_t m_options;
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
    /**
     * PinyinLookup::PinyinLookup:
     * @options: the pinyin options.
     * @pinyin_table: the pinyin table.
     * @phrase_index: the phrase index.
     * @system_bigram: the system bi-gram.
     * @user_bigram: the user bi-gram.
     *
     * The constructor of the PinyinLookup.
     *
     */
    PinyinLookup(pinyin_option_t options, FacadeChewingTable * pinyin_table,
                 FacadePhraseIndex * phrase_index, Bigram * system_bigram,
                 Bigram * user_bigram);

    /**
     * PinyinLookup::~PinyinLookup:
     *
     * The destructor of the PinyinLookup.
     *
     */
    ~PinyinLookup();

    /**
     * PinyinLookup::set_options:
     * @options: the pinyin options.
     * @returns: whether the set operation is successful.
     *
     * Set the pinyin options of this PinyinLookup.
     *
     */
    bool set_options(pinyin_option_t options) {
        m_options = options;
        return true;
    }

    /**
     * PinyinLookup::get_best_match:
     * @prefixes: the phrase tokens before the guessed sentence.
     * @keys: the pinyin keys of the guessed sentence.
     * @constraints: the constraints on the guessed sentence.
     * @results: the guessed sentence in the form of the phrase tokens.
     * @returns: whether the guess operation is successful.
     *
     * Guess the best sentence according to user inputs.
     *
     */
    bool get_best_match(TokenVector prefixes, ChewingKeyVector keys, CandidateConstraints constraints, MatchResults & results);

    /**
     * PinyinLookup::train_result2:
     * @keys: the pinyin keys of the guessed sentence.
     * @constraints: the constraints on the guessed sentence.
     * @results: the guessed sentence in the form of the phrase tokens.
     * @returns: whether the train operation is successful.
     *
     * Self learning the guessed sentence based on the constraints.
     *
     */
    bool train_result2(ChewingKeyVector keys, CandidateConstraints constraints, MatchResults results);

    /**
     * PinyinLookup::convert_to_utf8:
     * @results: the guessed sentence in the form of the phrase tokens.
     * @result_string: the guessed sentence in the utf8 encoding.
     * @returns: whether the convert operation is successful.
     *
     * Convert the guessed sentence from the phrase tokens to the utf8 string.
     *
     */
    bool convert_to_utf8(MatchResults results,
                         /* out */ char * & result_string)
    {
        return pinyin::convert_to_utf8(m_phrase_index, results,
                                       NULL, result_string);
    }

    /**
     * PinyinLookup::add_constraint:
     * @constraints: the constraints on the guessed sentence.
     * @index: the character offset in the guessed sentence.
     * @token: the phrase token in the candidate list chosen by user.
     * @returns: the number of the characters in the chosen token.
     *
     * Add one constraint to the constraints on the guessed sentence.
     *
     */
    guint8 add_constraint(CandidateConstraints constraints, size_t index, phrase_token_t token);

    /**
     * PinyinLookup::clear_constraint:
     * @constraints: the constraints on the guessed sentence.
     * @index: the character offset in the guessed sentence.
     * @returns: whether the clear operation is successful.
     *
     * Clear one constraint in the constraints on the guessed sentence.
     *
     */
    bool clear_constraint(CandidateConstraints constraints, size_t index);

    /**
     * PinyinLookup::validate_constraint:
     * @constraints: the constraints on the guessed sentence.
     * @keys: the pinyin keys of the guessed sentence.
     * @returns: whether the validate operation is successful.
     *
     * Validate the old constraints with the new pinyin keys.
     *
     */
    bool validate_constraint(CandidateConstraints constraints, ChewingKeyVector keys);

};

};
#endif
