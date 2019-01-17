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

#ifndef FACADE_CHEWING_TABLE2_H
#define FACADE_CHEWING_TABLE2_H

#include "novel_types.h"
#include "chewing_large_table2.h"

namespace pinyin{

/**
 * FacadeChewingTable2:
 *
 * The facade class of chewing large table2.
 *
 */

class FacadeChewingTable2{
private:
    ChewingLargeTable2 * m_system_chewing_table;
    ChewingLargeTable2 * m_user_chewing_table;

    void reset() {
        if (m_system_chewing_table) {
            delete m_system_chewing_table;
            m_system_chewing_table = NULL;
        }

        if (m_user_chewing_table) {
            delete m_user_chewing_table;
            m_user_chewing_table = NULL;
        }
    }

public:
    /**
     * FacadeChewingTable2::FacadeChewingTable2:
     *
     * The constructor of the FacadeChewingTable2.
     *
     */
    FacadeChewingTable2() {
        m_system_chewing_table = NULL;
        m_user_chewing_table = NULL;
    }

    /**
     * FacadeChewingTable2::~FacadeChewingTable2:
     *
     * The destructor of the FacadeChewingTable2.
     *
     */
    ~FacadeChewingTable2() {
        reset();
    }

    bool load(const char * system_filename,
              const char * user_filename) {
        reset();

        bool result = false;
        if (system_filename) {
            m_system_chewing_table = new ChewingLargeTable2;
            result = m_system_chewing_table->attach
                (system_filename, ATTACH_READONLY) || result;
        }
        if (user_filename) {
            m_user_chewing_table = new ChewingLargeTable2;
            result = m_user_chewing_table->load_db
                (user_filename) || result;
        }
        return result;
    }

    bool store(const char * new_user_filename) {
        if (NULL == m_user_chewing_table)
            return false;
        return m_user_chewing_table->store_db(new_user_filename);
    }

    /**
     * FacadeChewingTable2::search:
     * @phrase_length: the length of the phrase to be searched.
     * @keys: the pinyin key of the phrase to be searched.
     * @ranges: the array of GArrays to store the matched phrase token.
     * @returns: the search result of enum SearchResult.
     *
     * Search the phrase tokens according to the pinyin keys.
     *
     */
    int search(int phrase_length, /* in */ const ChewingKey keys[],
               /* out */ PhraseIndexRanges ranges) const {
#if 0
        /* clear ranges. */
        for (size_t i = 0; i < PHRASE_INDEX_LIBRARY_COUNT; ++i) {
            if (ranges[i])
                g_array_set_size(ranges[i], 0);
        }
#endif
        int result = SEARCH_NONE;

        if (NULL != m_system_chewing_table)
            result |= m_system_chewing_table->search
                (phrase_length, keys, ranges);

        if (NULL != m_user_chewing_table)
            result |= m_user_chewing_table->search
                (phrase_length, keys, ranges);

        return result;
    }

    /**
     * FacadeChewingTable2::add_index:
     * @phrase_length: the length of the phrase to be added.
     * @keys: the pinyin keys of the phrase to be added.
     * @token: the token of the phrase to be added.
     * @returns: the add result of enum ErrorResult.
     *
     * Add the phrase token to the user chewing table.
     *
     */
    int add_index(int phrase_length, /* in */ const ChewingKey keys[],
                  /* in */ phrase_token_t token) {
        if (NULL == m_user_chewing_table)
            return ERROR_NO_USER_TABLE;
        return m_user_chewing_table->add_index(phrase_length, keys, token);
    }

    /**
     * FacadeChewingTable2::remove_index:
     * @phrase_length: the length of the phrase to be removed.
     * @keys: the pinyin keys of the phrase to be removed.
     * @token: the token of the phrase to be removed.
     * @returns: the remove result of enum ErrorResult.
     *
     * Remove the phrase token from the user chewing table.
     *
     */
    int remove_index(int phrase_length, /* in */ const ChewingKey keys[],
                     /* in */ phrase_token_t token) {
        if (NULL == m_user_chewing_table)
            return ERROR_NO_USER_TABLE;
        return m_user_chewing_table->remove_index(phrase_length, keys, token);
    }

    /**
     * FacadeChewingTable2::mask_out:
     * @mask: the mask.
     * @value: the value.
     * @returns: whether the mask out operation is successful.
     *
     * Mask out the matched chewing index.
     *
     */
    bool mask_out(phrase_token_t mask, phrase_token_t value) {
        if (NULL == m_user_chewing_table)
            return false;
        return m_user_chewing_table->mask_out(mask, value);
    }

};

};

#endif
