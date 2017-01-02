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
};

template <gint32 nbest>
struct trellis_node {
private:
    gint32 m_nelem;
    trellis_value_t m_elements[nbest];
public:

};

struct matrix_value_t {
    phrase_token_t m_cur_token;
    gfloat m_poss;
    // the below information for recovering the final phrase array.
    // the m_next_step and m_next_index points to this matrix.
    gint32 m_next_step;
    gint32 m_next_index;
};

template <gint32 nbest>
struct matrix_step {
private:
    gint32 m_nelem;
    matrix_value_t m_elements[nbest];
public:

};

struct trellis_constraint_t {
    constraint_type m_type;
    // expand the previous union into struct to catch some errors.
    phrase_token_t m_token;
    guint32 m_constraint_step;
};


};

#endif
