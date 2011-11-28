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


#include <stdio.h>

static const char * help_msg =
    "Usage:\n"
    "  test-parser -p <parser> [-s <scheme>] [options].\n\n"
    "  -p <parser> fullpinyin/doublepinyin/chewing.\n"
    "  -s <scheme> specify scheme for doublepinyin/chewing.\n"
    "     schemes for doublepinyin: zrm, ms, ziguang, abc, pyjj, xhe.\n"
    "     schemes for chewing: standard, ibm, ginyieh, eten.\n"
    "  -i          Use incomplete pinyin.\n"
    ;


void print_help(){
    printf(help_msg);
}

int main(int argc, char * argv[]) {
    return 0;
}
