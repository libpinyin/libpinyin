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

#ifndef PHRASE_LARGE_TABLE_H
#define PHRASE_LARGE_TABLE_H

class PhraseLargeTable{
protected:
    MemoryChunk * m_chunk;

public:
    /* load/store method */
    bool load(MemoryChunk * chunk);
    bool store(MemoryChunk * new_chunk);

    bool load_text(FILE * file);

    /* search/add_index/remove_index method */
    int search( int phrase_length, /* in */ utf16_t phrase[],
                /* out */ phrase_token_t & token);

    int add_index( int phrase_length, /* in */ utf16_t phrase[], /* in */ phrase_token_t token);
    int remove_index( int phrase_length, /* in */ utf16_t phrase[], /* out */ phrase_token_t & token);
};

#endif
