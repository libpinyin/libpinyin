/* 
 *  libpinyin
 *  Library to deal with pinyin.
 *  
 *  Copyright (C) 2012 Peng Wu <alexepico@gmail.com>
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

#ifndef TESTS_HELPER_H
#define TESTS_HELPER_H

inline bool load_phrase_index(const pinyin_table_info_t * phrase_files,
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

inline bool load_phrase_table(const pinyin_table_info_t * phrase_files,
                              ChewingLargeTable2 * chewing_table,
                              PhraseLargeTable3 * phrase_table,
                              FacadePhraseIndex * phrase_index,
                              TABLE_PHONETIC_TYPE type){
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
            chewing_table->load_text(tablefile, type);
        fseek(tablefile, 0L, SEEK_SET);
        if (phrase_table)
            phrase_table->load_text(tablefile);
        fseek(tablefile, 0L, SEEK_SET);
        if (phrase_index)
            phrase_index->load_text(i, tablefile, type);
        fclose(tablefile);
    }
    return true;
}

inline bool dump_ranges(FacadePhraseIndex * phrase_index,
                        PhraseIndexRanges ranges) {

    for (size_t i = 0; i < PHRASE_INDEX_LIBRARY_COUNT; ++i) {
        GArray * & range = ranges[i];
        if (!range)
            continue;

        if (range->len)
            printf("range items number:%d\n", range->len);

        for (size_t k = 0; k < range->len; ++k) {
            PhraseIndexRange * onerange =
                &g_array_index(range, PhraseIndexRange, k);
            printf("start:%d\tend:%d\n", onerange->m_range_begin,
                   onerange->m_range_end);

            PhraseItem item;
            for (phrase_token_t token = onerange->m_range_begin;
                  token != onerange->m_range_end; ++token){

                phrase_index->get_phrase_item(token, item);

                /* get phrase string */
                ucs4_t buffer[MAX_PHRASE_LENGTH + 1];
                item.get_phrase_string(buffer);
                char * string = g_ucs4_to_utf8
                    ( buffer, item.get_phrase_length(),
                      NULL, NULL, NULL);
                printf("%s\t", string);
                g_free(string);

                ChewingKey chewing_buffer[MAX_PHRASE_LENGTH];
                size_t npron = item.get_n_pronunciation();
                guint32 freq;
                for (size_t m = 0; m < npron; ++m){
                    item.get_nth_pronunciation(m, chewing_buffer, freq);
                    for (size_t n = 0; n < item.get_phrase_length();
                         ++n){
                        gchar * pinyins =
                            chewing_buffer[n].get_pinyin_string();
                        printf("%s'", pinyins);
                        g_free(pinyins);
                    }
                    printf("\b\t%d\t", freq);
                }
            }
            printf("\n");
        }
    }

    return true;
}

#endif
