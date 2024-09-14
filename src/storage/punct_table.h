/* 
 *  libpinyin
 *  Library to deal with pinyin.
 *  
 *  Copyright (C) 2024 Peng Wu <alexepico@gmail.com>
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


#ifndef PUNCT_TABLE_H
#define PUNCT_TABLE_H

#include <glib.h>
#include "memory_chunk.h"

#ifdef HAVE_BERKELEY_DB
#include "punct_table_bdb.h"
#endif

#ifdef HAVE_KYOTO_CABINET
#include "punct_table_kyotodb.h"
#endif

namespace pinyin{

class PunctTable;

/**
 * In order to support some punctuations with variable length,
 * the code store "..." like ".....". The ".." string means
 * this punctuation has another character following ".".
 */
class PunctTableEntry{
    friend class PunctTable;

private:
    /* Disallow used outside. */
    PunctTableEntry();

public:
    virtual ~PunctTableEntry();

public:
    /** Note: The following method requires the puncts.table content is
     *        listed in the decreasing order of frequency.
     */
    /* check duplicated punctuations with get_all_punctuations. */
    bool append_punctuation(const gchar * punct);
    bool remove_punctuation(const gchar * punct);
    bool get_all_punctuations(gchar ** & puncts);

private:
    bool escape(const gchar * punct, gint maxlen = -1);
    int unescape(const ucs4_t * punct, gint maxlen = -1);

private:
    MemoryChunk m_chunk;
    GArray * m_ucs4_cache;
    GString * m_utf8_cache;
};

};

#endif
