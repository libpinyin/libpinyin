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
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

#include <assert.h>
#include <string.h>
#include "phrase_large_table.h"

PhraseBitmapIndexLevel::PhraseBitmapIndexLevel(){
    memset(m_phrase_length_indexes, 0, sizeof(m_phrase_length_indexes));
}

void PhraseBitmapIndexLevel::reset(){
    for ( int i = 0; i < PHRASE_Number_Of_Bitmap_Index; i++){
        PhraseLengthIndexLevel * length_array =
            m_phrase_length_indexes[i];
        if ( length_array )
            delete length_array;
    }
}

int PhraseBitmapIndexLevel::search( int phrase_length, /* in */ utf16_t phrase[], /* out */ phrase_token_t & token){
    assert(phrase_length > 0);

    int result = SEARCH_NONE;
    utf16_t first_key = phrase[0];

    PhraseLengthIndexLevel * phrase_array = m_phrase_length_indexes[first_key];
    if ( phrase_array )
        return phrase_array->search(phrase_length - 1, phrase + 1, token);
    return result;
}

PhraseLengthIndexLevel::PhraseLengthIndexLevel(){
    m_phrase_array_indexes = g_array_new(FALSE, TRUE, sizeof(void *));
}

PhraseLengthIndexLevel::~PhraseLengthIndexLevel(){
#define CASE(x) case x:                                                 \
    {                                                                   \
        PhraseArrayIndexLevel<x> * array =  g_array_index               \
            (m_phrase_array_indexes, PhraseArrayIndexLevel<x> *, x);    \
        if ( array )                                                    \
            delete array;                                               \
        break;                                                          \
    }

    for ( int i = 0 ; i < m_phrase_array_indexes->len; ++i){
        switch (i){
            CASE(0);
            	    CASE(1);
	    CASE(2);
	    CASE(3);
	    CASE(4);
	    CASE(5);
	    CASE(6);
	    CASE(7);
	    CASE(8);
	    CASE(9);
	    CASE(10);
	    CASE(11);
	    CASE(12);
	    CASE(13);
	    CASE(14);
	    CASE(15);
	default:
	    assert(false);
        }
    }
    g_array_free(m_phrase_array_indexes, TRUE);
#undef CASE
}

int PhraseLengthIndexLevel::search(int phrase_length,
                                   /* in */ utf16_t phrase[],
                                   /* out */ phrase_token_t & token){
    int result = SEARCH_NONE;
    if(m_phrase_array_indexes->len < phrase_length + 1)
        return result;
    if (m_phrase_array_indexes->len > phrase_length + 1)
        result |= SEARCH_CONTINUED;

#define CASE(len) case len:                                             \
    {                                                                   \
        PhraseArrayIndexLevel<len> * array = g_array_index              \
            (m_phrase_array_indexes, PhraseArrayIndexLevel<len> *, len); \
        if ( !array )                                                   \
            return result;                                              \
        result |= array->search(phrase, token);                         \
        return result;                                                  \
    }

    switch ( phrase_length ){
        CASE(0);
	CASE(1);
	CASE(2);
	CASE(3);
	CASE(4);
	CASE(5);
	CASE(6);
	CASE(7);
	CASE(8);
	CASE(9);
	CASE(10);
	CASE(11);
	CASE(12);
	CASE(13);
	CASE(14);
	CASE(15);
    default:
	assert(false);
    }
#undef CASE
}

template<size_t phrase_length>
int PinyinArrayIndexLevel<phrase_length>::search(/* in */ utf16_t phrase[], /* out */ phrase_token_t & token){
    
}
