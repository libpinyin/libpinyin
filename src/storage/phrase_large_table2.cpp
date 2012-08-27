/* 
 *  libpinyin
 *  Library to deal with pinyin.
 *  
 *  Copyright (C) 2012 Peng Wu <alexepico@gmail.com>
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

#include <assert.h>
#include <string.h>
#include "phrase_large_table2.h"


/* class definition */

namespace pinyin{

class PhraseLengthIndexLevel2{
protected:
    GArray * m_phrase_array_indexes;
public:
    PhraseLengthIndexLevel2();
    ~PhraseLengthIndexLevel2();

    /* load/store method */
    bool load(MemoryChunk * chunk, table_offset_t offset, table_offset_t end);
    bool store(MemoryChunk * new_chunk, table_offset_t offset, table_offset_t & end);

    /* search method */
    int search(int phrase_length, /* in */ ucs4_t phrase[],
               /* out */ PhraseTokens tokens) const;

    /* add_index/remove_index method */
    int add_index(int phrase_length, /* in */ ucs4_t phrase[],
                  /* in */ phrase_token_t token);
    int remove_index(int phrase_length, /* in */ ucs4_t phrase[],
                     /* in */ phrase_token_t token);
};


template<size_t phrase_length>
struct PhraseIndexItem2{
    phrase_token_t m_token;
    ucs4_t m_phrase[phrase_length];
public:
    PhraseIndexItem2<phrase_length>(ucs4_t phrase[], phrase_token_t token){
        memmove(m_phrase, phrase, sizeof(ucs4_t) * phrase_length);
        m_token = token;
    }
};


template<size_t phrase_length>
class PhraseArrayIndexLevel2{
protected:
    typedef PhraseIndexItem2<phrase_length> IndexItem;

protected:
    MemoryChunk m_chunk;
public:
    bool load(MemoryChunk * chunk, table_offset_t offset, table_offset_t end);
    bool store(MemoryChunk * new_chunk, table_offset_t offset, table_offset_t & end);

    /* search method */
    int search(/* in */ ucs4_t phrase[], /* out */ PhraseTokens tokens) const;

    /* add_index/remove_index method */
    int add_index(/* in */ ucs4_t phrase[], /* in */ phrase_token_t token);
    int remove_index(/* in */ ucs4_t phrase[], /* in */ phrase_token_t token);
};

};

using namespace pinyin;

/* class implementation */

template<size_t phrase_length>
static int phrase_compare(const PhraseIndexItem2<phrase_length> &lhs,
                          const PhraseIndexItem2<phrase_length> &rhs){
    ucs4_t * phrase_lhs = (ucs4_t *) lhs.m_phrase;
    ucs4_t * phrase_rhs = (ucs4_t *) rhs.m_phrase;

    return memcmp(phrase_lhs, phrase_rhs, sizeof(ucs4_t) * phrase_length);
}

template<size_t phrase_length>
static bool phrase_less_than(const PhraseIndexItem2<phrase_length> & lhs,
                             const PhraseIndexItem2<phrase_length> & rhs){
    return 0 > phrase_compare(lhs, rhs);
}

PhraseBitmapIndexLevel2::PhraseBitmapIndexLevel2(){
    memset(m_phrase_length_indexes, 0, sizeof(m_phrase_length_indexes));
}

void PhraseBitmapIndexLevel2::reset(){
    for ( size_t i = 0; i < PHRASE_NUMBER_OF_BITMAP_INDEX; i++){
        PhraseLengthIndexLevel2 * length_array =
            m_phrase_length_indexes[i];
        if ( length_array )
            delete length_array;
    }
}

int PhraseBitmapIndexLevel2::search(int phrase_length,
                                    /* in */ ucs4_t phrase[],
                                    /* out */ PhraseTokens tokens) const {
    assert(phrase_length > 0);

    int result = SEARCH_NONE;
    /* use the first 8-bit of the lower 16-bit for bitmap index,
     * as most the higher 16-bit are zero.
     */
    guint8 first_key = (phrase[0] & 0xFF00) >> 8;

    PhraseLengthIndexLevel2 * phrase_array = m_phrase_length_indexes[first_key];
    if ( phrase_array )
        return phrase_array->search(phrase_length, phrase, tokens);
    return result;
}
