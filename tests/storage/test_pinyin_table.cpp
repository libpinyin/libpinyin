#include "timer.h"
#include <string.h>
#include <errno.h>
#include "novel_types.h"
#include "pinyin_base.h"
#include "pinyin_large_table.h"

using namespace pinyin;

size_t bench_times = 1000;

int main( int argc, char * argv[]){

    PinyinCustomSettings custom;
    PinyinLargeTable largetable(&custom);

    FILE * gbfile = fopen("../../data/gb_char.table", "r");
    if ( gbfile == NULL ) {
	fprintf(stderr, "open gb_char.table failed!\n");
	exit(ENOENT);
    }

    largetable.load_text(gbfile);
    fclose(gbfile);

    FILE * gbkfile = fopen("../../data/gbk_char.table","r");
    if ( gbkfile == NULL ) {
	fprintf(stderr, "open gbk_char.table failed!\n");
	exit(ENOENT);
    }
    
    largetable.load_text(gbkfile);
    fclose(gbkfile);

    MemoryChunk* new_chunk = new MemoryChunk;
    largetable.store(new_chunk);
    largetable.load(new_chunk);
    
    char* linebuf = NULL;
    size_t size = 0;
    while( getline(&linebuf, &size, stdin) ){
        linebuf[strlen(linebuf)-1] = '\0';
	if ( strcmp ( linebuf, "quit" ) == 0)
	    break;
	
	PinyinDefaultParser parser;
	NullPinyinValidator validator;
	PinyinKeyVector keys;
	PinyinKeyPosVector poses;
	
	keys = g_array_new(FALSE, FALSE, sizeof( PinyinKey));
	poses = g_array_new(FALSE, FALSE, sizeof( PinyinKeyPos));
	parser.parse(validator, keys, poses, linebuf);
	
	guint32 start = record_time();

	PhraseIndexRanges ranges;
	for( size_t i = 0 ; i < PHRASE_INDEX_LIBRARY_COUNT ; ++i){
	    ranges[i] = g_array_new(FALSE, FALSE, sizeof (PhraseIndexRange));
	}
	for ( size_t i = 0 ; i < bench_times; ++i){
	    largetable.search(keys->len, (PinyinKey *)keys->data, ranges);
	}
       
	for( size_t i = 0 ; i < PHRASE_INDEX_LIBRARY_COUNT ; ++i){
	    GArray * range = ranges[i];
	    g_array_set_size( range, 0);
	}
	print_time(start, bench_times);

	largetable.search(keys->len, (PinyinKey *)keys->data, ranges);
	for( size_t i = 0 ; i < PHRASE_INDEX_LIBRARY_COUNT ; ++i){
	    GArray * range = ranges[i];
            if (range) {
                if (range->len)
                    printf("range items number:%d\n", range->len);

                for (size_t k = 0; k < range->len; ++k) {
                    PhraseIndexRange * onerange =
                        &g_array_index(range, PhraseIndexRange, k);
                    printf("start:%d\tend:%d\n", onerange->m_range_begin,
                           onerange->m_range_end);

                }
            }

            g_array_set_size(range, 0);
        }

	g_array_free(keys, TRUE);
	g_array_free(poses, TRUE);
    }
    if (linebuf)
        free(linebuf);
    return 0;
}
