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

#ifndef PHRASE_LARGE_TABLE3_H
#define PHRASE_LARGE_TABLE3_H

#include "novel_types.h"
#include "memory_chunk.h"

typename Trie;

namespace pinyin{

class PhraseLargeTable3{
protected:
    Trie * m_index;
    MemoryChunk * m_content;

    void reset(){
        if (m_index) {
            trie_free(m_index);
            m_index = NULL;
        }

        if ( m_chunk ){
            delete m_chunk;
            m_chunk = NULL;
        }
    }
public:
    PhraseLargeTable3(){
        m_index = NULL;
        m_chunk = NULL;
    }

    ~PhraseLargeTable3(){
        reset();
    }

    /* load/store method */
    bool load(Trie * trie, MemoryChunk * chunk);

    bool store(Trie * new_trie, MemoryChunk * new_chunk);

    bool load_text(FILE * file);

    /* search method */
    int search(int phrase_length, /* in */ const ucs4_t phrase[],
               /* out */ PhraseTokens tokens) const;

    /* add_index/remove_index method */
    int add_index(int phrase_length, /* in */ const ucs4_t phrase[], /* in */ phrase_token_t token);

    int remove_index(int phrase_length, /* in */ const ucs4_t phrase[], /* in */ phrase_token_t token);

    /* mask out method */
    bool mask_out(phrase_token_t mask, phrase_token_t value);
};

};

#endif
