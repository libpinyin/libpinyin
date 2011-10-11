/* 
 *  libpinyin
 *  Library to deal with pinyin.
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
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include <stdio.h>
#include "pinyin_internal.h"


/* increase all unigram frequency by a constant. */

int main(int argc, char * argv[]){

    FacadePhraseIndex phrase_index;
    
    /* gb_char binary file */
    MemoryChunk * chunk = new MemoryChunk;
    chunk->load("gb_char.bin");
    phrase_index.load(1, chunk);
    
    /* gbk_char binary file */
    chunk = new MemoryChunk;
    chunk->load("gbk_char.bin");
    phrase_index.load(2, chunk);

    /* Note: please increase the value when corpus size becomes larger.
     *  To avoid zero value when computing unigram frequency in float format.
     */
    guint32 freq = 1; PhraseIndexRange range;
    int result = phrase_index.get_range(1, range);
    if ( result == ERROR_OK ) {
        for ( size_t i = range.m_range_begin; i <= range.m_range_end; ++i ) {
            phrase_index.add_unigram_frequency(i, freq);
        }
    }

#if 1
    result = phrase_index.get_range(2, range);
    if ( result == ERROR_OK ) {
        for ( size_t i = range.m_range_begin; i <= range.m_range_end; ++i ) {
            phrase_index.add_unigram_frequency(i, freq);
        }
    }
#endif

    MemoryChunk * new_chunk = new MemoryChunk;
    phrase_index.store(1, new_chunk);
    new_chunk->save("gb_char.bin");
    phrase_index.load(1, new_chunk);

    new_chunk = new MemoryChunk;
    phrase_index.store(2, new_chunk);
    new_chunk->save("gbk_char.bin");
    phrase_index.load(2, new_chunk);

    return 0;
}
