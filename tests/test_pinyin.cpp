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

int main(int argc, char * argv[]){
    pinyin_context_t * context =
        pinyin_init("../data", "../data");

    pinyin_option_t options = PINYIN_INCOMPLETE |
        PINYIN_CORRECT_ALL | USE_DIVIDED_TABLE | USE_RESPLIT_TABLE |
        DYNAMIC_ADJUST;
    pinyin_set_options(context, options);

    pinyin_instance_t * instance = pinyin_alloc_instance(context);

    char * prefixbuf = NULL; size_t prefixsize = 0;
    char * linebuf = NULL; size_t linesize = 0;
    ssize_t read;

    while( TRUE ){
        fprintf(stdout, "prefix:");
        fflush(stdout);

        if ((read = getline(&prefixbuf, &prefixsize, stdin)) == -1)
            break;

        if ( '\n' == prefixbuf[strlen(prefixbuf) - 1] ) {
            prefixbuf[strlen(prefixbuf) - 1] = '\0';
        }

        fprintf(stdout, "pinyin:");
        fflush(stdout);

        if ((read = getline(&linebuf, &linesize, stdin)) == -1)
            break;

        if ( '\n' == linebuf[strlen(linebuf) - 1] ) {
            linebuf[strlen(linebuf) - 1] = '\0';
        }

        if ( strcmp ( linebuf, "quit" ) == 0)
            break;

        size_t len = pinyin_parse_more_full_pinyins(instance, linebuf);
        pinyin_guess_sentence_with_prefix(instance, prefixbuf);
        pinyin_guess_candidates(instance, 0,
                                SORT_BY_PHRASE_LENGTH_AND_FREQUENCY);

        size_t i = 0;
        for (i = 0; i <= len; ++i) {
            gchar * aux_text = NULL;
            pinyin_get_full_pinyin_auxiliary_text(instance, i, &aux_text);
            printf("auxiliary text:%s\n", aux_text);
            g_free(aux_text);
        }

        guint num = 0;
        pinyin_get_n_candidate(instance, &num);
        for (i = 0; i < num; ++i) {
            lookup_candidate_t * candidate = NULL;
            pinyin_get_candidate(instance, i, &candidate);

            const char * word = NULL;
            pinyin_get_candidate_string(instance, candidate, &word);

            printf("%s\t", word);
        }
        printf("\n");

        pinyin_train(instance, 0);
        pinyin_reset(instance);
        pinyin_save(context);
    }

    pinyin_free_instance(instance);

    pinyin_mask_out(context, 0x0, 0x0);
    pinyin_save(context);
    pinyin_fini(context);

    free(prefixbuf); free(linebuf);
    return 0;
}
