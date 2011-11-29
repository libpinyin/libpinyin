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

#include "chewing_large_table.h"
#include <assert.h>
#include "pinyin_phrase2.h"


/* internal class definition */

namespace pinyin{
class ChewingLengthIndexLevel{

protected:
    GArray * m_chewing_array_indexes;

public:
    /* constructor/destructor */
    ChewingLengthIndexLevel();
    ~ChewingLengthIndexLevel();

    /* load/store method */
    bool load(MemoryChunk * chunk, table_offset_t offset, table_offset_t end);
    bool store(MemoryChunk * new_chunk, table_offset_t offset,
               table_offset_t & end);

    /* search method */
    int search(pinyin_option_t options, int phrase_length,
               /* in */ ChewingKey keys[],
               /* out */ PhraseIndexRanges ranges);

    /* add/remove index method */
    int add_index(int phrase_length, /* in */ ChewingKey keys[],
                  /* in */ phrase_token_t token);
    int remove_index(int phrase_length, /* in */ ChewingKey keys[],
                     /* in */ phrase_token_t token);
};


template<size_t phrase_length>
class ChewingArrayIndexLevel{
protected:
    MemoryChunk m_chunk;

    /* compress consecutive tokens */
    int convert(pinyin_option_t options,
                ChewingKey keys[],
                PinyinIndexItem2<phrase_length> * begin,
                PinyinIndexItem2<phrase_length> * end,
                PhraseIndexRanges ranges);

public:
    /* load/store method */
    bool load(MemoryChunk * chunk, table_offset_t offset, table_offset_t end);
    bool store(MemoryChunk * new_chunk, table_offset_t offset,
               table_offset_t & end);

    /* search method */
    int search(pinyin_option_t options, /* in */ChewingKey keys[],
               /* out */ PhraseIndexRanges ranges);

    /* add/remove index method */
    int add_index(/* in */ ChewingKey keys[], /* in */ phrase_token_t token);
    int remove_index(/* in */ ChewingKey keys[],
                     /* in */ phrase_token_t token);
};

};


using namespace pinyin;

/* class implementation */
