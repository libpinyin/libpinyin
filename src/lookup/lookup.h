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

#ifndef LOOKUP_H
#define LOOKUP_H


/** @file lookup.h
 *  @brief the definitions of common lookup related classes and structs.
 */

#include "novel_types.h"
#include <limits.h>

namespace pinyin{

typedef phrase_token_t lookup_key_t;

struct lookup_value_t{
    /* previous and current tokens of the node */
    phrase_token_t m_handles[2];
    /* the sentence length */
    gint32 m_length;
    /* maximum possibility of current node  */
    gfloat m_poss;
    /* trace back information for final step */
    gint32 m_last_step;

    lookup_value_t(gfloat poss = FLT_MAX){
        m_handles[0] = null_token;
        m_handles[1] = null_token;
        m_length = 0;
        m_poss = poss;
        m_last_step = -1;
    }
};


enum constraint_type{NO_CONSTRAINT, CONSTRAINT_ONESTEP, CONSTRAINT_NOSEARCH };

class FacadePhraseIndex;


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

/* Key: lookup_key_t, Value: int m, index to m_steps_content[i][m] */
typedef GHashTable * LookupStepIndex;
/* Array of lookup_value_t or trellis_node */
typedef GArray * LookupStepContent;

bool convert_to_utf8(FacadePhraseIndex * phrase_index,
                     MatchResult match_result,
                     /* in */ const char * delimiter,
                     /* in */ bool show_tokens,
                     /* out */ char * & result_string);

};
#endif
