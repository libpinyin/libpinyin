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


#include "novel_types.h"
#include "pinyin_base.h"
#include "pinyin_phrase.h"
#include <stdio.h>
#include <errno.h>
#include <locale.h>
#include <glib.h>

using namespace pinyin;


GTree * g_pinyin_tree;
GArray * g_item_array[MAX_PHRASE_LENGTH + 1];

struct phrase_item{
    size_t length;
    gunichar * uniphrase;
};

struct pinyin_and_freq_item{
    GArray * pinyin;
    guint32 freq;
};

struct item{
    phrase_item * phrase;
    GArray * pinyin_and_freq_array;       /* Array of pinyin_and_freq_item. */
};

void feed_file(const char * filename);

void feed_line(const char * phrase, const char * pinyin, const guint32 freq);

void store_in_item_array();

void sort_item_array();

void gen_phrase_file(const char * outfilename, int phrase_index);

void print_help(){
    printf("Usage: gen_pinyin_table -t <PHRASE_INDEX> "
	   "-o <OUTPUTFILE> <FILE1> <FILE2> .. <FILEn>\n");
    printf("<OUTPUTFILE> the result output file\n");
    printf("<FILEi> input pinyin files\n");
    printf("<PHRASE_INDEX> phrase index identifier\n");
}

gint phrase_item_compare(gconstpointer a, gconstpointer b){
    phrase_item * itema = (phrase_item *) a;
    phrase_item * itemb = (phrase_item *) b;
    if ( itema->length != itemb->length )
	return itema->length - itemb->length;
    else 
	return memcmp(itema->uniphrase, itemb->uniphrase, 
		      sizeof(gunichar) * itema->length);
}

int main(int argc, char * argv[]){  
    char * outfilename = "temp.out";
    int phrase_index = 0;
    int i = 1;

    g_pinyin_tree = g_tree_new(phrase_item_compare);

    setlocale(LC_ALL,"");
    while (  i < argc ){
	if ( strcmp("--help", argv[i] ) == 0) {
		print_help();
                exit(0);
	}else if ( strcmp("-t", argv[i] ) == 0){
	    if ( ++i >= argc ) {
		print_help();
                exit(EINVAL);
            }
	    phrase_index = atoi(argv[i]);
	}else if ( strcmp("-o", argv[i] ) == 0 ){
	    if ( ++i >= argc ) {
		print_help();
                exit(EINVAL);
            }
	    outfilename = g_strdup(argv[i]);
	} else {
	    feed_file(argv[i]);
	}
	++i;
    }
    
    printf("nnodes: %d\n", g_tree_nnodes(g_pinyin_tree));

    store_in_item_array();
    sort_item_array();
    gen_phrase_file(outfilename, phrase_index);

    return 0;
}


void feed_file ( const char * filename){
    char phrase[1024], pinyin[1024];
    guint32 n_freq;
    FILE * infile = fopen(filename, "r");
    if ( NULL == infile ){
        fprintf(stderr, "Can't open file %s.\n", filename);
        exit(ENOENT);
    }
    while ( !feof(infile)){
	fscanf(infile, "%s", phrase);
	fscanf(infile, "%s", pinyin);
	fscanf(infile, "%u", &n_freq);
	if (feof(infile))
		break;
	feed_line(phrase, pinyin, n_freq);
    }
    fclose(infile);
}

void feed_line (const char * phrase, const char * pinyin, const guint32 freq){
    phrase_item * new_phrase_ptr = (phrase_item *)
	malloc( sizeof(phrase_item));     
    new_phrase_ptr->length = g_utf8_strlen(phrase, -1);
	/* FIXME: modify ">" to ">=" according to pinyin_large_table.cpp
	 *	where is the code which I don't want to touch. :-)
	 */
	if (new_phrase_ptr->length >= MAX_PHRASE_LENGTH ) {
		fprintf(stderr, "too long phrase:%s\t%s\t%d\n", phrase,
			pinyin, freq);
		free(new_phrase_ptr);
		return;
	}
    new_phrase_ptr->uniphrase = g_utf8_to_ucs4(phrase, -1, NULL, NULL, NULL);
    
    PinyinDefaultParser parser;
    NullPinyinValidator validator;
    PinyinKeyVector keys;
    PinyinKeyPosVector poses;
    
    keys = g_array_new(FALSE, FALSE, sizeof( PinyinKey));
    poses = g_array_new(FALSE, FALSE, sizeof( PinyinKeyPos));
    parser.parse(validator, keys, poses, pinyin);

    GArray * array = (GArray *)g_tree_lookup(g_pinyin_tree, new_phrase_ptr);

    pinyin_and_freq_item value_item;
    value_item.pinyin = keys;
    value_item.freq = freq;
    
    if(new_phrase_ptr->length != value_item.pinyin->len){
	fprintf(stderr, "error:phrase:%s\tpinyin:%s\n", phrase, pinyin);
	return;
    }

    if ( array == NULL){
	array = g_array_new(FALSE, TRUE, sizeof(pinyin_and_freq_item));
	g_array_append_val(array, value_item);
	g_tree_insert(g_pinyin_tree, new_phrase_ptr, array);
	return;
    }
    bool found = false;
    for ( size_t i = 0; i < array->len ; ++i){
	pinyin_and_freq_item * old_value_item = &g_array_index(array, pinyin_and_freq_item, i);
	int result = pinyin_exact_compare((PinyinKey *)value_item.pinyin->data, 
					  (PinyinKey *)old_value_item->pinyin->data , value_item.pinyin->len);
	if ( result == 0 ){
	    printf("Duplicate item: phrase:%s\tpinyin:%s\tfreq:%u\n", 
		   phrase, pinyin, freq);
	    old_value_item->freq += freq;
	    found = true;
	}
    }

    g_array_free(poses, TRUE);
    
    if ( !found ){
	g_array_append_val(array, value_item);
	g_tree_insert(g_pinyin_tree, new_phrase_ptr, array);
    }else
	g_array_free(keys, TRUE);

    free(new_phrase_ptr);
    //g_array_free(keys, TRUE);
}

gboolean store_one_item (gpointer key, gpointer value, gpointer data){
    item oneitem;
    oneitem.phrase = (phrase_item *)key; 
    oneitem.pinyin_and_freq_array = (GArray *)value;
    int length = oneitem.phrase->length;
    g_array_append_val(g_item_array[length], oneitem);
    return FALSE;
}

void store_in_item_array(){
    for ( int i = 1; i < MAX_PHRASE_LENGTH + 1; ++i){
	g_item_array[i] = g_array_new(FALSE, TRUE, sizeof(item));
    }
    g_tree_foreach(g_pinyin_tree, store_one_item, NULL);
}

gint phrase_array_compare ( gconstpointer a, gconstpointer b, gpointer user_data){
    int phrase_length = *((int *) user_data);
    GArray * arraya = 
	g_array_index(((item *)a)->pinyin_and_freq_array, pinyin_and_freq_item, 0).pinyin;
    GArray * arrayb = 
	g_array_index(((item *)b)->pinyin_and_freq_array, pinyin_and_freq_item, 0).pinyin;
    return  pinyin_exact_compare((PinyinKey *)arraya->data, (PinyinKey*)arrayb->data, phrase_length);
}

void sort_item_array(){
    for ( int i = 1; i < MAX_PHRASE_LENGTH + 1; ++i){
	g_array_sort_with_data(g_item_array[i], phrase_array_compare , &i);
    }
}

void gen_phrase_file(const char * outfilename, int phrase_index){
    FILE * outfile = fopen(outfilename, "w");
    if (NULL == outfile ) {
        fprintf(stderr, "Can't write file %s.\n", outfilename);
        exit(ENOENT);
    }
    phrase_token_t token = 1;
    char pinyin_buffer[4096];
    //phrase length
    for ( size_t i = 1; i < MAX_PHRASE_LENGTH + 1; ++i){
	GArray * item_array = g_item_array[i];
	//item array
	for( size_t m = 0; m < item_array->len; ++m){
	    item* oneitem = & g_array_index(item_array, item, m);
	    phrase_item * phrase = oneitem->phrase;
	    GArray * pinyin_and_freqs = oneitem->pinyin_and_freq_array;
	    const char * phrase_buffer = g_ucs4_to_utf8(phrase->uniphrase,
						 phrase->length, 
						 NULL, NULL, NULL);
	    //each pinyin
	    for( size_t n = 0 ; n < pinyin_and_freqs->len; ++n){
		pinyin_and_freq_item * pinyin_and_freq = &g_array_index(pinyin_and_freqs, pinyin_and_freq_item, n);
		GArray * pinyin = pinyin_and_freq->pinyin;
		PinyinKey * key = &g_array_index(pinyin, PinyinKey, 0);
		strcpy(pinyin_buffer,key->get_key_string());
		for (size_t k = 1; k < pinyin->len; ++k){
		    strcat(pinyin_buffer, "'");
		    PinyinKey * key = &g_array_index(pinyin, PinyinKey, k);
		    strcat(pinyin_buffer, key->get_key_string ());
		}
		guint32 freq = pinyin_and_freq -> freq;
		if ( freq < 3 ) 
		    freq = 3;
		fprintf( outfile, "%s\t%s\t%d\t%d\n", 
			 pinyin_buffer, phrase_buffer, 
			 PHRASE_INDEX_MAKE_TOKEN(phrase_index, token),
			 freq);
	    }
	    token++;
	}
    }
    fclose(outfile);
}
