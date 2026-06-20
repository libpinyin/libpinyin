/* 
 *  libpinyin
 *  Library to deal with pinyin.
 *  
 *  Copyright (C) 2011 Peng Wu <alexepico@gmail.com>
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

#include "pinyin.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <string>

int main(int argc, char * argv[]){
    pinyin_context_t * context =
        pinyin_init("../data", "../data");

    pinyin_instance_t * instance = pinyin_alloc_instance(context);

    std::string linebuf;
    while (std::getline(std::cin, linebuf)) {
	if ( linebuf == "quit" )
            break;

        pinyin_phrase_segment(instance, linebuf.c_str());
        guint len = 0;
        pinyin_get_n_phrase(instance, &len);

        for ( size_t i = 0; i < len; ++i ){
            phrase_token_t token = null_token;
            pinyin_get_phrase_token(instance, i, &token);

            if ( null_token == token )
                continue;

            char * word = NULL;
            pinyin_token_get_phrase(instance, token, NULL, &word);
            printf("%s\t", word);
            g_free(word);
        }
        printf("\n");

        pinyin_save(context);
    }

    pinyin_free_instance(instance);

    pinyin_mask_out(context, 0x0, 0x0);
    pinyin_save(context);
    pinyin_fini(context);

    return 0;
}
