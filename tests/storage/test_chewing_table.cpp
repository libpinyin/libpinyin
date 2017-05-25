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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "timer.h"
#include <string.h>
#include "pinyin_internal.h"
#include "tests_helper.h"

size_t bench_times = 1000;

int main(int argc, char * argv[]) {
    SystemTableInfo2 system_table_info;

    bool retval = system_table_info.load("../../data/table.conf");
    if (!retval) {
        fprintf(stderr, "load table.conf failed.\n");
        exit(ENOENT);
    }

    pinyin_option_t options = USE_TONE | PINYIN_INCOMPLETE;
    ChewingLargeTable2 largetable;
    FacadePhraseIndex phrase_index;

    const pinyin_table_info_t * phrase_files =
        system_table_info.get_default_tables();

    TABLE_PHONETIC_TYPE type = system_table_info.get_table_phonetic_type();
    if (!load_phrase_table(phrase_files, &largetable,
                           NULL, &phrase_index, type))
        exit(ENOENT);

#if 0
    MemoryChunk * new_chunk = new MemoryChunk;
    largetable.store(new_chunk);
    largetable.load(new_chunk);
#endif

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
        size_t i = 0;
        PhraseIndexRanges ranges;
        memset(ranges, 0, sizeof(PhraseIndexRanges));

        phrase_index.prepare_ranges(ranges);

        for (i = 0; i < bench_times; ++i) {
            phrase_index.clear_ranges(ranges);
            largetable.search(keys->len, (ChewingKey *)keys->data, ranges);
        }
        print_time(start, bench_times);

        /* test search continued information. */
        int retval = SEARCH_NONE;
        for (i = 1; i < keys->len; ++i) {
            phrase_index.clear_ranges(ranges);
            retval = largetable.search(i, (ChewingKey *)keys->data, ranges);
            if (retval & SEARCH_CONTINUED)
                printf("return continued information with length:%ld\n", i);
        }

        phrase_index.clear_ranges(ranges);
        largetable.search(keys->len, (ChewingKey *)keys->data, ranges);

        dump_ranges(&phrase_index, ranges);

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
