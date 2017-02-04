/* 
 *  libpinyin
 *  Library to deal with pinyin.
 *  
 *  Copyright (C) 2006-2007 Peng Wu
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

#ifndef PHRASE_LOOKUP_H
#define PHRASE_LOOKUP_H

#include "novel_types.h"
#include "ngram.h"
#include "lookup.h"

/**
 * phrase_lookup.h
 *
 * The definitions of phrase lookup related classes and structs.
 *
 */

namespace pinyin{

/**
 * PhraseLookup:
 *
 * The phrase lookup class to convert the sentence to phrase tokens.
 *
 */
class PhraseLookup{
private:
    const gfloat bigram_lambda;
    const gfloat unigram_lambda;

    PhraseItem m_cached_phrase_item;
    SingleGram m_merged_single_gram;
protected:
    //saved varibles
    FacadePhraseTable3 * m_phrase_table;
    FacadePhraseIndex * m_phrase_index;
    Bigram * m_system_bigram;
    Bigram * m_user_bigram;

    //internal step data structure
    GPtrArray * m_steps_index;
    /* Array of LookupStepIndex */
    GPtrArray * m_steps_content;
    /* Array of LookupStepContent */

    /* Saved sentence */
    int m_sentence_length;
    ucs4_t * m_sentence;

protected:
    /* Explicitly search the next phrase,
     *  to avoid double phrase lookup as the next token has only one.
     */
    bool search_unigram2(int nstep, PhraseTokens tokens);
    bool search_bigram2(int nstep, PhraseTokens tokens);

    bool unigram_gen_next_step(int nstep, lookup_value_t * cur_value, phrase_token_t token);
    bool bigram_gen_next_step(int nstep, lookup_value_t * cur_value, phrase_token_t token, gfloat bigram_poss);

    bool save_next_step(int next_step_pos, lookup_value_t * cur_value, lookup_value_t * next_step);

    bool final_step(MatchResult & result);
public:
    /**
     * PhraseLookup::PhraseLookup:
     * @lambda: the lambda parameter for interpolation model.
     * @phrase_table: the phrase table.
     * @phrase_index: the phrase index.
     * @system_bigram: the system bi-gram.
     * @user_bigram: the user bi-gram.
     *
     * The constructor of the PhraseLookup.
     *
     */
    PhraseLookup(const gfloat lambda,
                 FacadePhraseTable3 * phrase_table,
                 FacadePhraseIndex * phrase_index,
                 Bigram * system_bigram,
                 Bigram * user_bigram);

    /**
     * PhraseLookup::~PhraseLookup:
     *
     * The destructor of the PhraseLookup.
     *
     */
    ~PhraseLookup();

    /**
     * PhraseLookup::get_best_match:
     * @sentence_length: the length of the sentence in ucs4 characters.
     * @sentence: the ucs4 characters of the sentence.
     * @results: the segmented sentence in the form of phrase tokens.
     * @returns: whether the segment operation is successful.
     *
     * Segment the sentence into phrase tokens.
     *
     * Note: this method only accepts the characters in phrase large table.
     *
     */
    bool get_best_match(int sentence_length, ucs4_t sentence[], MatchResult & result);

    /**
     * PhraseLookup::convert_to_utf8:
     * @results: the guessed sentence in the form of phrase tokens.
     * @result_string: the converted sentence in utf8 string.
     * @returns: whether the convert operation is successful.
     *
     * Convert the sentence from phrase tokens to the utf8 string.
     *
     * Note: free the result_string by g_free.
     *
     */
    bool convert_to_utf8(MatchResult result,
                         /* out */ char * & result_string)
    {
        return pinyin::convert_to_utf8(m_phrase_index, result,
                                       "\n", true, result_string);
    }
};

};

#endif
