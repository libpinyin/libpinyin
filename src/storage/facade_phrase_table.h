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

#ifndef FACADE_PHRASE_TABLE_H
#define FACADE_PHRASE_TABLE_H

#include "phrase_large_table.h"

namespace pinyin{

class FacadePhraseTable{
private:
    PhraseLargeTable * m_system_phrase_table;
    PhraseLargeTable * m_user_phrase_table;

public:
    /* constructor/destructor */
    FacadePhraseTable() {
        m_system_phrase_table = NULL;
        m_user_phrase_table = NULL;
    }

    /* load/store methods */
    bool load(MemoryChunk * system, MemoryChunk * user) {
        bool result = false;
        if (system) {
            m_system_phrase_table = new PhraseLargeTable;
            result = m_system_phrase_table->load(system) || result;
        }
        if (user) {
            m_user_phrase_table = new PhraseLargeTable;
            result = m_user_phrase_table->load(user) || result;
        }
        return result;
    }

    bool store(MemoryChunk * new_user) {
        assert(NULL != m_user_phrase_table);
        return m_user_phrase_table->store(new_user);
    }

    /* search method */
    int search(int phrase_length, /* in */ utf16_t phrase[],
               /* out */ phrase_token_t & token){
        int result = SEARCH_NONE;
        token = null_token;

        if (NULL != m_system_phrase_table)
            result |= m_system_phrase_table->search
                (phrase_length, phrase, token);

        if (NULL != m_user_phrase_table)
            result |= m_user_phrase_table->search
                (phrase_length, phrase, token);
        return result;
    }

    /* add/remove index method */
    int add_index(int phrase_length, /* in */ utf16_t phrase[],
                  /* in */ phrase_token_t token) {
        assert(NULL != m_user_phrase_table);
        return m_user_phrase_table->add_index
            (phrase_length, phrase, token);
    }

    int remove_index(int phrase_length, /* in */ utf16_t phrase[],
                     /* out */ phrase_token_t & token){
        assert(NULL != m_user_phrase_table);
        return m_user_phrase_table->remove_index
            (phrase_length, phrase, token);
    }
};

};


#endif
