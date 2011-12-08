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

#ifndef LOOKUP_H
#define LOOKUP_H


/** @file lookup.h
 *  @brief the definitions of common lookup related classes and structs.
 */

#include "novel_types.h"

namespace pinyin{

typedef phrase_token_t lookup_key_t;

struct lookup_value_t{
    /* previous and current tokens of the node */
    phrase_token_t m_handles[2];
    /* maximum possibility of current node  */
    gfloat m_poss;
    /* trace back information for final step */
    gint32 m_last_step;

    lookup_value_t(gfloat poss = FLT_MAX){
	m_handles[0] = null_token; m_handles[1] = null_token;
	m_poss = poss;
	m_last_step = -1;
    }
};


class PinyinLargeTable;
class PhraseLargeTable;
class FacadePhraseIndex;
class Bigram;


/* Note:
 *   LookupStepIndex:
 *     the main purpose of lookup step index is served for an index
 *     for lookup step content, which can quickly merge the same node
 *     with different possibilities,
 *     then only keep the highest value of the node.
 *   LookupStepContent:
 *     the place to store the lookup values of current step,
 *     and indexed by lookup step index.
 *     See also comments on lookup_value_t.
 */

typedef GHashTable * LookupStepIndex;
/* Key: lookup_key_t, Value: int m, index to m_steps_content[i][m] */
typedef GArray * LookupStepContent; /* array of lookup_value_t */

bool convert_to_utf8(FacadePhraseIndex * phrase_index,
                     MatchResults match_results,
                     /* in */ const char * delimiter,
                     /* out */ char * & result_string);

};
#endif
