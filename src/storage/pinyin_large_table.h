/* 
 *  novel-pinyin,
 *  A Simplified Chinese Sentence-Based Pinyin Input Method Engine
 *  Based On Markov Model.
 *  
 *  Copyright (C) 2006-2007 Peng Wu
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
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

#ifndef PINYIN_LARGE_TABLE_H
#define PINYIN_LARGE_TABLE_H

#include <stdio.h>
#include "novel_types.h"
#include "memory_chunk.h"

namespace novel{

/* Because this is not large,
 * Store this in user home directory.
 */

class PinyinLengthIndexLevel;

class PinyinBitmapIndexLevel{
    friend class PinyinLargeTable;
    PinyinCustomSettings * m_custom;
protected:
    PinyinLengthIndexLevel * m_pinyin_length_indexes[PINYIN_Number_Of_Initials]
						     [PINYIN_Number_Of_Finals]
						     [PINYIN_Number_Of_Tones];
    //search function
    int initial_level_search(int word_length, /* in */PinyinKey keys[],
			     /* out */ PhraseIndexRanges ranges) const;
    int final_level_search(PinyinInitial initial, int word_length, /* in */PinyinKey keys[], /* out */ PhraseIndexRanges ranges) const;
    int tone_level_search(PinyinInitial initial, PinyinFinal final, int word_length, /* in */PinyinKey keys[], /* out */ PhraseIndexRanges ranges) const;
    void reset();
public:
    PinyinBitmapIndexLevel(PinyinCustomSettings * custom);
    ~PinyinBitmapIndexLevel(){
	reset();
    }

    bool load(MemoryChunk * chunk, table_offset_t offset, table_offset_t end);
    bool store(MemoryChunk * new_chunk, table_offset_t offset, table_offset_t &end);

    /*bool load_text(FILE * file);*/
    /*bool save_text(FILE * file);*/
    
    /*search/add_index method */
    int search( int phrase_length, /* in */ PinyinKey keys[],
		/* out */ PhraseIndexRanges ranges) const;
    int add_index( int phrase_length, /* in */ PinyinKey keys[] ,/* in */ phrase_token_t token); 
    int remove_index( int phrase_length, /* in */ PinyinKey keys[] ,/* in */ phrase_token_t token);
};

class PinyinLengthIndexLevel{
protected:
    GArray* m_pinyin_array_indexes;
public:
    PinyinLengthIndexLevel();
    ~PinyinLengthIndexLevel();
    bool load(MemoryChunk * chunk, table_offset_t offset, table_offset_t end);
    bool store(MemoryChunk * new_chunk, table_offset_t offset, table_offset_t& end);
    
    /*search/add_index method */
    int search( int phrase_length, /* in */ PinyinCustomSettings * custom,
		/* in */ PinyinKey keys[],
		/* out */ PhraseIndexRanges ranges);
    int add_index( int phrase_length, /* in */ PinyinKey keys[] ,/* in */ phrase_token_t token); 
    int remove_index( int phrase_length, /* in */ PinyinKey keys[] ,/* in */ phrase_token_t token);
};

template<size_t phrase_length>
class PinyinArrayIndexLevel{
protected:
    MemoryChunk m_chunk;
    int convert(PinyinCustomSettings * custom,
		PinyinKey keys[],
		PinyinIndexItem<phrase_length> * begin,
		PinyinIndexItem<phrase_length> * end,
		PhraseIndexRanges ranges);
public:
    bool load(MemoryChunk * chunk, table_offset_t offset, table_offset_t end);
    bool store(MemoryChunk * new_chunk, table_offset_t offset, table_offset_t& end);
    
    /*search/add_index method */
    int search(/* in */ PinyinCustomSettings * custom,
	       /* in */ PinyinKey keys[],
	       /* out */ PhraseIndexRanges ranges);
    int add_index(/* in */ PinyinKey keys[] ,/* in */ phrase_token_t token); 
    int remove_index(/* in */ PinyinKey keys[] ,/* in */ phrase_token_t token);
};


/* TODO: add file version check */
class PinyinLargeTable{
protected:
    PinyinBitmapIndexLevel m_bitmap_table;
    MemoryChunk * m_chunk;

    void reset(){
	if ( m_chunk ){
	    delete m_chunk;
	    m_chunk = NULL;
	}
    }

public:
    PinyinLargeTable(PinyinCustomSettings * custom):
	m_bitmap_table(custom){
	m_chunk = NULL;
    }
    
    ~PinyinLargeTable(){
	reset();
    }

    /*load/save method*/
    bool load(MemoryChunk * chunk){
	reset();
	m_chunk = chunk;
	return m_bitmap_table.load(chunk, 0 , chunk->size());
    }

    bool store(MemoryChunk * new_chunk){
	table_offset_t end;
	return m_bitmap_table.store(new_chunk, 0, end);
    }
    
    bool load_text(FILE * file);
/*
    bool save_text(FILE * file){
	return m_bitmap_table.save_text(file);
    }
*/
    
    /*search/add_index method */
    int search( int phrase_length, /* in */ PinyinKey keys[],
		/* out */ PhraseIndexRanges ranges){
	return m_bitmap_table.search(phrase_length, keys, ranges);
    }

    int add_index( int phrase_length, /* in */ PinyinKey keys[] ,/* in */ phrase_token_t token){
	return m_bitmap_table.add_index(phrase_length, keys, token);
    }
    int remove_index( int phrase_length, /* in */ PinyinKey keys[] ,/* in */ phrase_token_t token){
	return m_bitmap_table.remove_index(phrase_length, keys, token);
    }

    bool has_key(PinyinKey key) const {
	PhraseIndexRanges ranges;
	memset(ranges, 0, sizeof(ranges));
	ranges[1] = g_array_new(FALSE, FALSE, sizeof(PhraseIndexRange));
	int result = m_bitmap_table.search(1, &key, ranges);
	g_array_free(ranges[1], TRUE);
	ranges[1] = NULL;
	return result & SEARCH_OK;
    }
};

};

using namespace novel;
#endif
