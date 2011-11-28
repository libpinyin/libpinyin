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


#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "pinyin_parser2.h"


using namespace pinyin;

static const char * help_msg =
    "Usage:\n"
    "  test-parser -p <parser> [-s <scheme>] [options].\n\n"
    "  -p <parser> fullpinyin/doublepinyin/chewing.\n"
#if 0
    "  -s <scheme> specify scheme for doublepinyin/chewing.\n"
    "     schemes for doublepinyin: zrm, ms, ziguang, abc, pyjj, xhe.\n"
    "     schemes for chewing: standard, ibm, ginyieh, eten.\n"
#endif
    "  -i          Use incomplete pinyin.\n"
    "  -h          print this help msg.\n"
    ;


void print_help(){
    printf(help_msg);
}

int main(int argc, char * argv[]) {
    PinyinParser2 * parser = NULL;
    ChewingKeyVector keys = g_array_new(FALSE, FALSE, sizeof(ChewingKey));
    ChewingKeyRestVector key_rests =
        g_array_new(FALSE, FALSE, sizeof(ChewingKeyRest));
    guint32 options = PINYIN_CORRECT_ALL;

    int i = 1;
    while(i < argc) {
        if (strcmp("-h", argv[i]) == 0) {
            print_help();
            exit(0);
        } else if (strcmp("-i", argv[i]) == 0) {
            options |= PINYIN_INCOMPLETE | CHEWING_INCOMPLETE;
        } else if (strcmp("-p", argv[i]) == 0) {
            if ( ++i >= argc ) {
                print_help();
                exit(EINVAL);
            }
            const char * name = argv[i];
            if (strcmp("fullpinyin", name) == 0) {
                parser = new FullPinyinParser2();
            } else if (strcmp("doublepinyin", name) == 0) {
                parser = new DoublePinyinParser2();
            } else if (strcmp("chewing", name) == 0) {
                parser = new ChewingParser2();
            } else {
                print_help();
                exit(EINVAL);
            }
        } else {
            print_help();
            exit(EINVAL);
        }
        ++i;
    }

    if (NULL == parser) {
        print_help();
        exit(EINVAL);
    }



    return 0;
}
