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


#include "pinyin.h"
#include <stdlib.h>
#include <string.h>

int main(int argc, char * argv[]){
    pinyin_context_t * context =
        pinyin_init("../data", "../data");

    pinyin_option_t options =
        PINYIN_CORRECT_ALL | USE_DIVIDED_TABLE | USE_RESPLIT_TABLE |
        DYNAMIC_ADJUST;
    pinyin_set_options(context, options);

    pinyin_instance_t * instance = pinyin_alloc_instance(context);
    CandidateVector candidates = g_array_new
        (FALSE, FALSE, sizeof(lookup_candidate_t));

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

        if ((read = getline(&linebuf, &linesize, stdin)) == -1)
            break;

        if ( '\n' == linebuf[strlen(linebuf) - 1] ) {
            linebuf[strlen(linebuf) - 1] = '\0';
        }

        if ( strcmp ( linebuf, "quit" ) == 0)
            break;

        pinyin_parse_more_full_pinyins(instance, linebuf);
        pinyin_guess_sentence_with_prefix(instance, prefixbuf);

        pinyin_get_full_pinyin_candidates(instance, 0, candidates);
        for (size_t i = 0; i < candidates->len; ++i) {
            lookup_candidate_t * candidate = &g_array_index
                (candidates, lookup_candidate_t, i);
            const char * pinyins = candidate->m_new_pinyins;
            const char * word = candidate->m_phrase_string;

            if (pinyins)
                printf("%s %s\t", pinyins, word);
            else
                printf("%s\t", word);
        }
        printf("\n");

        pinyin_train(instance);
        pinyin_reset(instance);
        pinyin_save(context);
    }

    g_array_free(candidates, TRUE);
    pinyin_free_instance(instance);
    pinyin_fini(context);
    free(prefixbuf); free(linebuf);
    return 0;
}
