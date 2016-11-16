/* 
 *  libpinyin
 *  Library to deal with pinyin.
 *  
 *  Copyright (C) 2016 Peng Wu <alexepico@gmail.com>
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
#include <stdlib.h>
#include "pinyin_internal.h"
#include "tests_helper.h"

size_t bench_times = 1000;

using namespace pinyin;

bool test_pronunciation_possibility(PhoneticKeyMatrix * matrix,
                                    size_t start, size_t end,
                                    FacadePhraseIndex * phrase_index,
                                    PhraseIndexRanges ranges) {
    GArray * cached_keys = g_array_new(TRUE, TRUE, sizeof(ChewingKey));

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

                gfloat origin = compute_pronunciation_possibility
                    (matrix, start, end, cached_keys, item);

                bool increased = increase_pronunciation_possibility
                    (matrix, start, end, cached_keys, item, 30);

                gfloat updated = compute_pronunciation_possibility
                    (matrix, start, end, cached_keys, item);

                if (increased) {
                    printf("origin:%f\t updated:%f\n", origin, updated);
                } else {
                    assert(origin == updated);
                    printf("origin:%f\n", origin);
                }
            }
        }

    }

    g_array_free(cached_keys, TRUE);
    return true;
}

int main(int argc, char * argv[]) {
    SystemTableInfo2 system_table_info;

    bool retval = system_table_info.load("../../data/table.conf");
    if (!retval) {
        fprintf(stderr, "load table.conf failed.\n");
        exit(ENOENT);
    }

    pinyin_option_t options = PINYIN_CORRECT_ALL | PINYIN_AMB_ALL | PINYIN_INCOMPLETE;

    FacadeChewingTable2 largetable;
    largetable.load("../../data/pinyin_index.bin", NULL);

    const pinyin_table_info_t * phrase_files =
        system_table_info.get_default_tables();

    FacadePhraseIndex phrase_index;
    if (!load_phrase_index(phrase_files, &phrase_index))
        exit(ENOENT);

    PhoneticParser2 * parser = new FullPinyinParser2();
    ChewingKeyVector keys = g_array_new(FALSE, FALSE, sizeof(ChewingKey));
    ChewingKeyRestVector key_rests =
        g_array_new(FALSE, FALSE, sizeof(ChewingKeyRest));

    PhoneticKeyMatrix matrix;

    char* linebuf = NULL; size_t size = 0; ssize_t read;
    while( (read = getline(&linebuf, &size, stdin)) != -1 ){
        if ( '\n' == linebuf[strlen(linebuf) - 1] ) {
            linebuf[strlen(linebuf) - 1] = '\0';
        }

        if ( strcmp ( linebuf, "quit" ) == 0)
            break;

        int len = 0;
        guint32 start_time = record_time();
        for (size_t i = 0; i < bench_times; ++i) {
            matrix.clear_all();

            len = parser->parse(options, keys, key_rests,
                                linebuf, strlen(linebuf));

            fill_matrix(&matrix, keys, key_rests, len);

            resplit_step(options, &matrix);

            inner_split_step(options, &matrix);

            fuzzy_syllable_step(options, &matrix);
        }
        print_time(start_time, bench_times);

        printf("parsed %d chars, %d keys.\n", len, keys->len);

        dump_matrix(&matrix);

        PhraseIndexRanges ranges;
        memset(ranges, 0, sizeof(PhraseIndexRanges));

        phrase_index.prepare_ranges(ranges);

        for (size_t i = 0; i < matrix.size(); ++i) {
            for (size_t j = i + 1; j < matrix.size(); ++j) {
                phrase_index.clear_ranges(ranges);

                printf("search index: start %ld\t end %ld\n", i, j);
                int retval = search_matrix(&largetable, &matrix, i, j, ranges);

#if 0
                if (retval & SEARCH_OK) {
                    dump_ranges(&phrase_index, ranges);
                    test_pronunciation_possibility
                        (&matrix, i, j, &phrase_index, ranges);
                }
#endif

                if (!(retval & SEARCH_CONTINUED))
                    break;
            }
        }

        phrase_index.destroy_ranges(ranges);
    }

    if (linebuf)
        free(linebuf);

    delete parser;

    g_array_free(key_rests, TRUE);
    g_array_free(keys, TRUE);

    return 0;
}
