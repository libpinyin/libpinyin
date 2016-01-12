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

#ifndef FACADE_PHRASE_TABLE3_H
#define FACADE_PHRASE_TABLE3_H

#include "phrase_large_table3.h"

namespace pinyin{

class FacadePhraseTable3{
private:
    PhraseLargeTable3 * m_system_phrase_table;
    PhraseLargeTable3 * m_user_phrase_table;

    void reset(){
        if (m_system_phrase_table) {
            delete m_system_phrase_table;
            m_system_phrase_table = NULL;
        }

        if (m_user_phrase_table) {
            delete m_user_phrase_table;
            m_user_phrase_table = NULL;
        }
    }

public:
    FacadePhraseTable3() {
        m_system_phrase_table = NULL;
        m_user_phrase_table = NULL;
    }

    ~FacadePhraseTable3() {
        reset();
    }

    bool load(FILE * system_index, MemoryChunk * system_content,
              FILE * user_index, MemoryChunk * user_content) {
        reset();

        bool result = false;
        if (system_index && system_content) {
            m_system_phrase_table = new PhraseLargeTable3;
            result = m_system_phrase_table->load
                (system_index, system_content) || result;
        }
        if (user_index && user_content) {
            m_user_phrase_table = new PhraseLargeTable3;
            result = m_user_phrase_table->load
                (user_index, system_content) || result;
        }
        return result;
    }

    bool store(FILE * new_user_index, MemoryChunk * new_user_content) {
        if (NULL == m_user_phrase_table)
            return false;
        return m_user_phrase_table->store(new_user_index, new_user_content);
    }

        int search(int phrase_length, /* in */ const ucs4_t phrase[],
               /* out */ PhraseTokens tokens) const {
        /* clear tokens. */
        for (size_t i = 0; i < PHRASE_INDEX_LIBRARY_COUNT; ++i) {
            if (tokens[i])
                g_array_set_size(tokens[i], 0);
        }

        int result = SEARCH_NONE;

        if (NULL != m_system_phrase_table)
            result |= m_system_phrase_table->search
                (phrase_length, phrase, tokens);

        if (NULL != m_user_phrase_table)
            result |= m_user_phrase_table->search
                (phrase_length, phrase, tokens);

        return result;
    }

        int add_index(int phrase_length, /* in */ const ucs4_t phrase[],
                  /* in */ phrase_token_t token) {
        if (NULL == m_user_phrase_table)
            return ERROR_NO_USER_TABLE;

        return m_user_phrase_table->add_index
            (phrase_length, phrase, token);
    }

        int remove_index(int phrase_length, /* in */ const ucs4_t phrase[],
                     /* in */ phrase_token_t token) {
        if (NULL == m_user_phrase_table)
            return ERROR_NO_USER_TABLE;

        return m_user_phrase_table->remove_index
            (phrase_length, phrase, token);
    }

        bool mask_out(phrase_token_t mask, phrase_token_t value) {
        if (NULL == m_user_phrase_table)
            return false;

        return m_user_phrase_table->mask_out
            (mask, value);
    }

};

};

#endif
