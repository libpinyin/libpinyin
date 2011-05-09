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
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */



#include <glib.h>
#include "pinyin.h"

typedef GHashTable * HashofWordPair;
typedef GHashTable * HashofSecondWord;

/* Hash token of Hash token of word count. */
HashofWordPair g_hash_of_document = NULL;
PhraseLargeTable * g_phrases = NULL;

void print_help(){
    printf("gen_k_mixture_model [--skip-pi-gram-training]\n");
    printf("                    [--skip-bi-gram-training]\n");
    printf("                    [--skip-k-mixture-model-training]\n");
    printf("                    [--maximum-ocurrs-allowed <INT>]\n");
    printf("                    [--maximum-increase-rates-allowed <FLOAT>]\n");
    printf("                    [--k-mixture-model-file <FILENAME>]\n");
    printf("                    {<FILENAME>}+");
}


bool convert_document_to_hash(FILE * document){
    char * linebuf = NULL;
    size_t size = 0;
    phrase_token_t last_token, cur_token = last_token = 0;

    while ( getline(&linebuf, &size, document) ){
        if ( feof(document) )
            break;
        /* Note: check '\n' here? */
        linebuf[strlen(linebuf) - 1] = "\0";

        glong phrase_len = 0;
        utf16_t * phrase = g_utf8_to_utf16(linebuf, -1, NULL, &phrase_len, NULL);

        if ( phrase_len == 0 )
            continue;

        phrase_token_t token = 0;
        int result = g_phrases->search( phrase_len, phrase, token );
        if ( ! (result & SEARCH_OK) )
            token = 0;

        last_token = cur_token;
        cur_token = token;

        /* remember the (last_token, cur_token) word pair. */
        HashofSecondWord hash_of_second_word = NULL;
        gboolean result = g_hash_table_lookup_extended
            (g_hash_of_document, GUINT_TO_POINTER(last_token),
             NULL, &hash_of_second_word);
        if ( !result ){
            hash_of_second_word = g_hash_table_new(g_int_hash, g_int_equal);
        }
        gpointer value = NULL;
        result = g_hash_table_lookup_extended
            (hash_of_second_word, GUINT_TO_POINTER(cur_token),
             NULL, &value);
        guint32 count = 0;
        if ( result ) {
            count = GPOINTER_TO_UINT(value);
        }
        count ++;
        g_hash_table_insert(hash_of_second_word,
                            GUINT_TO_POINTER(cur_token),
                            GUINT_TO_POINTER(count));
        g_hash_table_insert(g_hash_of_document,
                            GUINT_TO_POINTER(last_token),
                            hash_of_second_word);
                            
    }

    return true;
}

int main(int argc, char * argv[]){
    g_hash_of_document = g_hash_table_new(g_int_hash, g_int_equal, NULL, g_hash_table_unref);


    return 0;
}
