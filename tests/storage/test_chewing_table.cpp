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


size_t bench_times = 1000;

int main(int argc, char * argv[]) {
    pinyin_option_t options = USE_TONE;
    ChewingLargeTable largetable(options);
    FacadePhraseIndex phrase_index;

    FILE * gbfile = fopen("../../data/gb_char.table", "r");
    if (NULL == gbfile) {
	fprintf(stderr, "open gb_char.table failed!\n");
	exit(ENOENT);
    }

    largetable.load_text(gbfile);
    fseek(gbfile, 0L, SEEK_SET);
    phrase_index.load_text(1, gbfile);
    fclose(gbfile);

    FILE * gbkfile = fopen("../../data/gbk_char.table", "r");
    if (NULL == gbkfile) {
	fprintf(stderr, "open gbk_char.table failed!\n");
	exit(ENOENT);
    }

    largetable.load_text(gbkfile);
    fseek(gbkfile, 0L, SEEK_SET);
    phrase_index.load_text(2, gbkfile);
    fclose(gbkfile);

    MemoryChunk * new_chunk = new MemoryChunk;
    largetable.store(new_chunk);
    largetable.load(new_chunk);

    char* linebuf = NULL; size_t size = 0;
    while( getline(&linebuf, &size, stdin) ){
        linebuf[strlen(linebuf)-1] = '\0';
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
        for (size_t i = 0; i < PHRASE_INDEX_LIBRARY_COUNT; ++i) {
            ranges[i] = g_array_new(FALSE, FALSE, sizeof(PhraseIndexRange));
        }

        for (size_t i = 0; i < bench_times; ++i) {
            largetable.search(keys->len, (ChewingKey *)keys->data, ranges);
        }

        for (size_t i = 0; i < PHRASE_INDEX_LIBRARY_COUNT; ++i) {
            g_array_set_size(ranges[i], 0);
        }
        print_time(start, bench_times);

        largetable.search(keys->len, (ChewingKey *)keys->data, ranges);

        for (size_t i = 0; i < PHRASE_INDEX_LIBRARY_COUNT; ++i) {
            GArray * & range = ranges[i];
            if (range) {
                for (size_t k = 0; k < range->len; ++k) {
                    PhraseIndexRange * onerange =
                        &g_array_index(range, PhraseIndexRange, k);
                    printf("start:%d\tend%d\n", onerange->m_range_begin,
                           onerange->m_range_end);
                }
                if (range->len)
                    printf("range items number:%d\n", range->len);
            }
            g_array_set_size(range, 0);
        }
	g_array_free(keys, TRUE);
	g_array_free(key_rests, TRUE);
    }

    if (linebuf)
        free(linebuf);
    return 0;
}
