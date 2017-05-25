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

#ifndef CHEWING_LARGE_TABLE_H
#define CHEWING_LARGE_TABLE_H


#include <stdio.h>
#include "novel_types.h"
#include "memory_chunk.h"
#include "chewing_key.h"
#include "table_info.h"

namespace pinyin{

class ChewingLengthIndexLevel;

class ChewingBitmapIndexLevel{

protected:
    pinyin_option_t m_options;

protected:
    ChewingLengthIndexLevel * m_chewing_length_indexes
    [CHEWING_NUMBER_OF_INITIALS][CHEWING_NUMBER_OF_MIDDLES]
    [CHEWING_NUMBER_OF_FINALS][CHEWING_NUMBER_OF_TONES];

    /* search functions */
    int initial_level_search(int phrase_length,
                             /* in */ const ChewingKey keys[],
                             /* out */ PhraseIndexRanges ranges) const;

    int middle_and_final_level_search(ChewingInitial initial,
                                      int phrase_length,
                                      /* in */ const ChewingKey keys[],
                                      /* out */ PhraseIndexRanges ranges) const;
    int tone_level_search(ChewingInitial initial, ChewingMiddle middle,
                          ChewingFinal final, int phrase_length,
                          /* in */ const ChewingKey keys[],
                          /* out */ PhraseIndexRanges ranges) const;

    void reset();

public:
    /* constructor/destructor */
    ChewingBitmapIndexLevel(pinyin_option_t options);
    ~ChewingBitmapIndexLevel() { reset(); }

    /* set options method */
    bool set_options(pinyin_option_t options) {
        m_options = options;
        return true;
    }

    /* load/store method */
    bool load(MemoryChunk * chunk, table_offset_t offset, table_offset_t end);
    bool store(MemoryChunk * new_chunk, table_offset_t offset,
               table_offset_t & end);

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


class ChewingLargeTable{
protected:
    ChewingBitmapIndexLevel m_bitmap_table;
    MemoryChunk * m_chunk;

    void reset(){
        if (m_chunk) {
            delete m_chunk; m_chunk = NULL;
        }
    }

public:
    /* constructor/destructor */
    ChewingLargeTable(pinyin_option_t options):
        m_bitmap_table(options), m_chunk(NULL) {}

    ~ChewingLargeTable() { reset(); }

    /* set options method */
    bool set_options(pinyin_option_t options) {
        return m_bitmap_table.set_options(options);
    }

    /* load/store method */
    bool load(MemoryChunk * chunk) {
        reset();
        m_chunk = chunk;
        return m_bitmap_table.load(chunk, 0, chunk->size());
    }

    bool store(MemoryChunk * new_chunk) {
        table_offset_t end;
        return m_bitmap_table.store(new_chunk, 0, end);
    }

    bool load_text(FILE * file, TABLE_PHONETIC_TYPE type);

    /* search method */
    int search(int phrase_length, /* in */ const ChewingKey keys[],
               /* out */ PhraseIndexRanges ranges) const {
        return m_bitmap_table.search(phrase_length, keys, ranges);
    }

    /* add/remove index method */
    int add_index(int phrase_length, /* in */ const ChewingKey keys[],
                  /* in */ phrase_token_t token) {
        return m_bitmap_table.add_index(phrase_length, keys, token);
    }

    int remove_index(int phrase_length, /* in */ const ChewingKey keys[],
                     /* in */ phrase_token_t token) {
        return m_bitmap_table.remove_index(phrase_length, keys, token);
    }

    /* mask out method */
    bool mask_out(phrase_token_t mask, phrase_token_t value) {
        return m_bitmap_table.mask_out(mask, value);
    }
};

};

#endif
