/* 
 *  libpinyin
 *  Library to deal with pinyin.
 *  
 *  Copyright (C) 2012 Peng Wu <alexepico@gmail.com>
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

#include "timer.h"
#include <string.h>
#include "pinyin_internal.h"
#include "tests_helper.h"

size_t bench_times = 100;

int main( int argc, char * argv[]){
    SystemTableInfo2 system_table_info;

    bool retval = system_table_info.load("../../data/table.conf");
    if (!retval) {
        fprintf(stderr, "load table.conf failed.\n");
        exit(ENOENT);
    }

    pinyin_option_t options =
        USE_TONE | PINYIN_CORRECT_ALL | PINYIN_INCOMPLETE;
    FacadeChewingTable2 largetable;

    largetable.load("../../data/pinyin_index.bin", NULL);

    const pinyin_table_info_t * phrase_files =
        system_table_info.get_default_tables();

    FacadePhraseIndex phrase_index;
    if (!load_phrase_index(phrase_files, &phrase_index))
        exit(ENOENT);

    Bigram system_bigram;
    system_bigram.attach("../../data/bigram.db", ATTACH_READONLY);
    Bigram user_bigram;
    user_bigram.attach(NULL, ATTACH_CREATE|ATTACH_READWRITE);

    gfloat lambda = system_table_info.get_lambda();

    PhoneticLookup<2, 3> pinyin_lookup(lambda, &largetable, &phrase_index,
                                       &system_bigram, &user_bigram);

    /* prepare the prefixes for get_nbest_match. */
    TokenVector prefixes = g_array_new
        (FALSE, FALSE, sizeof(phrase_token_t));
    g_array_append_val(prefixes, sentence_start);

    ForwardPhoneticConstraints constraints(&phrase_index);
    NBestMatchResults results;

    char* linebuf = NULL; size_t size = 0; ssize_t read;
    while( (read = getline(&linebuf, &size, stdin)) != -1 ){
        if ( '\n' == linebuf[strlen(linebuf) - 1] ) {
            linebuf[strlen(linebuf) - 1] = '\0';
        }

        if ( strcmp ( linebuf, "quit" ) == 0)
            break;
	
        FullPinyinParser2 parser;
        ChewingKeyVector keys = g_array_new(FALSE, FALSE, sizeof(ChewingKey));
        ChewingKeyRestVector key_rests =
            g_array_new(FALSE, FALSE, sizeof(ChewingKeyRest));
        int parsed_len = parser.parse(options, keys, key_rests,
                                      linebuf, strlen(linebuf));

        PhoneticKeyMatrix matrix;

        if ( 0 == keys->len ) /* invalid pinyin */
            continue;

        /* fill the matrix. */
        fill_matrix(&matrix, keys, key_rests, parsed_len);

        resplit_step(options, &matrix);

        inner_split_step(options, &matrix);

        fuzzy_syllable_step(options, &matrix);

        dump_matrix(&matrix);

        g_array_free(keys, TRUE);
        g_array_free(key_rests, TRUE);

        /* initialize constraints. */
        constraints.validate_constraint(&matrix);

        guint32 start_time = record_time();
        for (size_t i = 0; i < bench_times; ++i)
            pinyin_lookup.get_nbest_match(prefixes, &matrix, &constraints, &results);
        print_time(start_time, bench_times);

        for (size_t i = 0; i < results.size(); ++i) {
            MatchResult result = NULL;
            assert(results.get_result(i, result));

            for (size_t j = 0; j < result->len; ++j){
                phrase_token_t * token = &g_array_index(result, phrase_token_t, j);
                if ( null_token == *token)
                    continue;
                printf("pos:%ld,token:%d\t", j, *token);
            }
            printf("\n");

            char * sentence = NULL;
            pinyin_lookup.convert_to_utf8(result, sentence);
            printf("%s\n", sentence);
            g_free(sentence);
        }
    }

    g_array_free(prefixes, TRUE);

    free(linebuf);
    return 0;
}
