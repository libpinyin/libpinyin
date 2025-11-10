/* 
 *  libpinyin
 *  Library to deal with pinyin.
 *  
 *  Copyright (C) 2025 Peng Wu <alexepico@gmail.com>
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


#ifndef PUNCT_TABLE_TKRZWDB_H
#define PUNCT_TABLE_TKRZWDB_H

#include <tkrzw_dbm.h>
#include <tkrzw_dbm_baby.h>
#include "novel_types.h"

namespace pinyin{

class PunctTableEntry;

class PunctTable{
private:
    tkrzw::DBM * m_db;

protected:
    PunctTableEntry * m_entry;

    void reset();

public:
    PunctTable();

    ~PunctTable(){
        reset();
    }

protected:
    bool load_entry(phrase_token_t index);
    bool store_entry(phrase_token_t index);

public:
    bool load_db(const char * dbfile);
    bool save_db(const char * dbfile);
    bool attach(const char * dbfile, guint32 flags);

    bool get_all_punctuations(/* in */ phrase_token_t index,
                              /* out */ gchar ** & puncts);
    bool append_punctuation(/* in */ phrase_token_t index,
                            /* in */ const gchar * punct);
    bool remove_punctuation(/* in */ phrase_token_t index,
                            /* in */ const gchar * punct);

    bool remove_all_punctuations(/* in */ phrase_token_t index);
    bool get_all_items(/* out */ GArray * items);

    bool load_text(FILE * infile);
};

};

#endif
