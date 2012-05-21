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


#ifndef UTILS_HELPER_H
#define UTILS_HELPER_H

static bool load_phrase_index(FacadePhraseIndex * phrase_index) {
    MemoryChunk * chunk = NULL;
    for (size_t i = 0; i < PHRASE_INDEX_LIBRARY_COUNT; ++i) {
        const char * bin_file = pinyin_phrase_files[i];
        if (NULL == bin_file)
            continue;

        chunk = new MemoryChunk;
        bool retval = chunk->load(bin_file);
        if (!retval) {
            fprintf(stderr, "load %s failed!\n", bin_file);
            return false;
        }

        phrase_index->load(i, chunk);
    }
    return true;
}

static bool save_phrase_index(FacadePhraseIndex * phrase_index) {
    MemoryChunk * new_chunk = NULL;
    for (size_t i = 0; i < PHRASE_INDEX_LIBRARY_COUNT; ++i) {
        const char * bin_file = pinyin_phrase_files[i];
        if (NULL == bin_file)
            continue;

        new_chunk = new MemoryChunk;
        phrase_index->store(i, new_chunk);
        bool retval = new_chunk->save(bin_file);
        if (!retval) {
            fprintf(stderr, "save %s failed.", bin_file);
            return false;
        }

        phrase_index->load(i, new_chunk);
    }
    return true;
}

#endif
