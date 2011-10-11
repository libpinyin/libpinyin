/* 
 *  libpinyin
 *  Library to deal with pinyin.
 *  
 *  Copyright (C) 2010 Peng Wu
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

#ifndef PHRASE_LARGE_TABLE_H
#define PHRASE_LARGE_TABLE_H

#include <stdio.h>
#include "novel_types.h"
#include "memory_chunk.h"

namespace pinyin{

const size_t PHRASE_Number_Of_Bitmap_Index = 1<<(sizeof(utf16_t) * 8);

class PhraseLengthIndexLevel;

class PhraseBitmapIndexLevel{
protected:
    PhraseLengthIndexLevel * m_phrase_length_indexes[PHRASE_Number_Of_Bitmap_Index];
    //shift one utf16_t for class PhraseLengthIndexLevel, just like PinyinLengthIndexLevel.
    void reset();
public:
    PhraseBitmapIndexLevel();
    ~PhraseBitmapIndexLevel(){
        reset();
    }

    /* load/store method */
    bool load(MemoryChunk * chunk, table_offset_t offset, table_offset_t end);
    bool store(MemoryChunk * new_chunk, table_offset_t offset, table_offset_t & end);

    /* search/add_index/remove_index method */
    int search( int phrase_length, /* in */ utf16_t phrase[],
                /* out */ phrase_token_t & token);

    int add_index( int phrase_length, /* in */ utf16_t phrase[], /* in */ phrase_token_t token);
    int remove_index( int phrase_length, /* in */ utf16_t phrase[], /* out */ phrase_token_t & token);
};

class PhraseLargeTable{
protected:
    PhraseBitmapIndexLevel m_bitmap_table;
    MemoryChunk * m_chunk;

    void reset(){
        if ( m_chunk ){
            delete m_chunk;
            m_chunk = NULL;
        }
    }
public:
    PhraseLargeTable(){
        m_chunk = NULL;
    }

    ~PhraseLargeTable(){
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

    /* search/add_index/remove_index method */
    int search( int phrase_length, /* in */ utf16_t phrase[],
                /* out */ phrase_token_t & token){
        return m_bitmap_table.search(phrase_length, phrase, token);
    }

    int add_index( int phrase_length, /* in */ utf16_t phrase[], /* in */ phrase_token_t token){
        return m_bitmap_table.add_index(phrase_length, phrase, token);
    }

    int remove_index( int phrase_length, /* in */ utf16_t phrase[], /* out */ phrase_token_t & token){
        return m_bitmap_table.remove_index(phrase_length, phrase, token);
    }
};

};

#endif
