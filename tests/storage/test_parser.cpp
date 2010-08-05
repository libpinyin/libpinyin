/*
 * libpinyin
 * 
 * Copyright (c) 2006 James Su <suzhe@tsinghua.org.cn>
 *
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place, Suite 330,
 * Boston, MA  02111-1307  USA
 *
 * $Id$
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "pinyin_base.h"

static const char *help_msg =
    "Too few argument!\n"
    "Usage:\n"
    "  test-parser [options]\n\n"
    "  -i            Use incomplete pinyin.\n"
    "  -f table      Use specified pinyin table file.\n"
    "  -p parser     Use specified parser instead of Default.\n"
    "                parser could be:\n"
    "                sp-stone\n"
    "                sp-zrm\n"
    "                sp-ms\n"
    "                sp-ziguang\n"
    "                sp-abc\n"
    "                sp-liushi\n"
    "                zy-zhuyin\n"
    "                zy-standard\n"
    "                zy-hsu\n"
    "                zy-ibm\n"
    "                zy-gin-yieh\n"
    "                zy-et\n"
    "                zy-et26\n";

int main (int argc, char * argv [])
{
    NullPinyinValidator validator;
    PinyinKeyVector keys;
    PinyinKeyPosVector poses;
    PinyinCustomSettings custom;
    PinyinParser *parser = 0;
    //PinyinTable table;
    const char *tablefile = "../data/pinyin-table.txt";

    keys = g_array_new(FALSE, FALSE, sizeof( PinyinKey));
    poses = g_array_new(FALSE, FALSE, sizeof( PinyinKeyPos));

    int i = 0;
    while (i<argc) {
        if (++i >= argc) break;

        if ( !strcmp("-h", argv [i]) || !strcmp ("--help", argv [i]) ) {
            printf(help_msg);
            return 0;
        }

        if ( !strcmp("-i", argv [i]) ) {
            custom.set_use_incomplete (true);
            continue;
        }

        if ( !strcmp("-p", argv [i]) ) {
            if (++i >= argc) {
                fprintf(stderr, "No argument for option %s.\n", argv [i-1]);
                return -1;
            }
            if (!strcmp (argv[i], "sp") || !strcmp (argv[i], "sp-default"))
                parser = new PinyinShuangPinParser ();
            else if (!strcmp (argv[i], "sp-stone"))
                parser = new PinyinShuangPinParser (SHUANG_PIN_STONE);
            else if (!strcmp (argv[i], "sp-zrm"))
                parser = new PinyinShuangPinParser (SHUANG_PIN_ZRM);
            else if (!strcmp (argv[i], "sp-ms"))
                parser = new PinyinShuangPinParser (SHUANG_PIN_MS);
            else if (!strcmp (argv[i], "sp-ziguang"))
                parser = new PinyinShuangPinParser (SHUANG_PIN_ZIGUANG);
            else if (!strcmp (argv[i], "sp-abc"))
                parser = new PinyinShuangPinParser (SHUANG_PIN_ABC);
            else if (!strcmp (argv[i], "sp-liushi"))
                parser = new PinyinShuangPinParser (SHUANG_PIN_LIUSHI);
            continue;
        }

        if (!strcmp("-f", argv [i])) {
            if (++i >= argc) {
                fprintf(stderr, "No argument for option %s.\n", argv [i-1]);
                return -1;
            }
            tablefile = argv [i];
            continue;
        }

        fprintf(stderr, "Invalid option: %s.\n", argv [i]);
        return -1;
    };

    if (!parser) parser = new PinyinDefaultParser ();

    char * line = NULL;
    size_t len = 0;

    while (1) {
        printf("Input:"); fflush(stdout);
        getline(&line, &len, stdin);

        if (!strncmp (line, "quit", 4)) break;

        int len = parser->parse (validator, keys, poses,(const char *) line);

        printf("Parsed %d chars, %d keys:\n", len, keys->len);

        for (size_t i=0; i < keys->len; ++i){
            PinyinKey * key = &g_array_index(keys, PinyinKey, i);
            printf("%s ", key->get_key_string ());
        }
        printf("\n");

        for ( size_t i=0; i < poses->len; ++i){
            PinyinKeyPos * pos = &g_array_index(poses, PinyinKeyPos, i);
            printf("%d %d ", pos->get_pos(), pos->get_length());
        }
        printf("\n");

        for (size_t i=0; i < keys->len; ++i){
            PinyinKey * key = &g_array_index(keys, PinyinKey, i);
            printf("%s ", key->get_key_zhuyin_string ());
        }
        printf("\n");
    }

    if (line)
        free(line);

    return 0;
}

/*
vi:ts=4:nowrap:ai:expandtab
*/
