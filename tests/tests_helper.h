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

static bool load_phrase_index(const pinyin_table_info_t * phrase_files,
                              FacadePhraseIndex * phrase_index){
    MemoryChunk * chunk = NULL;
    for (size_t i = 0; i < PHRASE_INDEX_LIBRARY_COUNT; ++i) {
        const pinyin_table_info_t * table_info = phrase_files + i;

        if (SYSTEM_FILE != table_info->m_file_type)
            continue;

        const char * binfile = table_info->m_system_filename;

        gchar * filename = g_build_filename("..", "..", "data",
                                            binfile, NULL);
        chunk = new MemoryChunk;
        bool retval = chunk->load(filename);
        if (!retval) {
            fprintf(stderr, "open %s failed!\n", binfile);
            delete chunk;
            return false;
        }

        phrase_index->load(i, chunk);
        g_free(filename);
    }
    return true;
}

static bool load_phrase_table(const pinyin_table_info_t * phrase_files,
                              ChewingLargeTable * chewing_table,
                              PhraseLargeTable2 * phrase_table,
                              FacadePhraseIndex * phrase_index){
    for (size_t i = 0; i < PHRASE_INDEX_LIBRARY_COUNT; ++i) {
        const pinyin_table_info_t * table_info = phrase_files + i;

        if (SYSTEM_FILE != table_info->m_file_type)
            continue;

        const char * tablename = table_info->m_table_filename;

        gchar * filename = g_build_filename("..", "..", "data",
                                            tablename, NULL);
        FILE * tablefile = fopen(filename, "r");
        if (NULL == tablefile) {
            fprintf(stderr, "open %s failed!\n", tablename);
            return false;
        }
        g_free(filename);

        if (chewing_table)
            chewing_table->load_text(tablefile);
        fseek(tablefile, 0L, SEEK_SET);
        if (phrase_table)
            phrase_table->load_text(tablefile);
        fseek(tablefile, 0L, SEEK_SET);
        if (phrase_index)
            phrase_index->load_text(i, tablefile);
        fclose(tablefile);
    }
    return true;
}

#endif
