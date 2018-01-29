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

#ifndef PHONETIC_LOOKUP_LINEAR_H
#define PHONETIC_LOOKUP_LINEAR_H

template <gint32 nstore>
struct trellis_node {
private:
    gint32 m_nelem;
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

        /* still have space */
        if (m_nelem < nstore) {
            m_elements[m_nelem] = *item;
            m_nelem ++;
            return true;
        }

        /* find minium item */
        trellis_value_t * min = m_elements;
        for (gint32 i = 1; i < m_nelem; ++i) {
            if (trellis_value_less_than<nstore>(m_elements + i, min))
                min = m_elements + i;
        }

        /* compare new item */
        if (trellis_value_less_than<nstore>(min, item)) {
            *min = *item;
            return true;
        }

        return false;
    }
};

#endif
