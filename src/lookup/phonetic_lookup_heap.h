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

template <gint32 nstore>
static inline bool trellis_value_comp(const trellis_value_t &lhs,
                                      const trellis_value_t &rhs) {
    /* min heap here */
    return trellis_value_less_than<nstore>(&lhs, &rhs);
}

template <gint32 nstore>
struct trellis_node {
private:
    gint32 m_nelem;
    /* invariant: min heap */
    trellis_value_t m_elements[nstore];

public:
    trellis_node(){
        m_nelem = 0;
        /* always assume non-used m_elements contains random data. */
    }

public:
    gint32 length() { return m_nelem; }
    const trellis_value_t * begin() { return m_elements; }
    const trellis_value_t * end() { return m_elements + m_nelem; }

    bool number() {
        for (ssize_t i = 0; i < m_nelem; ++i)
            m_elements[i].m_current_index = i;
        return true;
    }

    /* return true if the item is stored into m_elements. */
    bool eval_item(const trellis_value_t * item) {

        /* min heap here. */

        /* still have space */
        if (m_nelem < nstore) {
            m_elements[m_nelem] = *item;
            m_nelem ++;
            /* always push heap. */
            std_lite::push_heap(m_elements, m_elements + m_nelem, trellis_value_comp<nstore>);
            return true;
        }

        /* find minium item */
        trellis_value_t * min = m_elements;

        /* compare new item */
        if (trellis_value_less_than<nstore>(min, item)) {
            std_lite::pop_heap(m_elements, m_elements + m_nelem, trellis_value_comp<nstore>);
            m_elements[m_nelem - 1] = *item;
            std_lite::push_heap(m_elements, m_elements + m_nelem, trellis_value_comp<nstore>);
            return true;
        }

        return false;
    }
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

    bool number() {
        m_element.m_current_index = 0;
        return true;
    }

    /* return true if the item is stored into m_element. */
    bool eval_item(const trellis_value_t * item) {

        /* no item yet. */
        if (0 == m_element.m_sentence_length) {
            m_element = *item;
            return true;
        }

        if (trellis_value_less_than<1>(&m_element, item)) {
            m_element = *item;
            return true;
        }

        return false;
    }
};


#endif
