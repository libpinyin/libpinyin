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

namespace pinyin {

template<struct Item>
class PhoneticTable {
protected:
    /* Pointer Array of Array of Item. */
    GPtrArray * m_table_content;

public:
    bool clear_all();

    /* when call this function,
       reserve one extra slot for the end slot. */
    bool set_size(size_t size);

    /* Array of Item. */
    bool get_items(size_t index, GArray * items);

    bool append(size_t index, Item item);

};

class PhoneticKeyMatrix {
protected:
    PhoneticTable<ChewingKey> m_keys;
    PhoneticTable<ChewingKeyRest> m_key_rests;

public:
    bool clear_all();

    /* reserve one extra slot, same as PhoneticTable. */
    bool set_size(size_t size);

    /* Array of keys and key rests. */
    bool get_column(size_t index, GArray * keys, GArray * key_rests);

    bool append(size_t index, ChewingKey & key, ChewingKeyRest & key_rest);

};

};

#endif
