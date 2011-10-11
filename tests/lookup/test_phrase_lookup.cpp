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
#include <locale.h>
#include "pinyin_internal.h"

void print_help(){
    printf("Usage: test_phrase_lookup\n");
}

bool try_phrase_lookup(PhraseLookup * phrase_lookup,
                       utf16_t * utf16, glong utf16_len){
    char * result_string = NULL;
    MatchResults results = g_array_new(FALSE, FALSE, sizeof(phrase_token_t));
    phrase_lookup->get_best_match(utf16_len, utf16, results);
#if 0
    for ( size_t i = 0; i < results->len; ++i) {
        phrase_token_t * token = &g_array_index(results, phrase_token_t, i);
        if ( *token == null_token )
            continue;
        printf("%d:%d\t", i, *token);
    }
    printf("\n");
#endif
    phrase_lookup->convert_to_utf8(results, "\n", result_string);
    if (result_string)
        printf("%s\n", result_string);
    else
        fprintf(stderr, "Error: Un-segmentable sentence encountered!\n");
    g_array_free(results, TRUE);
    g_free(result_string);
    return true;
}

int main(int argc, char * argv[]){
    int i = 1;

    setlocale(LC_ALL, "");
    //deal with options.
    while ( i < argc ){
        if ( strcmp ("--help", argv[i]) == 0 ){
            print_help();
            exit(0);
        } else {
            print_help();
            exit(EINVAL);
        }
        ++i;
    }


    //init phrase table
    PhraseLargeTable phrase_table;
    MemoryChunk * chunk = new MemoryChunk;
    chunk->load("../../data/phrase_index.bin");
    phrase_table.load(chunk);

    //init phrase index
    FacadePhraseIndex phrase_index;
    chunk = new MemoryChunk;
    chunk->load("../../data/gb_char.bin");
    phrase_index.load(1, chunk);
    chunk = new MemoryChunk;
    chunk->load("../../data/gbk_char.bin");
    phrase_index.load(2, chunk);

    //init bi-gram
    Bigram system_bigram;
    system_bigram.attach("../../data/bigram.db", ATTACH_READONLY);
    Bigram user_bigram;

    //init phrase lookup
    PhraseLookup phrase_lookup(&phrase_table, &phrase_index,
                               &system_bigram, &user_bigram);

    //try one sentence
    char * linebuf = NULL;
    size_t size = 0;
    ssize_t read;
    while( (read = getline(&linebuf, &size, stdin)) != -1 ){
        if ( '\n' == linebuf[strlen(linebuf) - 1] ) {
            linebuf[strlen(linebuf) - 1] = '\0';
        }

        if ( strcmp ( linebuf, "quit" ) == 0)
            break;

        //check non-ucs2 characters
        const glong num_of_chars = g_utf8_strlen(linebuf, -1);
        glong len = 0;
        utf16_t * sentence = g_utf8_to_utf16(linebuf, -1, NULL, &len, NULL);
        if ( len != num_of_chars ) {
            fprintf(stderr, "non-ucs2 characters are not accepted.\n");
            g_free(sentence);
            continue;
        }

        try_phrase_lookup(&phrase_lookup, sentence, len);
        g_free(sentence);
    }

    free(linebuf);
    return 0;
}
