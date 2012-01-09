/* 
 *  libpinyin
 *  Library to deal with pinyin.
 *  
 *  Copyright (C) 2011 Peng Wu <alexepico@gmail.com>
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

#ifndef FACADE_CHEWING_TABLE_H
#define FACADE_CHEWING_TABLE_H

#include "novel_types.h"
#include "chewing_large_table.h"

namespace pinyin{

class FacadeChewingTable{
private:
    ChewingLargeTable * m_system_chewing_table;
    ChewingLargeTable * m_user_chewing_table;

public:
    /* constructor/destructor */
    FacadeChewingTable() {
        m_system_chewing_table = NULL;
        m_user_chewing_table = NULL;
    }

    /* set options method */
    bool set_options(pinyin_option_t options) {
        bool result = false;
        if (m_system_chewing_table)
            result = m_system_chewing_table->set_options(options)  || result;
        if (m_user_chewing_table)
            result = m_user_chewing_table->set_options(options) || result;
        return result;
    }

    /* load/store method */
    bool load(pinyin_option_t options, MemoryChunk * system,
              MemoryChunk * user){
        bool result = false;
        if (system) {
            m_system_chewing_table = new ChewingLargeTable(options);
            result = m_system_chewing_table->load(system) || result;
        }
        if (user) {
            m_user_chewing_table = new ChewingLargeTable(options);
            result = m_user_chewing_table->load(user) || result;
        }
        return result;
    }

    /* search method */
    int search(int phrase_length, /* in */ ChewingKey keys[],
               /* out */ PhraseIndexRanges ranges) const {

        /* clear ranges. */
        for (size_t i = 0; i < PHRASE_INDEX_LIBRARY_COUNT; ++i) {
            if (ranges[i])
                g_array_set_size(ranges[i], 0);
        }

        int result = SEARCH_NONE;

        if (NULL != m_system_chewing_table)
            result |= m_system_chewing_table->search
                (phrase_length, keys, ranges);

        if (NULL != m_user_chewing_table)
            result |= m_user_chewing_table->search
                (phrase_length, keys, ranges);

        return result;
    }

    /* add/remove index method */
    int add_index(int phrase_length, /* in */ ChewingKey keys[],
                  /* in */ phrase_token_t token) {
        assert(NULL != m_user_chewing_table);
        return m_user_chewing_table->add_index(phrase_length, keys, token);
    }

    int remove_index(int phrase_length, /* in */ ChewingKey keys[],
                     /* in */ phrase_token_t token) {
        assert(NULL != m_user_chewing_table);
        return m_user_chewing_table->remove_index(phrase_length, keys, token);
    }
};

};

#endif
