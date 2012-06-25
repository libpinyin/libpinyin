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
#include "utils_helper.h"

/* increase all unigram frequency by a constant. */

int main(int argc, char * argv[]){
    MemoryChunk * chunk = NULL;

    FacadePhraseIndex phrase_index;
    if (!load_phrase_index(&phrase_index))
        exit(ENOENT);

    /* Note: please increase the value when corpus size becomes larger.
     *  To avoid zero value when computing unigram frequency in float format.
     */
    for (size_t i = 0; i < PHRASE_INDEX_LIBRARY_COUNT; ++i) {
        const pinyin_table_info_t * table_info = pinyin_phrase_files + i;

        if (SYSTEM_FILE != table_info->m_file_type)
            continue;

        const char * binfile = table_info->m_system_filename;

        guint32 freq = 1; PhraseIndexRange range;
        int result = phrase_index.get_range(i, range);
        if ( result == ERROR_OK ) {
            for (size_t token = range.m_range_begin;
                  token <= range.m_range_end; ++token) {
                phrase_index.add_unigram_frequency(token, freq);
            }
        }
    }

    if (!save_phrase_index(&phrase_index))
        exit(ENOENT);

    return 0;
}
