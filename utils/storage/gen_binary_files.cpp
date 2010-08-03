#include <stdio.h>
#include "memory_chunk.h"
#include "novel_types.h"
#include "pinyin_base.h"
#include "pinyin_phrase.h"
#include "pinyin_large_table.h"
#include "phrase_index.h"

int main(int argc, char * argv[]){
    /* generate pinyin index*/
    PinyinCustomSettings custom;
    PinyinLargeTable largetable(&custom);

    FILE * gbfile = fopen("../../data/gb_char.table", "r");
    if ( gbfile == NULL) {
	printf("open gb_char.table failed!");
	return 1;
    }
    FILE * gbkfile = fopen("../../data/gbk_char.table","r");
    if ( gbkfile == NULL) {
	printf("open gb_char.table failed!");
	return 1;
    }
    
    largetable.load_text(gbfile);
    fclose(gbfile);
    largetable.load_text(gbkfile);
    fclose(gbkfile);

    MemoryChunk * new_chunk = new MemoryChunk;
    largetable.store(new_chunk);
    new_chunk->save("../../data/pinyin_index.bin");
    largetable.load(new_chunk);
    

    /* generate phrase index*/
    FacadePhraseIndex phrase_index;

    FILE* infile = fopen("../../data/gb_char.table", "r");
    if ( NULL == infile ){
	printf("open gb_char.table failed!\n");
	exit(1);
    }

    phrase_index.load_text(1, infile);
    fclose(infile);

    infile = fopen("../../data/gbk_char.table", "r");
    if ( NULL == infile ){
	printf("open gbk_char.table failed!\n");
	exit(1);
    }

    phrase_index.load_text(2, infile);
    fclose(infile);

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
