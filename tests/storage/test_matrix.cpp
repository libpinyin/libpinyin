/* 
 *  libpinyin
 *  Library to deal with pinyin.
 *  
 *  Copyright (C) 2016 Peng Wu <alexepico@gmail.com>
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
#include <stdlib.h>
#include "pinyin_internal.h"

size_t bench_times = 1000;

using namespace pinyin;

int main(int argc, char * argv[]) {
    pinyin_option_t options = PINYIN_CORRECT_ALL;
    options |= PINYIN_AMB_ALL;
    options |= PINYIN_INCOMPLETE;

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

            fill_phonetic_key_matrix_from_chewing_keys
                (&matrix, keys, key_rests);

            fuzzy_syllable_step(options, &matrix);
        }
        print_time(start_time, bench_times);

        printf("parsed %d chars, %d keys.\n", len, keys->len);

        dump_phonetic_key_matrix(&matrix);
    }

    if (linebuf)
        free(linebuf);

    delete parser;

    g_array_free(key_rests, TRUE);
    g_array_free(keys, TRUE);

    return 0;
}
