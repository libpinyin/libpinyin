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

#include "pinyin_internal.h"


/* TODO: check whether gb_char.bin and gb_char2.bin should be the same. */

int main(int argc, char * argv[]){
    FacadePhraseIndex phrase_index;
    MemoryChunk * chunk = new MemoryChunk;
    chunk->load("../../data/gb_char.bin");
    phrase_index.load(1, chunk);

    PhraseIndexRange range;
    assert(ERROR_OK == phrase_index.get_range(1, range));
    for (size_t i = range.m_range_begin; i < range.m_range_end; ++i ) {
        phrase_index.add_unigram_frequency(i, 1);
    }

    printf("total freq:%d\n", phrase_index.get_phrase_index_total_freq());

    MemoryChunk * new_chunk = new MemoryChunk;
    phrase_index.store(1, new_chunk);
    new_chunk->save("/tmp/gb_char.bin");
    delete new_chunk;

    chunk = new MemoryChunk;
    chunk->load("../../data/gb_char.bin");
    new_chunk = new MemoryChunk;
    assert(phrase_index.diff(1, chunk, new_chunk));
    new_chunk->save("/tmp/gb_char.dbin");
    delete new_chunk;

    chunk = new MemoryChunk;
    chunk->load("../../data/gb_char.bin");
    phrase_index.load(1, chunk);
    new_chunk = new MemoryChunk;
    new_chunk->load("/tmp/gb_char.dbin");
    assert(phrase_index.merge(1, new_chunk));
    chunk = new MemoryChunk;
    phrase_index.store(1, chunk);
    chunk->save("/tmp/gb_char2.bin");
    delete chunk;

    printf("total freq:%d\n", phrase_index.get_phrase_index_total_freq());

    return 0;
}
