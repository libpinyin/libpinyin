/* 
 *  libpinyin
 *  Library to deal with pinyin.
 *  
 *  Copyright (C) 2011 Peng Wu <alexepico@gmail.com>
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

#include "timer.h"
#include <string.h>
#include "pinyin_internal.h"
#include "tests_helper.h"

size_t bench_times = 1000;

int main(int argc, char * argv[]) {
    SystemTableInfo system_table_info;

    bool retval = system_table_info.load("../../data/table.conf");
    if (!retval) {
        fprintf(stderr, "load table.conf failed.\n");
        exit(ENOENT);
    }

    pinyin_option_t options = USE_TONE | PINYIN_INCOMPLETE;
    ChewingLargeTable largetable(options);
    FacadePhraseIndex phrase_index;

    const pinyin_table_info_t * phrase_files =
        system_table_info.get_table_info();

    if (!load_phrase_table(phrase_files, &largetable, NULL, &phrase_index))
        exit(ENOENT);

    MemoryChunk * new_chunk = new MemoryChunk;
    largetable.store(new_chunk);
    largetable.load(new_chunk);

    char* linebuf = NULL; size_t size = 0; ssize_t read;
    while ((read = getline(&linebuf, &size, stdin)) != -1) {
        if ( '\n' == linebuf[strlen(linebuf) - 1] ) {
            linebuf[strlen(linebuf) - 1] = '\0';
        }

	if ( strcmp ( linebuf, "quit" ) == 0)
	    break;

        FullPinyinParser2 parser;
        ChewingKeyVector keys = g_array_new(FALSE, FALSE, sizeof(ChewingKey));
        ChewingKeyRestVector key_rests =
            g_array_new(FALSE, FALSE, sizeof(ChewingKeyRest));

        parser.parse(options, keys, key_rests, linebuf, strlen(linebuf));
        if (0 == keys->len) {
            fprintf(stderr, "Invalid input.\n");
            continue;
        }

        guint32 start = record_time();
        PhraseIndexRanges ranges;
        memset(ranges, 0, sizeof(PhraseIndexRanges));

        phrase_index.prepare_ranges(ranges);

        for (size_t i = 0; i < bench_times; ++i) {
            phrase_index.clear_ranges(ranges);
            largetable.search(keys->len, (ChewingKey *)keys->data, ranges);
        }
        print_time(start, bench_times);

        phrase_index.clear_ranges(ranges);
        largetable.search(keys->len, (ChewingKey *)keys->data, ranges);

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
                for ( phrase_token_t token = onerange->m_range_begin;
                      token != onerange->m_range_end; ++token){

                    phrase_index.get_phrase_item( token, item);

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
            g_array_set_size(range, 0);
        }

        phrase_index.destroy_ranges(ranges);
	g_array_free(keys, TRUE);
	g_array_free(key_rests, TRUE);
    }

    if (linebuf)
        free(linebuf);

    /* mask out all index items. */
    largetable.mask_out(0x0, 0x0);

    return 0;
}
