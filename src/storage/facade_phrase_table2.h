/* 
 *  libpinyin
 *  Library to deal with pinyin.
 *  
 *  Copyright (C) 2012 Peng Wu <alexepico@gmail.com>
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

#ifndef FACADE_PHRASE_TABLE2_H
#define FACADE_PHRASE_TABLE2_H

#include "phrase_large_table2.h"

namespace pinyin{

/**
 * FacadePhraseTable2:
 *
 * The facade class of phrase large table2.
 *
 */

class FacadePhraseTable2{
private:
    PhraseLargeTable2 * m_system_phrase_table;
    PhraseLargeTable2 * m_user_phrase_table;

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
    /**
     * FacadePhraseTable2::FacadePhraseTable2:
     *
     * The constructor of the FacadePhraseTable2.
     *
     */
    FacadePhraseTable2() {
        m_system_phrase_table = NULL;
        m_user_phrase_table = NULL;
    }

    /**
     * FacadePhraseTable2::~FacadePhraseTable2:
     *
     * The destructor of the FacadePhraseTable2.
     *
     */
    ~FacadePhraseTable2() {
        reset();
    }

    /**
     * FacadePhraseTable2::load:
     * @system_filename: the file name of the system phrase table.
     * @user_filename: the file name of the user phrase table.
     * @returns: whether the load operation is successful.
     *
     * Load the system or user phrase table from the files.
     *
     */
    bool load(const char * system_filename, const char * user_filename) {
        reset();

        MemoryChunk * chunk = NULL;

        bool result = false;

        /* load system phrase table */
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

            m_system_phrase_table = new PhraseLargeTable2;
            result = m_system_phrase_table->load(chunk) || result;
            chunk = NULL;
        }

        /* load user phrase table */
        if (user_filename) {
            chunk = new MemoryChunk;

            if (!chunk->load(user_filename)) {
                /* hack here: use local Phrase Table to create empty memory chunk. */
                PhraseLargeTable2 table;
                table.store(chunk);
            }

            m_user_phrase_table = new PhraseLargeTable2;
            result = m_user_phrase_table->load(chunk) || result;
        }

        return result;
    }

    /**
     * FacadePhraseTable2::store:
     * @new_user_filename: the file name to store the user phrase table.
     * @returns: whether the store operation is successful.
     *
     * Store the user phrase table to the file.
     *
     */
    bool store(const char * new_user_filename) {
        if (NULL == m_user_phrase_table)
            return false;

        /* save user phrase table */
        MemoryChunk * chunk = new MemoryChunk;
        bool retval = m_user_phrase_table->store(chunk);
        assert(retval);

        retval = chunk->save(new_user_filename);
        delete chunk;
        return retval;
    }

    /**
     * FacadePhraseTable2::search:
     * @phrase_length: the length of the phrase to be searched.
     * @phrase: the ucs4 characters of the phrase to be searched.
     * @tokens: the GArray of tokens to store the matched phrases.
     * @returns: the search result of enum SearchResult.
     *
     * Search the phrase tokens according to the ucs4 characters.
     *
     */
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

    /**
     * FacadePhraseTable2::add_index:
     * @phrase_length: the length of the phrase to be added.
     * @phrase: the ucs4 characters of the phrase to be added.
     * @token: the token of the phrase to be added.
     * @returns: the add result of enum ErrorResult.
     *
     * Add the phrase token to the user phrase table.
     *
     */
    int add_index(int phrase_length, /* in */ const ucs4_t phrase[],
                  /* in */ phrase_token_t token) {
        if (NULL == m_user_phrase_table)
            return ERROR_NO_USER_TABLE;

        return m_user_phrase_table->add_index
            (phrase_length, phrase, token);
    }

    /**
     * FacadePhraseTable2::remove_index:
     * @phrase_length: the length of the phrase to be removed.
     * @phrase: the ucs4 characters of the phrase to be removed.
     * @token: the token of the phrase to be removed.
     * @returns: the remove result of enum ErrorResult.
     *
     * Remove the phrase token from the user phrase table.
     *
     */
    int remove_index(int phrase_length, /* in */ const ucs4_t phrase[],
                     /* in */ phrase_token_t token) {
        if (NULL == m_user_phrase_table)
            return ERROR_NO_USER_TABLE;

        return m_user_phrase_table->remove_index
            (phrase_length, phrase, token);
    }

    /**
     * FacadePhraseTable2::mask_out:
     * @mask: the mask.
     * @value: the value.
     * @returns: whether the mask out operation is successful.
     *
     * Mask out the matched phrase index.
     *
     */
    bool mask_out(phrase_token_t mask, phrase_token_t value) {
        if (NULL == m_user_phrase_table)
            return false;

        return m_user_phrase_table->mask_out
            (mask, value);
    }
};

};


#endif
