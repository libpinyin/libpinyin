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
#include <assert.h>
#include <string.h>
#include <limits.h>
#include <locale.h>
#include <glib.h>
#include "novel_types.h"

static GHashTable * g_phrases;

struct SegmentStep{
    phrase_token_t  m_handle;
    char * m_phrase;
    //use formula W = No. of words. Zero handle means one word.
    size_t m_nword;
    //backtracing information, -1 one step backward.
    gint8 m_backward_nstep;
};

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

bool segment(GHashTable * phrases, // Lookup Phrases
	     const char * phrase,
	     GArray * strings /* Array of const char * */){
    GArray * steps = g_array_new(TRUE, TRUE, sizeof(SegmentStep));
    GArray * offsets = g_array_new(TRUE, TRUE, sizeof(size_t));
    //construct dynamic programming.
    size_t phrase_length = g_utf8_strlen(phrase, -1);
    const char * p = phrase;
    size_t offset = p - phrase;
    g_array_append_val(offsets, offset);
    g_array_set_size(steps, phrase_length + 1);
    for ( size_t i = 0 ; i < phrase_length; ++i){
	p = g_utf8_next_char(p);
	offset = p - phrase;
	g_array_append_val(offsets, offset);
    }
    assert( *p == '\0' );

    //initialize segment steps values.
    for ( size_t i = 0; i < phrase_length + 1; ++i){
	SegmentStep* step = &g_array_index(steps, SegmentStep, i);
	step->m_nword = UINT_MAX;
    }
    
    for ( size_t i = 0 ; i < phrase_length + 1; ++i){
	size_t* offset_begin = &g_array_index(offsets, size_t, i);
	const char * phrase_begin = phrase + *offset_begin;
	SegmentStep * step_begin = &g_array_index(steps, SegmentStep, i);
	size_t nword = step_begin->m_nword;
	for ( size_t k = i + 1; k < phrase_length + 1; ++k){
	    size_t* offset_end = &g_array_index(offsets, size_t, k);
	    size_t len = *offset_end - *offset_begin;
	    char * cur_phrase = g_strndup(phrase_begin, len);
	    phrase_token_t token; 
	    gpointer orig_key, value;
	    gboolean result = g_hash_table_lookup_extended
		(phrases, cur_phrase, &orig_key, &value);
	    if ( result ){
		token = GPOINTER_TO_UINT(value);
	    }else{
		token = 0;
		if ( 1 != k - i ){ //skip non-phrase
		    g_free(cur_phrase);
		    continue;
		}
	    }
	    ++nword;
	    SegmentStep * step_end = &g_array_index(steps, SegmentStep, k);
	    if ( nword < step_end->m_nword){
		if ( step_end->m_phrase ){
		    g_free(step_end->m_phrase);
		    step_end->m_phrase = NULL;
		}
		step_end->m_handle = token;
		step_end->m_phrase = cur_phrase;
		step_end->m_nword = nword;
		step_end->m_backward_nstep = k - i;
	    }else{
		g_free(cur_phrase);
	    }
	}
    }
    //backtracing to get the result.
    size_t cur_step = phrase_length;
    g_array_set_size(strings, 0);
    while ( cur_step ){
	SegmentStep* step_end = &g_array_index(steps, SegmentStep, cur_step);
	char * str_dup = g_strdup(step_end->m_phrase);
	g_array_append_val(strings, str_dup);
	cur_step = cur_step - step_end->m_backward_nstep;
    }
    
    for ( size_t i = 0; i < strings->len / 2; ++i){
	char ** phrase_head = &g_array_index(strings, char * , i);
	char ** phrase_tail = &g_array_index(strings, char * , strings->len -1 - i);
	char * phrase_tmp;
	phrase_tmp = * phrase_head; 
	* phrase_head = * phrase_tail; 
	* phrase_tail = phrase_tmp;
    }

    //free strndup memory
    for ( size_t i = 0; i < steps->len; ++i){
	SegmentStep* step = &g_array_index(steps, SegmentStep, i);
	if ( step->m_phrase ){
	    g_free(step->m_phrase);
	    step->m_phrase = NULL;
	}
    }

    g_array_free(offsets, TRUE);
    g_array_free(steps, TRUE);
	return true;
}

void print_help(){
    printf("Usage: mmseg [--generate-extra-enter]\n");
    exit(1);
}

int main(int argc, char * argv[]){
    int i = 1;
    bool gen_extra_enter = false;

    setlocale(LC_ALL,"");
    while ( i < argc ){
	if ( strcmp("--help", argv[i] ) == 0) {
	    print_help();
	}else if ( strcmp("--generate-extra-enter", argv[i]) == 0) {
	    gen_extra_enter = true;
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
    
    char* linebuf = (char *)malloc ( 1024 * sizeof (char) );
    size_t size = 1024;
    while( getline(&linebuf, &size, stdin) ){
	if ( feof(stdin) )
	    break;
        linebuf[strlen(linebuf)-1] = '\0';

	GArray * phrases = g_array_new(TRUE, TRUE, sizeof( char *));
	segment(g_phrases, linebuf, phrases);
	for ( size_t i = 0; i < phrases->len; ++i){
	    char * phrase = g_array_index(phrases, char *, i);
	    printf("%s\n", phrase);
	    g_free(phrase);
	}
	if ( gen_extra_enter )
	    printf("\n");
	g_array_free(phrases, TRUE);
    }
    free(linebuf);
}
