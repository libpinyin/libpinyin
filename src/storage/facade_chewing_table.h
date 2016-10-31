/* 
 *  libpinyin
 *  Library to deal with pinyin.
 *  
 *  Copyright (C) 2011 Peng Wu <alexepico@gmail.com>
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

#ifndef FACADE_CHEWING_TABLE_H
#define FACADE_CHEWING_TABLE_H

#include "novel_types.h"
#include "chewing_large_table.h"

namespace pinyin{

/**
 * FacadeChewingTable:
 *
 * The facade class of chewing large table.
 *
 */

class FacadeChewingTable{
private:
    ChewingLargeTable * m_system_chewing_table;
    ChewingLargeTable * m_user_chewing_table;

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
     * FacadeChewingTable::FacadeChewingTable:
     *
     * The constructor of the FacadeChewingTable.
     *
     */
    FacadeChewingTable() {
        m_system_chewing_table = NULL;
        m_user_chewing_table = NULL;
    }

    /**
     * FacadeChewingTable::~FacadeChewingTable:
     *
     * The destructor of the FacadeChewingTable.
     *
     */
    ~FacadeChewingTable() {
        reset();
    }

    /**
     * FacadeChewingTable::set_options:
     * @options: the pinyin options.
     * @returns: whether the setting options is successful.
     *
     * Set the options of the system and user chewing table.
     *
     */
    bool set_options(pinyin_option_t options) {
        bool result = false;
        if (m_system_chewing_table)
            result = m_system_chewing_table->set_options(options)  || result;
        if (m_user_chewing_table)
            result = m_user_chewing_table->set_options(options) || result;
        return result;
    }

    /**
     * FacadeChewingTable::load:
     * @options: the pinyin options.
     * @system_filename: the file name of the system chewing table.
     * @user_filename: the file name of the user chewing table.
     * @returns: whether the load operation is successful.
     *
     * Load the system or user chewing table from the files.
     *
     */
    bool load(pinyin_option_t options, const char * system_filename,
              const char * user_filename){
        reset();

        MemoryChunk * chunk = NULL;

        bool result = false;

        /* load system chewing table. */
        if (system_filename) {
            chunk = new MemoryChunk;

#ifdef LIBPINYIN_USE_MMAP
            if (!chunk->mmap(system_filename)) {
                fprintf(stderr, "mmap %s failed!\n", system_filename);
                return false;
            }
#else
            if (!chunk->load(system_filename)) {
                fprintf(stderr, "open %s failed!\n", system_filename);
                return false;
            }
#endif

            m_system_chewing_table = new ChewingLargeTable(options);
            result = m_system_chewing_table->load(chunk) || result;
            chunk = NULL;
        }

        /* load user chewing table */
        if (user_filename) {
            chunk = new MemoryChunk;

            if (!chunk->load(user_filename)) {
                /* hack here: use local Chewing Table to create empty memory chunk. */
                ChewingLargeTable table(options);
                table.store(chunk);
            }

            m_user_chewing_table = new ChewingLargeTable(options);
            result = m_user_chewing_table->load(chunk) || result;
        }

        return result;
    }

    /**
     * FacadeChewingTable::store:
     * @new_user_filename: the file name to store the user chewing table.
     * @returns: whether the store operation is successful.
     *
     * Store the user chewing table to the file.
     *
     */
    bool store(const char * new_user_filename) {
        if (NULL == m_user_chewing_table)
            return false;

        /* save user chewing table */
        MemoryChunk * chunk = new MemoryChunk;
        bool retval = m_user_chewing_table->store(chunk);
        assert(retval);

        retval = chunk->save(new_user_filename);
        delete chunk;
        return retval;
    }

    /**
     * FacadeChewingTable::search:
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

    /**
     * FacadeChewingTable::add_index:
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
     * FacadeChewingTable::remove_index:
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
     * FacadeChewingTable::mask_out:
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
