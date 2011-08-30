#include <string.h>
#include <stdio.h>
#include <sys/time.h>
#include "pinyin_internal.h"

size_t bench_times = 1000;

guint32 record_time ()
{
    timeval tv;
    gettimeofday (&tv, NULL);
    return (guint32) tv.tv_sec * 1000000 + tv.tv_usec;
}

void print_time (guint32 old_time, guint32 times)
{
    timeval tv;
    gettimeofday (&tv, NULL);

    guint32 wasted = (guint32) tv.tv_sec * 1000000 + tv.tv_usec - old_time;

    printf("Spent %d us for %d operations, %f us/op, %f times/s.\n\n" , wasted , times , ((double) wasted)/times , times * 1000000.0/wasted );
}


int main( int argc, char * argv[]){

    PinyinCustomSettings custom;
    PinyinLargeTable largetable(&custom);

    FacadePhraseIndex phrase_index;

    FILE * gbfile = fopen("../../data/gb_char.table", "r");
    if ( gbfile == NULL ) {
	fprintf(stderr, "open gb_char.table failed!\n");
	exit(ENOENT);
    }

    largetable.load_text(gbfile);
    fseek(gbfile, 0L, SEEK_SET);
    phrase_index.load_text(1, gbfile);
    fclose(gbfile);

    FILE * gbkfile = fopen("../../data/gbk_char.table","r");
    if ( gbkfile == NULL ) {
	fprintf(stderr, "open gb_char.table failed!\n");
	exit(ENOENT);
    }
    
    largetable.load_text(gbkfile);
    fseek(gbkfile, 0L, SEEK_SET);
    phrase_index.load_text(2, gbkfile);
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
	    if ( range ){
		for (size_t k = 0; k < range->len; ++k){
		    PhraseIndexRange* onerange = &g_array_index(range, PhraseIndexRange, k);
		    printf("start:%d\tend:%d\n", onerange->m_range_begin, onerange->m_range_end); 
		    PhraseItem item;
		    for ( phrase_token_t token = onerange->m_range_begin; token != onerange->m_range_end; ++token){
			phrase_index.get_phrase_item( token, item);
			gunichar2 bufstr[1024];
			item.get_phrase_string(bufstr);
			char * string = g_utf16_to_utf8
			    ( bufstr, item.get_phrase_length(), 
			      NULL, NULL, NULL);
			printf("%s\t", string);
			g_free(string);
			PinyinKey pinyin_buffer[1024];
			size_t npron = item.get_n_pronunciation();
			guint32 freq;
			for ( size_t n = 0; n < npron; ++n){
			    item.get_nth_pronunciation(n, pinyin_buffer, freq);
			    for ( size_t o = 0; o < item.get_phrase_length(); ++o){
				printf("%s'", pinyin_buffer[o].get_key_string());
			    }
			    printf("\b\t%d\t", freq);
			}
			printf("\n");
		    }
		}
		if ( range->len)
		    printf("range items number:%d\n", range->len);
	    }
	    g_array_set_size( range, 0);
	}

	g_array_free(keys, TRUE);
	g_array_free(poses, TRUE);
    }
    if (linebuf)
        free(linebuf);
    return 0;
}
