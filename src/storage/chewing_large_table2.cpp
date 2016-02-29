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

#include "chewing_large_table2.h"
#include "pinyin_phrase2.h"
#include "pinyin_phrase3.h"
#include "pinyin_parser2.h"


/* load text method */
bool ChewingLargeTable2::load_text(FILE * infile) {
    char pinyin[256];
    char phrase[256];
    phrase_token_t token;
    size_t freq;

    while (!feof(infile)) {
        int num = fscanf(infile, "%256s %256s %u %ld",
                         pinyin, phrase, &token, &freq);

        if (4 != num)
            continue;

        if(feof(infile))
            break;

        glong len = g_utf8_strlen(phrase, -1);

        PinyinDirectParser2 parser;
        ChewingKeyVector keys;
        ChewingKeyRestVector key_rests;

        keys = g_array_new(FALSE, FALSE, sizeof(ChewingKey));
        key_rests = g_array_new(FALSE, FALSE, sizeof(ChewingKeyRest));

        pinyin_option_t options = USE_TONE;
        parser.parse(options, keys, key_rests, pinyin, strlen(pinyin));

        if (len != keys->len) {
            fprintf(stderr, "ChewingLargeTable::load_text:%s\t%s\t%u\t%ld\n",
                    pinyin, phrase, token, freq);
            continue;
        }

        add_index(keys->len, (ChewingKey *)keys->data, token);

        g_array_free(keys, TRUE);
        g_array_free(key_rests, TRUE);
    }

    return true;
}
