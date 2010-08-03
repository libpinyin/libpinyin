/* 
 *  novel-pinyin,
 *  A Simplified Chinese Sentence-Based Pinyin Input Method Engine
 *  Based On Markov Model.
 *  
 *  Copyright (C) 2006-2007 Peng Wu
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>
#include <glib.h>
#include "memory_chunk.h"
#include "novel_types.h"
#include "phrase_index.h"
#include "ngram.h"


static GHashTable * g_phrases;

//read gb_char.table and gbk_char.table
bool init_phrases(FILE * infile){
    char pinyin[256];
    char phrase[256];
    phrase_token_t token;
    size_t freq;
    while (!feof(infile)){
        fscanf(infile, "%s", pinyin);
        fscanf(infile, "%s", phrase);
        fscanf(infile, "%d", &token);
        fscanf(infile, "%ld", &freq);
        if ( feof(infile) )
            break;
	g_hash_table_insert(g_phrases, g_strdup(phrase), 
			    GUINT_TO_POINTER(token));	
    }
	return true;
}

void print_help(){
    printf("gen_ngram [--skip-pi-gram-training] [--skip-unigram-training]\n");
    printf("          [--bigram-file <FILENAME>]\n");
    exit(1);
}

int main(int argc, char * argv[]){
	int i = 1;
    bool train_pi_gram = true;
    bool train_unigram = true;
    const char * bigram_filename = "../../data/bigram.db";

    setlocale(LC_ALL,"");
    while ( i < argc ){
	if ( strcmp("--help", argv[i] ) == 0){
	    print_help();
	}else if ( strcmp("--skip-pi-gram-training", argv[i] ) == 0) {
	    train_pi_gram = false;
	}else if ( strcmp("--skip-unigram-training", argv[i] ) == 0) {
	    train_unigram = false;
	}else if ( strcmp("--bigram-file", argv[i] ) == 0){
            if ( ++i >= argc )
                print_help();
            bigram_filename = argv[i];
	}
	++i;
    }
    
    g_phrases = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, NULL);
    //init phrase lookup
    FILE * gb_file = fopen("../../data/gb_char.table", "r");
    if ( gb_file == NULL ){
	fprintf(stderr, "can't open gb_char.table!\n");
	exit(1);
    }
    init_phrases(gb_file);
    fclose(gb_file);
    FILE * gbk_file = fopen("../../data/gbk_char.table", "r");
    if ( gbk_file == NULL ){
	fprintf(stderr, "can't open gbk_char.table!\n");
	exit(1);
    }
    init_phrases(gbk_file);
    fclose(gbk_file);

    FacadePhraseIndex phrase_index;
    
    //gb_char binary file
    MemoryChunk * chunk = new MemoryChunk;
    chunk->load("../../data/gb_char.bin");
    phrase_index.load(1, chunk);
    
    //gbk_char binary file
    chunk = new MemoryChunk;
    chunk->load("../../data/gbk_char.bin");
    phrase_index.load(2, chunk);
    
    Bigram bigram;
    bigram.attach(NULL, bigram_filename);
    
    
    char* linebuf = (char *)malloc ( 1024 * sizeof (char) );
    size_t size = 1024;
    phrase_token_t last_token, cur_token = last_token = 0;
    while( getline(&linebuf, &size, stdin) ){
	if ( feof(stdin) )
	    break;
        linebuf[strlen(linebuf)-1] = '\0';
	
	phrase_token_t token;
	gpointer orig_key, value;
	gboolean result = g_hash_table_lookup_extended
	    (g_phrases, linebuf, &orig_key, &value);
	if (result){
	    token = GPOINTER_TO_UINT(value);
	}else{
	    token = 0;
	}
	
	last_token = cur_token;
	cur_token = token;
	if ( cur_token ){
	    //training uni-gram
	    if ( train_unigram )
		phrase_index.add_unigram_frequency(cur_token, 1);
	}
	if ( cur_token ){
	    SingleGram * system = NULL, * user = NULL;
	    if ( 0 == last_token ){
		if (train_pi_gram)
		    bigram.load(sentence_start, system, user);
	    } else
		bigram.load(last_token, system, user);
	    assert(NULL == system);
	    if ( NULL == user ){
		user = new SingleGram;
	    }
	    guint32 freq, total_freq;
	    //increase freq
	    user->get_freq(cur_token, freq);
	    user->set_freq(cur_token, freq + 1);
	    //increase total freq
	    user->get_total_freq(total_freq);
	    user->set_total_freq(total_freq + 1);
	    if ( 0 == last_token ){
		if ( train_pi_gram )
		    bigram.store(sentence_start, user);
	    }else
		bigram.store(last_token, user);
	    delete user;
	}
    }
    
    MemoryChunk * new_chunk = new MemoryChunk;
    phrase_index.store(1, new_chunk);
    new_chunk->save("../../data/gb_char.bin");
    phrase_index.load(1, new_chunk);

    new_chunk = new MemoryChunk;
    phrase_index.store(2, new_chunk);
    new_chunk->save("../../data/gbk_char.bin");
    phrase_index.load(2, new_chunk);

    return 0;
}
