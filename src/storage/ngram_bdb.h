/* 
 *  libpinyin
 *  Library to deal with pinyin.
 *  
 *  Copyright (C) 2013 Peng Wu <alexepico@gmail.com>
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

#ifndef NGRAM_BDB_H
#define NGRAM_BDB_H

#include <db.h>

namespace pinyin{

class SingleGram;

/**
 * Bigram:
 *
 * The Bi-gram class.
 *
 */
class Bigram{
private:
    DB * m_db;

    void reset();

public:
    /**
     * Bigram::Bigram:
     *
     * The constructor of the Bigram.
     *
     */
    Bigram();

    /**
     * Bigram::~Bigram:
     *
     * The destructor of the Bigram.
     *
     */
    ~Bigram();

    /**
     * Bigram::load_db:
     * @dbfile: the Berkeley DB file name.
     * @returns: whether the load operation is successful.
     *
     * Load the Berkeley DB into memory.
     *
     */
    bool load_db(const char * dbfile);

    /**
     * Bigram::save_db:
     * @dbfile: the Berkeley DB file name.
     * @returns: whether the save operation is successful.
     *
     * Save the in-memory Berkeley DB into disk.
     *
     */
    bool save_db(const char * dbfile);

    /**
     * Bigram::attach:
     * @dbfile: the Berkeley DB file name.
     * @flags: the flags of enum ATTACH_FLAG.
     * @returns: whether the attach operation is successful.
     *
     * Attach this Bigram with the Berkeley DB.
     *
     */
    bool attach(const char * dbfile, guint32 flags);

    /**
     * Bigram::load:
     * @index: the previous token in the bi-gram.
     * @single_gram: the single gram of the previous token.
     * @copy: whether copy content to the single gram.
     * @returns: whether the load operation is successful.
     *
     * Load the single gram of the previous token.
     *
     */
    bool load(/* in */ phrase_token_t index,
              /* out */ SingleGram * & single_gram, bool copy=false);

    /**
     * Bigram::store:
     * @index: the previous token in the bi-gram.
     * @single_gram: the single gram of the previous token.
     * @returns: whether the store operation is successful.
     *
     * Store the single gram of the previous token.
     *
     */
    bool store(/* in */ phrase_token_t index,
               /* in */ SingleGram * single_gram);

    /**
     * Bigram::remove:
     * @index: the previous token in the bi-gram.
     * @returns: whether the remove operation is successful.
     *
     * Remove the single gram of the previous token.
     *
     */
    bool remove(/* in */ phrase_token_t index);

    /**
     * Bigram::get_all_items:
     * @items: the GArray to store all previous tokens.
     * @returns: whether the get operation is successful.
     *
     * Get the array of all previous tokens for parameter estimation.
     *
     */
    bool get_all_items(/* out */ GArray * items);

    /**
     * Bigram::mask_out:
     * @mask: the mask.
     * @value: the value.
     * @returns: whether the mask out operation is successful.
     *
     * Mask out the matched items.
     *
     */
    bool mask_out(phrase_token_t mask, phrase_token_t value);
};

};

#endif
