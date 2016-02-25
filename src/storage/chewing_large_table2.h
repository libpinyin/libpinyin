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

#ifndef CHEWING_LARGE_TABLE2_H
#define CHEWING_LARGE_TABLE2_H

#include "novel_types.h"
#include "memory_chunk.h"
#include "chewing_key.h"

namespace pinyin{

template<size_t phrase_length>
class ChewingTableEntry{
    friend class ChewingLargeTable2;
protected:
    MemoryChunk m_chunk;

    /* cache for storing the chewing keys index. */
    ChewingKey m_cache_index[phrase_length];

private:
    /* Disallow used outside. */
    ChewingTableEntry() {}

public:
    /* search method */
    int search(/* in */ const ChewingKey keys[],
               /* out */ PhraseIndexRanges ranges) const;

    /* add/remove index method */
    int add_index(/* in */ const ChewingKey keys[],
                  /* in */ phrase_token_t token);
    int remove_index(/* in */ const ChewingKey keys[],
                     /* in */ phrase_token_t token);

    /* get length method */
    int get_length() const;

    /* mask out method */
    bool mask_out(phrase_token_t mask, phrase_token_t value);

};

};

#endif
