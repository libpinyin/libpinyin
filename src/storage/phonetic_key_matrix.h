/* 
 *  libpinyin
 *  Library to deal with pinyin.
 *  
 *  Copyright (C) 2016 Peng Wu <alexepico@gmail.com>
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

#ifndef PHONETIC_KEY_MATRIX_H
#define PHONETIC_KEY_MATRIX_H

#include <assert.h>
#include "novel_types.h"
#include "chewing_key.h"
#include "facade_chewing_table2.h"
#include "phrase_index.h"

namespace pinyin {

template<typename Item>
class PhoneticTable {
protected:
    /* Pointer Array of Array of Item. */
    GPtrArray * m_table_content;

public:
    PhoneticTable() {
        m_table_content = g_ptr_array_new();
    }

    ~PhoneticTable() {
        clear_all();

        g_ptr_array_free(m_table_content, TRUE);
        m_table_content = NULL;
    }

    bool clear_all() {
        for (size_t i = 0; i < m_table_content->len; ++i) {
            GArray * column = (GArray *)
                g_ptr_array_index(m_table_content, i);
            g_array_free(column, TRUE);
        }

        g_ptr_array_set_size(m_table_content, 0);
        return true;
    }

    size_t size() const {
        return m_table_content->len;
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
    bool get_items(size_t index, GArray * items) const {
        g_array_set_size(items, 0);

        if (index >= m_table_content->len)
            return false;

        GArray * column = (GArray *)
            g_ptr_array_index(m_table_content, index);
        g_array_append_vals(items, column->data, column->len);
        return true;
    }

    bool append(size_t index, const Item & item) {
        if (index >= m_table_content->len)
            return false;

        GArray * column = (GArray *)
            g_ptr_array_index(m_table_content, index);
        g_array_append_val(column, item);
        return true;
    }

    size_t get_column_size(size_t index) const {
        assert(index < m_table_content->len);

        GArray * column = (GArray *)
            g_ptr_array_index(m_table_content, index);
        return column->len;
    }

    bool get_item(size_t index, size_t row, Item & item) const {
        assert(index < m_table_content->len);

        GArray * column = (GArray *)
            g_ptr_array_index(m_table_content, index);
        assert(row < column->len);

        item = g_array_index(column, Item, row);
        return true;
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

    size_t size() const {
        assert(m_keys.size() == m_key_rests.size());
        return m_keys.size();
    }

    /* reserve one extra slot, same as PhoneticTable. */
    bool set_size(size_t size) {
        return m_keys.set_size(size) && m_key_rests.set_size(size);
    }

    /* Array of keys and key rests. */
    bool get_items(size_t index, GArray * keys, GArray * key_rests) const {
        bool result = m_keys.get_items(index, keys) &&
            m_key_rests.get_items(index, key_rests);

        assert(keys->len == key_rests->len);
        return result;
    }

    bool append(size_t index, const ChewingKey & key,
                const ChewingKeyRest & key_rest) {
        return m_keys.append(index, key) &&
            m_key_rests.append(index, key_rest);
    }

    size_t get_column_size(size_t index) const {
        const size_t size = m_keys.get_column_size(index);
        assert(size == m_key_rests.get_column_size(index));
        return size;
    }

    bool get_item(size_t index, size_t row,
                  ChewingKey & key, ChewingKeyRest & key_rest) const {
        return m_keys.get_item(index, row, key) &&
            m_key_rests.get_item(index, row, key_rest);
    }

};

/**
 * fill_matrix:
 * Convert ChewingKeyVector and ChewingKeyRestVector
 * to PhoneticKeyMatrix.
 */
bool fill_matrix(PhoneticKeyMatrix * matrix,
                 ChewingKeyVector keys,
                 ChewingKeyRestVector key_rests,
                 size_t parsed_len);

/**
 * resplit_step:
 * For "fa'nan" => "fan'an", add "fan'an" to the matrix for matched "fa'nan".
 * Currently only support this feature in Hanyu Pinyin.
 */
bool resplit_step(pinyin_option_t options,
                  PhoneticKeyMatrix * matrix);

/**
 * inner_split_step:
 * For "xian" => "xi'an", add "xi'an" to the matrix for matched "xian".
 * Currently only support this feature in Hanyu Pinyin.
 */
bool inner_split_step(pinyin_option_t options,
                      PhoneticKeyMatrix * matrix);


/**
 * fuzzy_syllable_step:
 * For "an" <=> "ang", fill the fuzzy pinyins into the matrix.
 * Supported nearly in all pinyin parsers.
 * At most 3 * 2 entries will be added.
 */
bool fuzzy_syllable_step(pinyin_option_t options,
                         PhoneticKeyMatrix * matrix);

bool dump_matrix(PhoneticKeyMatrix * matrix);

int search_matrix(const FacadeChewingTable2 * table,
                  const PhoneticKeyMatrix * matrix,
                  size_t start, size_t end,
                  PhraseIndexRanges ranges);

gfloat compute_pronunciation_possibility(const PhoneticKeyMatrix * matrix,
                                         size_t start, size_t end,
                                         GArray * cached_keys,
                                         PhraseItem & item);

bool increase_pronunciation_possibility(const PhoneticKeyMatrix * matrix,
                                        size_t start, size_t end,
                                        GArray * cached_keys,
                                        PhraseItem & item, gint32 delta);
};

#endif
