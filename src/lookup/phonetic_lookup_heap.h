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
static inline bool trellis_value_more_than(const trellis_value_t &exist_item,
                                           const trellis_value_t &new_item) {
    /* min heap here */
    return trellis_value_less_than<nbest>(&new_item, &exist_item);
}

template <gint32 nbest>
struct trellis_node {
private:
    gint32 m_nelem;
    /* invariant: min heap */
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

    bool number() {
        for (ssize_t i = 0; i < m_nelem; ++i)
            m_elements[i].m_current_index = i;
    }

    /* return true if the item is stored into m_elements. */
    bool eval_item(const trellis_value_t * item) {
        /* min heap here, and always push heap. */

        /* still have space */
        if (m_nelem < nbest) {
            m_elements[m_nelem] = *item;
            m_nelem ++;
            push_heap(begin(), end(), trellis_value_more_than<nbest>);
            return true;
        }

        /* find minium item */
        trellis_value_t * min = m_elements;

        /* compare new item */
        if (item->m_poss > min->m_poss) {
            pop_heap(begin(), end(), trellis_value_more_than<nbest>);
            m_elements[m_nelem - 1] = *item;
            push_heap(begin(), end(), trellis_value_more_than<nbest>);
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

    /* return true if the item is stored into m_element. */
    bool eval_item(const trellis_value_t * item) {
        if (trellis_value_less_than<1>(&m_element, item)) {
            m_element = *item;
            return true;
        }

        return false;
    }
};


#endif
