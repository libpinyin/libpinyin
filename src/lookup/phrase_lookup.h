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

#ifndef PHRASE_LOOKUP_H
#define PHRASE_LOOKUP_H

#include "novel_types.h"
#include "lookup.h"

/** @file phrase_lookup.h
 *  @brief the definitions of phrase lookup related classes and structs.
 */

namespace pinyin{

class PhraseLookup{
private:
    static const gfloat bigram_lambda;
    static const gfloat unigram_lambda;

    PhraseItem m_cache_phrase_item;
protected:
    //saved varibles
    PhraseLargeTable * m_phrase_table;
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
    utf16_t * m_sentence;

protected:
    /* Explicitly search the next phrase,
     *  to avoid double phrase lookup as the next token has only one.
     */
    bool search_unigram(int nstep, phrase_token_t token);
    bool search_bigram(int nstep, phrase_token_t token);

    bool unigram_gen_next_step(int nstep, lookup_value_t * cur_value, phrase_token_t token);
    bool bigram_gen_next_step(int nstep, lookup_value_t * cur_value, phrase_token_t token, gfloat bigram_poss);

    bool save_next_step(int next_step_pos, lookup_value_t * cur_value, lookup_value_t * next_step);

    bool final_step(MatchResults & results);
public:
    PhraseLookup(PhraseLargeTable * phrase_table,
                 FacadePhraseIndex * phrase_index,
                 Bigram * system_bigram,
                 Bigram * user_bigram);

    ~PhraseLookup();

    /* Note: this method only accepts the characters in phrase large table. */
    bool get_best_match(int sentence_length, utf16_t sentence[], MatchResults & results);

    /* Note: free the phrase by g_free */
    bool convert_to_utf8(MatchResults results,
                         /* in */ const char * delimiter,
                         /* out */ char * & result_string)
    {
        return pinyin::convert_to_utf8(m_phrase_index, results,
                                       delimiter, result_string);
    }
};

};

#endif
