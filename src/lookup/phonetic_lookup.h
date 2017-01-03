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
    gint32 m_last_index;

    trellis_value_t(gfloat poss = FLT_MAX){
        m_handles[0] = null_token;
        m_handles[1] = null_token;
        m_sentence_length = 0;
        m_poss = poss;
        m_last_step = -1;
        m_last_index = -1;
    }
};

template <gint32 nbest>
struct trellis_node {
private:
    gint32 m_nelem;
    trellis_value_t m_elements[nbest];

public:
    trellis_node(){
        m_nelem = 0;
        /* always assume non-used m_elements contains random data. */
    }

public:
    gint32 length() { return m_nelem; }
    const trellis_value_t * begin() { return m_elements; }
    const trellis_value_t * end() { return m_elements + m_nelem; }

    /* return true if the item is stored into m_elements. */
    bool eval_item(const trellis_value_t * item);
};

struct matrix_value_t {
    phrase_token_t m_cur_token;
    gfloat m_poss;
    // the below information for recovering the final phrase array.
    // the m_next_step and m_next_index points to this matrix.
    gint32 m_next_step;
    gint32 m_next_index;

    matrix_value_t(){
        m_cur_token = null_token;
        m_poss = FLT_MAX;
        m_next_step = -1;
        m_next_index = -1;
    }
};

template <gint32 nbest>
struct matrix_step {
private:
    gint32 m_nelem;
    matrix_value_t m_elements[nbest];

public:
    matrix_step(){
        m_nelem = 0;
        /* always assume non-used m_elements contains random data. */
    }

public:
    gint32 length() { return m_nelem; }
    const matrix_value_t * begin() { return m_elements; }
    const matrix_value_t * end() { return m_elements + m_nelem; }

    /* return true if the item is stored into m_elements. */
    bool eval_item(const trellis_value_t * item);
};

struct trellis_constraint_t {
    /* the constraint type */
    constraint_type m_type;
    // expand the previous union into struct to catch some errors.
    /* the token of the word */
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

class ForwardPhoneticTrellis {
private:
    /* Array of LookupStepIndex */
    GPtrArray * m_steps_index;
    /* Array of LookupStepContent */
    GPtrArray * m_steps_content;

};

class BackwardPhoneticMatrix {
private:
    /* Array of matrix_step */
    GArray * m_steps_matrix;
};

class ForwardPhoneticConstraints {
private:
    /* Array of trellis_constraint_t */
    GArray * m_constraints;
};

};

#endif
