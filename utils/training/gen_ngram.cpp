/* 
 *  libpinyin
 *  Library to deal with pinyin.
 *  
 *  Copyright (C) 2006-2007, 2011 Peng Wu
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
#include <string.h>
#include <locale.h>
#include <glib.h>
#include "pinyin_internal.h"

static PhraseLargeTable * g_phrases = NULL;

void print_help(){
    printf("Usage: gen_ngram [--skip-pi-gram-training]\n");
    printf("                 [--bigram-file <FILENAME>]\n");
}

int main(int argc, char * argv[]){
    int i = 1;
    bool train_pi_gram = true;
    const char * bigram_filename = "bigram.db";

    setlocale(LC_ALL, "");
    while ( i < argc ){
	if ( strcmp("--help", argv[i]) == 0){
	    print_help();
            exit(0);
	}else if ( strcmp("--skip-pi-gram-training", argv[i]) == 0 ){
	    train_pi_gram = false;
	}else if ( strcmp("--bigram-file", argv[i]) == 0){
            if ( ++i >= argc ) {
                print_help();
                exit(EINVAL);
            }
            bigram_filename = argv[i];
	}else{
            print_help();
            exit(EINVAL);
        }
	++i;
    }
    
    g_phrases = new PhraseLargeTable;
    //init phrase lookup
    MemoryChunk * chunk = new MemoryChunk;
    chunk->load("phrase_index.bin");
    g_phrases->load(chunk);

    FacadePhraseIndex phrase_index;
    
    //gb_char binary file
    chunk = new MemoryChunk;
    chunk->load("gb_char.bin");
    phrase_index.load(1, chunk);
    
    //gbk_char binary file
    chunk = new MemoryChunk;
    chunk->load("gbk_char.bin");
    phrase_index.load(2, chunk);
    
    Bigram bigram;
    bigram.attach(bigram_filename, ATTACH_CREATE|ATTACH_READWRITE);
    
    
    char* linebuf = NULL;
    size_t size = 0;
    phrase_token_t last_token, cur_token = last_token = 0;
    while( getline(&linebuf, &size, stdin) ){
	if ( feof(stdin) )
	    break;
        linebuf[strlen(linebuf)-1] = '\0';

        glong phrase_len = 0;
        utf16_t * phrase = g_utf8_to_utf16(linebuf, -1, NULL, &phrase_len, NULL);

	phrase_token_t token = 0;
        if ( 0 != phrase_len ) {
            int result = g_phrases->search( phrase_len, phrase, token);
            if ( ! (result & SEARCH_OK) )
                token = 0;
            g_free(phrase);
            phrase = NULL;
        }

	last_token = cur_token;
	cur_token = token;

        /* skip null_token in second word. */
        if ( null_token == cur_token )
            continue;

        //training uni-gram
        phrase_index.add_unigram_frequency(cur_token, 1);

        /* skip pi-gram training. */
        if ( null_token == last_token ){
            if ( !train_pi_gram )
                continue;
            last_token = sentence_start;
        }

        //train bi-gram
        SingleGram * single_gram = NULL;
        bigram.load(last_token, single_gram);

        if ( NULL == single_gram ){
            single_gram = new SingleGram;
        }
        guint32 freq, total_freq;
        //increase freq
        if (single_gram->get_freq(cur_token, freq))
            assert(single_gram->set_freq(cur_token, freq + 1));
        else
            assert(single_gram->insert_freq(cur_token, 1));
        //increase total freq
        single_gram->get_total_freq(total_freq);
        single_gram->set_total_freq(total_freq + 1);

        bigram.store(last_token, single_gram);
        delete single_gram;
    }

    free(linebuf);
    
    MemoryChunk * new_chunk = new MemoryChunk;
    phrase_index.store(1, new_chunk);
    new_chunk->save("gb_char.bin");
    phrase_index.load(1, new_chunk);

    new_chunk = new MemoryChunk;
    phrase_index.store(2, new_chunk);
    new_chunk->save("gbk_char.bin");
    phrase_index.load(2, new_chunk);

    return 0;
}
