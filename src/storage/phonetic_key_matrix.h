/* 
 *  libpinyin
 *  Library to deal with pinyin.
 *  
 *  Copyright (C) 2016 Peng Wu <alexepico@gmail.com>
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

#ifndef PHONETIC_KEY_MATRIX_H
#define PHONETIC_KEY_MATRIX_H

#include "novel_types.h"
#include "chewing_key.h"

namespace pinyin {

template<typename Item>
class PhoneticTable {
protected:
    /* Pointer Array of Array of Item. */
    GPtrArray * m_table_content;

public:
    bool clear_all() {
        for (size_t i = 0; i < m_table_content->len; ++i) {
            GArray * column = (GArray *)
                g_ptr_array_index(m_table_content, i);
            g_array_free(column, TRUE);
        }

        g_ptr_array_set_size(m_table_content, 0);
        return true;
    }

    /* when call this function,
       reserve one extra slot for the end slot. */
    bool set_size(size_t size) {
        clear_all();

        g_ptr_array_set_size(m_table_content, size);
        for (size_t i = 0; i < m_table_content->len; ++i) {
            g_ptr_array_index(m_table_content, i) =
                g_array_new(TRUE, TRUE, sizeof(Item));
        }

        return true;
    }

    /* Array of Item. */
    bool get_items(size_t index, GArray * items) {
        g_array_set_size(items, 0);

        if (index >= m_table_content->len)
            return false;

        GArray * column = (GArray *)
            g_ptr_array_index(m_table_content, index);
        g_array_append_vals(items, column->data, column->len);
        return true;
    }

    bool append(size_t index, Item & item) {
        if (index >= m_table_content->len)
            return false;

        GArray * column = (GArray *)
            g_ptr_array_index(m_table_content, index);
        g_array_append_val(column, item);
    }

};

class PhoneticKeyMatrix {
protected:
    PhoneticTable<ChewingKey> m_keys;
    PhoneticTable<ChewingKeyRest> m_key_rests;

public:
    bool clear_all() {
        return m_keys.clear_all() && m_key_rests.clear_all();
    }

    /* reserve one extra slot, same as PhoneticTable. */
    bool set_size(size_t size) {
        return m_keys.set_size(size) && m_key_rests.set_size(size);
    }

    /* Array of keys and key rests. */
    bool get_items(size_t index, GArray * keys, GArray * key_rests) {
        return m_keys.get_items(index, keys) &&
            m_key_rests.get_items(index, key_rests);
    }

    bool append(size_t index, ChewingKey & key, ChewingKeyRest & key_rest) {
        return m_keys.append(index, key) &&
            m_key_rests.append(index, key_rest);
    }

};

bool fill_phonetic_key_matrix_from_chewing_keys(PhoneticKeyMatrix * matrix,
                                                ChewingKeyVector * keys,
                                                ChewingKeyRestVector * key_rests);

};

#endif
