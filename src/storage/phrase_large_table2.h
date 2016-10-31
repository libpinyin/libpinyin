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

#ifndef PHRASE_LARGE_TABLE2_H
#define PHRASE_LARGE_TABLE2_H

#include <stdio.h>
#include "novel_types.h"
#include "memory_chunk.h"

namespace pinyin{

const size_t PHRASE_NUMBER_OF_BITMAP_INDEX = 1<<(sizeof(ucs4_t) / 4 * 8);

class PhraseLengthIndexLevel2;

class PhraseBitmapIndexLevel2{
protected:
    PhraseLengthIndexLevel2 * m_phrase_length_indexes[PHRASE_NUMBER_OF_BITMAP_INDEX];
    /* use the third byte of ucs4_t for class PhraseLengthIndexLevel2. */
    void reset();
public:
    PhraseBitmapIndexLevel2();
    ~PhraseBitmapIndexLevel2(){
        reset();
    }

    /* load/store method */
    bool load(MemoryChunk * chunk, table_offset_t offset, table_offset_t end);
    bool store(MemoryChunk * new_chunk, table_offset_t offset, table_offset_t & end);

    /* search method */
    int search(int phrase_length, /* in */ const ucs4_t phrase[],
               /* out */ PhraseTokens tokens) const;

    /* add_index/remove_index method */
    int add_index(int phrase_length, /* in */ const ucs4_t phrase[], /* in */ phrase_token_t token);

    int remove_index(int phrase_length, /* in */ const ucs4_t phrase[], /* in */ phrase_token_t token);

    /* mask out method */
    bool mask_out(phrase_token_t mask, phrase_token_t value);
};


class PhraseLargeTable2{
protected:
    PhraseBitmapIndexLevel2 m_bitmap_table;
    MemoryChunk * m_chunk;

    void reset(){
        if ( m_chunk ){
            delete m_chunk;
            m_chunk = NULL;
        }
    }
public:
    PhraseLargeTable2(){
        m_chunk = NULL;
    }

    ~PhraseLargeTable2(){
        reset();
    }

    /* load/store method */
    bool load(MemoryChunk * chunk){
        reset();
        m_chunk = chunk;
        return m_bitmap_table.load(chunk, 0, chunk->size());
    }

    bool store(MemoryChunk * new_chunk){
        table_offset_t end;
        return m_bitmap_table.store(new_chunk, 0, end);
    }

    bool load_text(FILE * file);

    /* search method */
    int search(int phrase_length, /* in */ const ucs4_t phrase[],
               /* out */ PhraseTokens tokens) const {
        return m_bitmap_table.search(phrase_length, phrase, tokens);
    }

    /* add_index/remove_index method */
    int add_index(int phrase_length, /* in */ const ucs4_t phrase[], /* in */ phrase_token_t token) {
        return m_bitmap_table.add_index(phrase_length, phrase, token);
    }

    int remove_index(int phrase_length, /* in */ const ucs4_t phrase[], /* in */ phrase_token_t token) {
        return m_bitmap_table.remove_index(phrase_length, phrase, token);
    }

    /* mask out method */
    bool mask_out(phrase_token_t mask, phrase_token_t value) {
        return m_bitmap_table.mask_out(mask, value);
    }
};


};

#endif
