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
#include "pinyin.h"

int main(int argc, char * argv[]){
    /* generate pinyin index*/
    PinyinCustomSettings custom;
    PinyinLargeTable pinyinlargetable(&custom);
    PhraseLargeTable phraselargetable;

    FILE * gbfile = fopen("../../data/gb_char.table", "r");
    if ( gbfile == NULL) {
	printf("open gb_char.table failed!");
	return 1;
    }

    pinyinlargetable.load_text(gbfile);

    fseek(gbfile, 0L, SEEK_SET);
    phraselargetable.load_text(gbfile);
    fclose(gbfile);

    FILE * gbkfile = fopen("../../data/gbk_char.table","r");
    if ( gbkfile == NULL) {
	printf("open gb_char.table failed!");
	return 1;
    }
    
    pinyinlargetable.load_text(gbkfile);

    fseek(gbkfile, 0L, SEEK_SET);
    phraselargetable.load_text(gbkfile);
    fclose(gbkfile);

    MemoryChunk * new_chunk = new MemoryChunk;
    pinyinlargetable.store(new_chunk);
    new_chunk->save("../../data/pinyin_index.bin");
    pinyinlargetable.load(new_chunk);
    
    new_chunk = new MemoryChunk;
    phraselargetable.store(new_chunk);
    new_chunk->save("../../data/phrase_index.bin");
    phraselargetable.load(new_chunk);

    /* generate phrase index*/
    FacadePhraseIndex phrase_index;

    FILE* infile = fopen("../../data/gb_char.table", "r");
    if ( NULL == infile ){
	printf("open gb_char.table failed!\n");
	exit(ENOENT);
    }

    phrase_index.load_text(1, infile);
    fclose(infile);

    infile = fopen("../../data/gbk_char.table", "r");
    if ( NULL == infile ){
	printf("open gbk_char.table failed!\n");
	exit(ENOENT);
    }

    phrase_index.load_text(2, infile);
    fclose(infile);

    phrase_index.compat();

    new_chunk = new MemoryChunk;
    phrase_index.store(1, new_chunk);
    new_chunk->save("../../data/gb_char.bin");
    phrase_index.load(1, new_chunk);

    new_chunk = new MemoryChunk;
    phrase_index.store(2, new_chunk);
    new_chunk->save("../../data/gbk_char.bin");
    phrase_index.load(2, new_chunk);
    
    return 0;
}
