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

#include <stdio.h>
#include <locale.h>
#include "pinyin_internal.h"
#include "tests_helper.h"


bool try_phrase_lookup(PhraseLookup * phrase_lookup,
                       ucs4_t * ucs4_str, glong ucs4_len){
    char * result_string = NULL;
    MatchResult result = g_array_new(FALSE, FALSE, sizeof(phrase_token_t));
    phrase_lookup->get_best_match(ucs4_len, ucs4_str, result);
#if 0
    for (size_t i = 0; i < result->len; ++i) {
        phrase_token_t * token = &g_array_index(result, phrase_token_t, i);
        if ( *token == null_token )
            continue;
        printf("%d:%d\t", i, *token);
    }
    printf("\n");
#endif
    phrase_lookup->convert_to_utf8(result, result_string);
    if (result_string)
        printf("%s\n", result_string);
    else
        fprintf(stderr, "Error: Un-segmentable sentence encountered!\n");
    g_array_free(result, TRUE);
    g_free(result_string);
    return true;
}

int main(int argc, char * argv[]){
    setlocale(LC_ALL, "");

    SystemTableInfo2 system_table_info;

    bool retval = system_table_info.load("../../data/table.conf");
    if (!retval) {
        fprintf(stderr, "load table.conf failed.\n");
        exit(ENOENT);
    }

    /* init phrase table */
    FacadePhraseTable3 phrase_table;
    phrase_table.load("../../data/phrase_index.bin", NULL);

    const pinyin_table_info_t * phrase_files =
        system_table_info.get_default_tables();

    /* init phrase index */
    FacadePhraseIndex phrase_index;
    if (!load_phrase_index(phrase_files, &phrase_index))
        exit(ENOENT);

    /* init bi-gram */
    Bigram system_bigram;
    system_bigram.attach("../../data/bigram.db", ATTACH_READONLY);
    Bigram user_bigram;

    gfloat lambda = system_table_info.get_lambda();

    /* init phrase lookup */
    PhraseLookup phrase_lookup(lambda,
                               &phrase_table, &phrase_index,
                               &system_bigram, &user_bigram);

    /* try one sentence */
    char * linebuf = NULL;
    size_t size = 0;
    ssize_t read;
    while( (read = getline(&linebuf, &size, stdin)) != -1 ){
        if ( '\n' == linebuf[strlen(linebuf) - 1] ) {
            linebuf[strlen(linebuf) - 1] = '\0';
        }

        if ( strcmp ( linebuf, "quit" ) == 0)
            break;

        /* check non-ucs4 characters */
        const glong num_of_chars = g_utf8_strlen(linebuf, -1);
        glong len = 0;
        ucs4_t * sentence = g_utf8_to_ucs4(linebuf, -1, NULL, &len, NULL);
        if ( len != num_of_chars ) {
            fprintf(stderr, "non-ucs4 characters are not accepted.\n");
            g_free(sentence);
            continue;
        }

        try_phrase_lookup(&phrase_lookup, sentence, len);
        g_free(sentence);
    }

    free(linebuf);
    return 0;
}
