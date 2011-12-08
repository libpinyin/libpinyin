/* 
 *  libpinyin
 *  Library to deal with pinyin.
 * 
 *  Copyright (c) 2006 James Su <suzhe@tsinghua.org.cn> 
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


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "pinyin_base.h"

using namespace pinyin;

static const char *help_msg =
    "Usage:\n"
    "  test-parser [options]\n\n"
    "  -i            Use incomplete pinyin.\n"
    "  -f table      Use specified pinyin table file.\n"
    "  -p parser     Use specified parser instead of Default.\n"
    "                parser could be:\n"
#if 0
    "                sp-stone\n"
#endif
    "                sp-zrm\n"
    "                sp-ms\n"
    "                sp-ziguang\n"
    "                sp-abc\n"
#if 0
    "                sp-liushi\n"
#endif
    "                sp-pyjj\n"
    "                sp-xhe\n"
    "                zy-zhuyin\n"
    "                zy-standard\n"
    "                zy-hsu\n"
    "                zy-ibm\n"
    "                zy-gin-yieh\n"
    "                zy-et\n"
    "                zy-et26\n";

void print_help(){
    printf(help_msg);
}

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
            print_help ();
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
#if 0
            else if (!strcmp (argv[i], "sp-stone"))
                parser = new PinyinShuangPinParser (SHUANG_PIN_STONE);
#endif
            else if (!strcmp (argv[i], "sp-zrm"))
                parser = new PinyinShuangPinParser (SHUANG_PIN_ZRM);
            else if (!strcmp (argv[i], "sp-ms"))
                parser = new PinyinShuangPinParser (SHUANG_PIN_MS);
            else if (!strcmp (argv[i], "sp-ziguang"))
                parser = new PinyinShuangPinParser (SHUANG_PIN_ZIGUANG);
            else if (!strcmp (argv[i], "sp-abc"))
                parser = new PinyinShuangPinParser (SHUANG_PIN_ABC);
#if 0
            else if (!strcmp (argv[i], "sp-liushi"))
                parser = new PinyinShuangPinParser (SHUANG_PIN_LIUSHI);
#endif
            else if (!strcmp (argv[i], "sp-pyjj"))
                parser = new PinyinShuangPinParser (SHUANG_PIN_PYJJ);
            else if (!strcmp (argv[i], "sp-xhe"))
                parser = new PinyinShuangPinParser (SHUANG_PIN_XHE);
            else if (!strcmp (argv[i], "zy") || !strcmp (argv[i], "zy-standard") || !strcmp (argv[i], "zy-default"))
                parser = new PinyinZhuYinParser ();
            else if (!strcmp (argv[i], "zy-hsu"))
                parser = new PinyinZhuYinParser (ZHUYIN_HSU);
            else if (!strcmp (argv[i], "zy-ibm"))
                parser = new PinyinZhuYinParser (ZHUYIN_IBM);
            else if (!strcmp (argv[i], "zy-gin-yieh"))
                parser = new PinyinZhuYinParser (ZHUYIN_GIN_YIEH);
            else if (!strcmp (argv[i], "zy-et"))
                parser = new PinyinZhuYinParser (ZHUYIN_ET);
            else if (!strcmp (argv[i], "zy-et26"))
                parser = new PinyinZhuYinParser (ZHUYIN_ET26);
            else if (!strcmp (argv[i], "zy-zhuyin"))
                parser = new PinyinZhuYinParser (ZHUYIN_ZHUYIN);
            else {
                fprintf(stderr, "Unknown Parser:%s.\n", argv[i]);
                print_help();
                exit(EINVAL);
            }

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
            printf("%d %ld ", pos->get_pos(), pos->get_length());
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

