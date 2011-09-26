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

    pinyin_instance_t * instance = pinyin_alloc_instance(context);

    char* linebuf = NULL;
    size_t size = 0;
    ssize_t read;
    while( (read = getline(&linebuf, &size, stdin)) != -1 ){
        if ( '\n' == linebuf[strlen(linebuf) - 1] ) {
            linebuf[strlen(linebuf) - 1] = '\0';
        }

        if ( strcmp ( linebuf, "quit" ) == 0) {
            break;
        }
        
        if (linebuf[0] == '\0') {
            continue;
        }

        pinyin_parse_more_full_pinyins(instance, linebuf);
        pinyin_guess_sentence(instance);

        char * sentence = NULL;
        pinyin_get_sentence (instance, &sentence);
        if (sentence)
            printf("%s\n", sentence);
        g_free(sentence);

        pinyin_train(instance);
        pinyin_reset(instance);
        pinyin_save(context);
    }

    pinyin_free_instance(instance);
    pinyin_fini(context);
    free(linebuf);
    return 0;
}
