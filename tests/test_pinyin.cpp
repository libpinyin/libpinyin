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
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */


#include <stdlib.h>
#include "pinyin.h"

int main(int argc, char * argv[]){
    pinyin_context_t * context =
        pinyin_init("../data", "../data");

    PinyinKeyVector pinyin_keys =
        g_array_new(FALSE, FALSE, sizeof(PinyinKey));

    char* linebuf = NULL;
    size_t size = 0;
    ssize_t read;
    while( (read = getline(&linebuf, &size, stdin)) != -1 ){
        if ( '\n' == linebuf[strlen(linebuf) - 1] ) {
            linebuf[strlen(linebuf) - 1] = '\0';
        }

	if ( strcmp ( linebuf, "quit" ) == 0)
            break;

        pinyin_parse_more_fulls(context, linebuf, pinyin_keys);
        pinyin_set_pinyin_keys(context, pinyin_keys);
        char * sentence = NULL;
        pinyin_get_guessed_sentence(context, &sentence);
        printf("%s\n", sentence);
        g_free(sentence);

        pinyin_train(context);
        pinyin_reset(context);
        pinyin_save(context);
    }

    pinyin_fini(context);
    g_array_free(pinyin_keys, TRUE);
    free(linebuf);
    return 0;
}
