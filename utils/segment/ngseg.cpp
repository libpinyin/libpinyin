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
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

#include <stdio.h>
#include <stdlib.h>
#include <locale.h>
#include "pinyin.h"

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

PhraseLargeTable * g_phrase_table = NULL;
FacadePhraseIndex * g_phrase_index = NULL;
Bigram * g_bigram = NULL;
PhraseLookup * g_phrase_lookup = NULL;

void print_help(){
    printf("Usage: ngseg [--generate-extra-enter]\n");
    exit(1);
}

int main(int argc, char * argv[]){
    int i = 1;
    bool gen_extra_enter = false;

    setlocale(LC_ALL, "");
    //deal with options.
    while ( i < argc ){
        if ( strcmp ("--help", argv[i]) == 0 ){
            print_help();
        } else if ( strcmp("--generate-extra-enter", argv[i]) == 0 ){
            gen_extra_enter = true;
        }
        ++i;
    }

    //init phrase table
    g_phrase_table = new PhraseLargeTable;
    MemoryChunk * chunk = new MemoryChunk;
    chunk->load("../../data/phrase_index.bin");
    g_phrase_table->load(chunk);

    //init phrase index
    g_phrase_index = new FacadePhraseIndex;
    chunk = new MemoryChunk;
    chunk->load("../../data/gb_char.bin");
    g_phrase_index->load(1, chunk);
    chunk = new MemoryChunk;
    chunk->load("../../data/gbk_char.bin");
    g_phrase_index->load(2, chunk);

    //init bi-gram
    g_bigram = new Bigram;
    g_bigram->attach("../../data/bigram.db", NULL);

    //init phrase lookup
    g_phrase_lookup = new PhraseLookup(g_phrase_table, g_phrase_index,
                                       g_bigram);


    return 0;
}
