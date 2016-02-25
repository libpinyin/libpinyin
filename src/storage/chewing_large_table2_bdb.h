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

#ifndef CHEWING_LARGE_TABLE2_BDB_H
#define CHEWING_LARGE_TABLE2_BDB_H

#include <stdio.h>
#include <db.h>
#include <glib.h>

namespace pinyin{

class ChewingTableEntry;

class ChewingLargeTable2{
private:
    /* member variables. */
    DB * m_db;

protected:
    /* Array of ChewingTableEntry. */
    GPtrArray * m_entries;

    void reset();

public:
    ChewingLargeTable2();

    ~ChewingLargeTable2() {
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
    int search(int phrase_length, /* in */ const ChewingKey keys[],
               /* out */ PhraseIndexRanges ranges) const;

    /* add/remove index method */
    int add_index(int phrase_length, /* in */ const ChewingKey keys[],
                  /* in */ phrase_token_t token);

    int remove_index(int phrase_length, /* in */ const ChewingKey keys[],
                     /* in */ phrase_token_t token);

    /* mask out method */
    bool mask_out(phrase_token_t mask, phrase_token_t value);
};

};

#endif
