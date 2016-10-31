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

#ifndef PHRASE_LARGE_TABLE3_BDB_H
#define PHRASE_LARGE_TABLE3_BDB_H

#include <db.h>

namespace pinyin{

class PhraseTableEntry;

class PhraseLargeTable3{
private:
    /* member variables. */
    DB * m_db;

protected:
    PhraseTableEntry * m_entry;

    void reset();

public:
    PhraseLargeTable3();

    ~PhraseLargeTable3(){
        reset();
    }

    /* attach method */
    bool attach(const char * dbfile, guint32 flags);

    /* load/store method */
    /* use in-memory DBM here, for better performance. */
    bool load_db(const char * filename);

    bool store_db(const char * new_filename);

    bool load_text(FILE * infile);

    /* search method */
    int search(int phrase_length, /* in */ const ucs4_t phrase[],
               /* out */ PhraseTokens tokens) const;

    /* add_index/remove_index method */
    int add_index(int phrase_length, /* in */ const ucs4_t phrase[], /* in */ phrase_token_t token);

    int remove_index(int phrase_length, /* in */ const ucs4_t phrase[], /* in */ phrase_token_t token);

    /* mask out method */
    bool mask_out(phrase_token_t mask, phrase_token_t value);
};


};

#endif
