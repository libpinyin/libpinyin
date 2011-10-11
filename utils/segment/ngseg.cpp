/* 
 *  libpinyin
 *  Library to deal with pinyin.
 *  
 *  Copyright (C) 2010 Peng Wu
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
#include <stdlib.h>
#include <locale.h>
#include "pinyin_internal.h"

/* n-gram based sentence segment. */

/* Note:
 * Currently libpinyin only supports ucs2 characters, as this is a
 * pre-processor tool for raw corpus, it will skip all sentences
 * which contains non-ucs2 characters.
 */

/* TODO:
 * Try to add punctuation mark and english support,
 * such as ',', '.', '?', '!', <english>, and other punctuations.
 */

PhraseLookup * g_phrase_lookup = NULL;

enum CONTEXT_STATE{
    CONTEXT_INIT,
    CONTEXT_SEGMENTABLE,
    CONTEXT_UNKNOWN
};

void print_help(){
    printf("Usage: ngseg [--generate-extra-enter]\n");
}

bool deal_with_segmentable(GArray * current_utf16){
    char * result_string = NULL;
    MatchResults results = g_array_new(FALSE, FALSE, sizeof(phrase_token_t));
    g_phrase_lookup->get_best_match(current_utf16->len, (utf16_t *) current_utf16->data, results);
#if 0
    for ( size_t i = 0; i < results->len; ++i) {
        phrase_token_t * token = &g_array_index(results, phrase_token_t, i);
        if ( *token == null_token )
            continue;
        printf("%d:%d\t", i, *token);
    }
    printf("\n");
#endif
    g_phrase_lookup->convert_to_utf8(results, "\n", result_string);
    if (result_string) {
        printf("%s\n", result_string);
    } else {
        char * tmp_string = g_utf16_to_utf8
            ( (utf16_t *) current_utf16->data, current_utf16->len,
              NULL, NULL, NULL);
        fprintf(stderr, "Un-segmentable sentence encountered:%s\n",
                tmp_string);
        g_array_free(results, TRUE);
        return false;
    }
    g_array_free(results, TRUE);
    g_free(result_string);
    return true;
}

bool deal_with_unknown(GArray * current_utf16){
    char * result_string = g_utf16_to_utf8
        ( (utf16_t *) current_utf16->data, current_utf16->len,
          NULL, NULL, NULL);
    printf("%s\n", result_string);
    g_free(result_string);
    return true;
}


int main(int argc, char * argv[]){
    int i = 1;
    bool gen_extra_enter = false;

    setlocale(LC_ALL, "");
    //deal with options.
    while ( i < argc ){
        if ( strcmp ("--help", argv[i]) == 0 ){
            print_help();
            exit(0);
        } else if ( strcmp("--generate-extra-enter", argv[i]) == 0 ){
            gen_extra_enter = true;
        } else {
            print_help();
            exit(EINVAL);
        }
        ++i;
    }

    //init phrase table
    PhraseLargeTable phrase_table;
    MemoryChunk * chunk = new MemoryChunk;
    chunk->load("phrase_index.bin");
    phrase_table.load(chunk);

    //init phrase index
    FacadePhraseIndex phrase_index;
    chunk = new MemoryChunk;
    chunk->load("gb_char.bin");
    phrase_index.load(1, chunk);
    chunk = new MemoryChunk;
    chunk->load("gbk_char.bin");
    phrase_index.load(2, chunk);

    //init bi-gram
    Bigram system_bigram;
    system_bigram.attach("bigram.db", ATTACH_READONLY);
    Bigram user_bigram;

    //init phrase lookup
    g_phrase_lookup = new PhraseLookup(&phrase_table, &phrase_index,
                                       &system_bigram, &user_bigram);


    CONTEXT_STATE state, next_state;
    GArray * current_utf16 = g_array_new(TRUE, TRUE, sizeof(utf16_t));
    phrase_token_t token = null_token;

    //split the sentence
    char * linebuf = NULL;
    size_t size = 0;
    ssize_t read;
    while( (read = getline(&linebuf, &size, stdin)) != -1 ){
        if ( '\n' ==  linebuf[strlen(linebuf) - 1] ) {
            linebuf[strlen(linebuf) - 1] = '\0';
        }

        //check non-ucs2 characters
        const glong num_of_chars = g_utf8_strlen(linebuf, -1);
        glong len = 0;
        utf16_t * sentence = g_utf8_to_utf16(linebuf, -1, NULL, &len, NULL);
        if ( len != num_of_chars ) {
            fprintf(stderr, "non-ucs2 characters encountered:%s.\n", linebuf);
            printf("\n");
            continue;
        }

        /* only new-line persists. */
        if ( 0  == num_of_chars ) {
            printf("\n");
            continue;
        }

        state = CONTEXT_INIT;
        bool result = phrase_table.search( 1, sentence, token);
        g_array_append_val( current_utf16, sentence[0]);
        if ( result & SEARCH_OK )
            state = CONTEXT_SEGMENTABLE;
        else
            state = CONTEXT_UNKNOWN;

        for ( int i = 1; i < num_of_chars; ++i) {
            bool result = phrase_table.search( 1, sentence + i, token);
            if ( result & SEARCH_OK )
                next_state = CONTEXT_SEGMENTABLE;
            else
                next_state = CONTEXT_UNKNOWN;

            if ( state == next_state ){
                g_array_append_val(current_utf16, sentence[i]);
                continue;
            }

            assert ( state != next_state );
            if ( state == CONTEXT_SEGMENTABLE )
                deal_with_segmentable(current_utf16);

            if ( state == CONTEXT_UNKNOWN )
                deal_with_unknown(current_utf16);

            /* save the current character */
            g_array_set_size(current_utf16, 0);
            g_array_append_val(current_utf16, sentence[i]);
            state = next_state;
        }

        if ( current_utf16->len ) {
            /* this seems always true. */
            if ( state == CONTEXT_SEGMENTABLE )
                deal_with_segmentable(current_utf16);

            if ( state == CONTEXT_UNKNOWN )
                deal_with_unknown(current_utf16);
            g_array_set_size(current_utf16, 0);
        }

        /* print extra enter */
        if ( gen_extra_enter )
            printf("\n");
    }

    delete g_phrase_lookup;
    g_phrase_lookup = NULL;
    /* print enter at file tail */
    printf("\n");
    g_array_free(current_utf16, TRUE);
    free(linebuf);
    return 0;
}
