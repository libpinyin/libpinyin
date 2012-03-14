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

/**
 * FacadePhraseTable:
 *
 * The facade class of phrase large table.
 *
 */

class FacadePhraseTable{
private:
    PhraseLargeTable * m_system_phrase_table;
    PhraseLargeTable * m_user_phrase_table;

public:
    /**
     * FacadePhraseTable::FacadePhraseTable:
     *
     * The constructor of the FacadePhraseTable.
     *
     */
    FacadePhraseTable() {
        m_system_phrase_table = NULL;
        m_user_phrase_table = NULL;
    }

    /**
     * FacadePhraseTable::load:
     * @system: the memory chunk of the system phrase table.
     * @user: the memory chunk of the user phrase table.
     * @returns: whether the load operation is successful.
     *
     * Load the system or user phrase table from the memory chunks.
     *
     */
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

    /**
     * FacadePhraseTable::store:
     * @new_user: the memory chunk to store the user phrase table.
     * @returns: whether the store operation is successful.
     *
     * Store the user phrase table to the memory chunk.
     *
     */
    bool store(MemoryChunk * new_user) {
        if (NULL == m_user_phrase_table)
            return false;
        return m_user_phrase_table->store(new_user);
    }

    /**
     * FacadePhraseTable::search:
     * @phrase_length: the length of the phrase to be searched.
     * @phrase: the ucs4 characters of the phrase to be searched.
     * @token: the token to store the matched phrase.
     * @returns: the search result of enum SearchResult.
     *
     * Search the phrase token according to the ucs4 characters.
     *
     */
    int search(int phrase_length, /* in */ ucs4_t phrase[],
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

    /**
     * FacadePhraseTable::add_index:
     * @phrase_length: the length of the phrase to be added.
     * @phrase: the ucs4 characters of the phrase to be added.
     * @token: the token of the phrase to be added.
     * @returns: the add result of enum AddIndexResult.
     *
     * Add the phrase token to the user phrase table.
     *
     */
    int add_index(int phrase_length, /* in */ ucs4_t phrase[],
                  /* in */ phrase_token_t token) {
        assert(NULL != m_user_phrase_table);
        return m_user_phrase_table->add_index
            (phrase_length, phrase, token);
    }

    /**
     * FacadePhraseTable::remove_index:
     * @phrase_length: the length of the phrase to be removed.
     * @phrase: the ucs4 characters of the phrase to be removed.
     * @token: the token of the phrase to be removed.
     * @returns: the remove result of enum RemoveIndexResult.
     *
     * Remove the phrase token from the user phrase table.
     *
     */
    int remove_index(int phrase_length, /* in */ ucs4_t phrase[],
                     /* out */ phrase_token_t & token){
        assert(NULL != m_user_phrase_table);
        return m_user_phrase_table->remove_index
            (phrase_length, phrase, token);
    }
};

};


#endif
