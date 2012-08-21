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
               /* out */ PhraseTokens tokens);

    /* add_index/remove_index method */
    int add_index(int phrase_length, /* in */ ucs4_t phrase[],
                  /* in */ phrase_token_t token);
    int remove_index(int phrase_length, /* in */ ucs4_t phrase[],
                     /* in */ phrase_token_t token);
};

template<size_t phrase_length>
class PhraseArrayIndexLevel2{
protected:
    MemoryChunk m_chunk;
public:
    bool load(MemoryChunk * chunk, table_offset_t offset, table_offset_t end);
    bool store(MemoryChunk * new_chunk, table_offset_t offset, table_offset_t & end);

    /* search method */
    int search(/* in */ ucs4_t phrase[], /* out */ PhraseTokens tokens);

    /* add_index/remove_index method */
    int add_index(/* in */ ucs4_t phrase[], /* in */ phrase_token_t token);
    int remove_index(/* in */ ucs4_t phrase[], /* in */ phrase_token_t token);
};

};

using namespace pinyin;

/* class implementation */

template<size_t phrase_length>
struct PhraseIndexItem{
    phrase_token_t m_token;
    ucs4_t m_phrase[phrase_length];
public:
    PhraseIndexItem<phrase_length>(ucs4_t phrase[], phrase_token_t token){
        memmove(m_phrase, phrase, sizeof(ucs4_t) * phrase_length);
        m_token = token;
    }
};

template<size_t phrase_length>
static int phrase_compare(const PhraseIndexItem<phrase_length> &lhs,
                          const PhraseIndexItem<phrase_length> &rhs){
    ucs4_t * phrase_lhs = (ucs4_t *) lhs.m_phrase;
    ucs4_t * phrase_rhs = (ucs4_t *) rhs.m_phrase;

    return memcmp(phrase_lhs, phrase_rhs, sizeof(ucs4_t) * phrase_length);
}

template<size_t phrase_length>
static bool phrase_less_than(const PhraseIndexItem<phrase_length> & lhs,
                             const PhraseIndexItem<phrase_length> & rhs){
    return 0 > phrase_compare(lhs, rhs);
}
