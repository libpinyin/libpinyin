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
#include <glib.h>
#include "novel_types.h"
#include "phrase_large_table.h"

/* Note:
 * Currently libpinyin only supports ucs2 characters, as this is a
 * pre-processor tool for raw corpus, it will skip all sentences
 * which contains non-ucs2 characters.
 */

static PhraseLargeTable * g_phrases = NULL;

struct SegmentStep{
    phrase_token_t m_handle;
    utf16_t * m_phrase;
    size_t m_phrase_len;
    //use formula W = number of words. Zero handle means one word.
    size_t m_nword;
    //backtrace information, -1 one step backward.
    gint m_backword_nstep;
}

int main(int argc, char * argv[]){
    int i = 1;
    bool gen_extra_enter = false;

    setlocale(LC_ALL, "");
    //deal with options.
    while ( i < argc ){
        if ( strcmp ("--help", argv[i]) == 0) {
            print_help();
        }else if (strcmp("--generate-extra-enter", argv[i]) == 0) {
            gen_extra_enter = true;
        }
        ++i;
    }

    //init phrase table
    g_phrases =  new PhraseLargeTable;
    FILE * gb_file = fopen("../../data/gb_char.table", "r");
    if ( gb_file == NULL ){
	fprintf(stderr, "can't open gb_char.table!\n");
	exit(1);
    }
    g_phrases.load_text(gb_file);
    fclose(gb_file);

    FILE * gbk_file = fopen("../../data/gbk_char.table", "r");
    if ( gbk_file == NULL ){
	fprintf(stderr, "can't open gbk_char.table!\n");
	exit(1);
    }
    g_phrases.load_text(gbk_file);
    fclose(gbk_file);

    MemoryChunk * chunk = new MemoryChunk;
    g_phrases.store(chunk);
    g_phrases.load(chunk);

    return 0;
}
