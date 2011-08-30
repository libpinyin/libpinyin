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

    TokenVector tokens =
        g_array_new(FALSE, FALSE, sizeof(phrase_token_t));

    char* linebuf = NULL;
    size_t size = 0;
    ssize_t read;
    while( (read = getline(&linebuf, &size, stdin)) != -1 ){
        if ( '\n' == linebuf[strlen(linebuf) - 1] ) {
            linebuf[strlen(linebuf) - 1] = '\0';
        }

	if ( strcmp ( linebuf, "quit" ) == 0)
            break;

        pinyin_phrase_segment(context, linebuf, tokens);
        for ( size_t i = 0; i < tokens->len; ++i ){
            phrase_token_t token = g_array_index
                (tokens, phrase_token_t, i);

            if ( null_token == token )
                continue;

            char * word = NULL;
            pinyin_translate_token(context, token, &word);
            printf("%s\t", word);
            g_free(word);
        }
        printf("\n");

        pinyin_train(context);
        pinyin_reset(context);
        pinyin_save(context);
    }

    pinyin_fini(context);
    g_array_free(tokens, TRUE);
    free(linebuf);
    return 0;
}
