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

#ifndef TESTS_HELPER_H
#define TESTS_HELPER_H

static bool load_phrase_index(FacadePhraseIndex * phrase_index){
    MemoryChunk * chunk = NULL;
    for (size_t i = 0; i < PHRASE_INDEX_LIBRARY_COUNT; ++i) {
        const char * binfile = pinyin_phrase_files[i];
        if (NULL == binfile)
            continue;

        gchar * filename = g_build_filename("..", "..", "data",
                                            binfile, NULL);
        chunk = new MemoryChunk;
        bool retval = chunk->load(filename);
        if (!retval) {
            fprintf(stderr, "open %s failed!\n", binfile);
            return false;
        }

        phrase_index->load(i, chunk);
        g_free(filename);
    }
    return true;
}

#endif
