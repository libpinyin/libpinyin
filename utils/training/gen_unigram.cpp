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

    FacadePhraseIndex phrase_index;

    /* Note: please increase the value when corpus size becomes larger.
     *  To avoid zero value when computing unigram frequency in float format.
     */
    for (size_t i = 0; i < PHRASE_INDEX_LIBRARY_COUNT; ++i) {
        const pinyin_table_info_t * table_info = pinyin_phrase_files + i;
        assert(table_info->m_dict_index == i);

        if (SYSTEM_FILE != table_info->m_file_type &&
            DICTIONARY != table_info->m_file_type)
            continue;

        gint count = 100;
        /* skip GBK_DICTIONARY. */
        if (GBK_DICTIONARY == table_info->m_dict_index)
            count = 1;

        const guint32 unigram_factor = 7;
        guint32 freq = count * unigram_factor;

        const char * binfile = table_info->m_system_filename;

        MemoryChunk * chunk = new MemoryChunk;
        bool retval = chunk->load(binfile);
        if (!retval) {
            fprintf(stderr, "load %s failed!\n", binfile);
            exit(ENOENT);
        }

        phrase_index.load(i, chunk);

        PhraseIndexRange range;
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

    if (!save_dictionary(&phrase_index))
        exit(ENOENT);

    return 0;
}
