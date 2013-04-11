/* 
 *  libpinyin
 *  Library to deal with pinyin.
 *  
 *  Copyright (C) 2012 Peng Wu <alexepico@gmail.com>
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


#include "timer.h"
#include <string.h>
#include "pinyin_internal.h"
#include "tests_helper.h"

size_t bench_times = 100;

int main( int argc, char * argv[]){
    SystemTableInfo system_table_info;

    bool retval = system_table_info.load("../../data/table.conf");
    if (!retval) {
        fprintf(stderr, "load table.conf failed.\n");
        exit(ENOENT);
    }

    pinyin_option_t options =
        USE_TONE | USE_RESPLIT_TABLE | PINYIN_CORRECT_ALL | PINYIN_AMB_ALL;
    FacadeChewingTable largetable;

    MemoryChunk * chunk = new MemoryChunk;
    chunk->load("../../data/pinyin_index.bin");
    largetable.load(options, chunk, NULL);

    const pinyin_table_info_t * phrase_files =
        system_table_info.get_table_info();

    FacadePhraseIndex phrase_index;
    if (!load_phrase_index(phrase_files, &phrase_index))
        exit(ENOENT);

    Bigram system_bigram;
    system_bigram.attach("../../data/bigram.db", ATTACH_READONLY);
    Bigram user_bigram;
    user_bigram.attach(NULL, ATTACH_CREATE|ATTACH_READWRITE);

    gfloat lambda = system_table_info.get_lambda();
    
    PinyinLookup2 pinyin_lookup(lambda, options,
                                &largetable, &phrase_index,
                                &system_bigram, &user_bigram);

    /* prepare the prefixes for get_best_match. */
    TokenVector prefixes = g_array_new
        (FALSE, FALSE, sizeof(phrase_token_t));
    g_array_append_val(prefixes, sentence_start);
    
    CandidateConstraints constraints = g_array_new
        (TRUE, FALSE, sizeof(lookup_constraint_t));

    MatchResults results = g_array_new(FALSE, FALSE, sizeof(phrase_token_t));

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
	parser.parse(options, keys, key_rests, linebuf, strlen(linebuf));

	if ( 0 == keys->len ) /* invalid pinyin */
	    continue;

        /* initialize constraints. */
	g_array_set_size(constraints, keys->len);
	for ( size_t i = 0; i < constraints->len; ++i){
	    lookup_constraint_t * constraint = &g_array_index(constraints, lookup_constraint_t, i);
	    constraint->m_type = NO_CONSTRAINT;
	}

	guint32 start_time = record_time();
	for ( size_t i = 0; i < bench_times; ++i)
	    pinyin_lookup.get_best_match(prefixes, keys, constraints, results);
	print_time(start_time, bench_times);
	for ( size_t i = 0; i < results->len; ++i){
	    phrase_token_t * token = &g_array_index(results, phrase_token_t, i);
	    if ( null_token == *token)
		continue;
	    printf("pos:%ld,token:%d\t", i, *token);
	}
	printf("\n");
	char * sentence = NULL;
	pinyin_lookup.convert_to_utf8(results, sentence);
	printf("%s\n", sentence);

	g_array_free(keys, TRUE);
	g_array_free(key_rests, TRUE);
	g_free(sentence);
    }

    g_array_free(prefixes, TRUE);
    g_array_free(constraints, TRUE);
    g_array_free(results, TRUE);

    free(linebuf);
    return 0;
}
