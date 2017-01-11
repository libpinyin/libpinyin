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

#ifndef PHONETIC_LOOKUP_HEAP_H
#define PHONETIC_LOOKUP_HEAP_H

template <gint32 nbest>
struct trellis_node {
private:
    gint32 m_nelem;
    trellis_value_t m_elements[nbest+1];
public:
    ... // helper methods
};

/* for space usage and performance. */
/* as trellis node only contains one element,
 * when the trellis node created, always put one element in it.
 * when no trellis node, it represents zero element.
 */
template <>
struct trellis_node <1> {
private:
    trellis_value_t m_element;

public:
    trellis_node <1> () : m_element(-FLT_MAX) {}

public:
    gint32 length() { return 1; }
    const trellis_value_t * begin() { return &m_element; }
    const trellis_value_t * end() { return &m_element + 1; }

    /* return true if the item is stored into m_element. */
    bool eval_item(const trellis_value_t * item) {
        if (item->m_poss > m_element.m_poss) {
            m_element = *item;
            return true;
        }

        return false;
    }
};


template <gint32 nbest>
struct matrix_step {
private:
    gint32 m_nelem;
    matrix_value_t m_elements[nbest+1];
public:
    ... // helper methods
};

/* for space usage and performance. */
/* as matrix step contains only one element,
   initialize with empty element. */
template <>
struct matrix_step <1> {
private:
    matrix_value_t m_element;

public:
    matrix_step <1> () : m_element(-FLT_MAX) {}

public:
    gint32 length() { return 1; }
    const matrix_value_t * begin() { return &m_element; }
    const matrix_value_t * end() { return &m_element + 1; }

    /* return true if the item is stored into m_element. */
    bool eval_item(const matrix_value_t * item) {
        if (item->m_poss > m_element.m_poss) {
            m_element = *item;
            return true;
        }

        return false;
    }
};


#endif
