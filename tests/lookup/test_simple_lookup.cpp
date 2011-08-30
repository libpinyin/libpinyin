#include <string.h>
#include <stdio.h>
#include <sys/time.h>
#include <glib.h>
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

    MemoryChunk * new_chunk = new MemoryChunk;
    new_chunk->load("../../data/pinyin_index.bin");
    largetable.load(new_chunk);

    BitmapPinyinValidator validator;
    validator.initialize(&largetable);
    
    FacadePhraseIndex phrase_index;
    new_chunk = new MemoryChunk;
    new_chunk->load("../../data/gb_char.bin");
    phrase_index.load(1, new_chunk);
    new_chunk = new MemoryChunk;
    new_chunk->load("../../data/gbk_char.bin");
    phrase_index.load(2, new_chunk);

    Bigram system_bigram;
    system_bigram.attach("../../data/bigram.db", ATTACH_READONLY);
    Bigram user_bigram;
    user_bigram.attach(NULL, ATTACH_CREATE|ATTACH_READWRITE);
    
    PinyinLookup pinyin_lookup(&custom, &largetable, &phrase_index,
                               &system_bigram, &user_bigram);
    
    char* linebuf = NULL;
    size_t size = 0;
    ssize_t read;
    while( (read = getline(&linebuf, &size, stdin)) != -1 ){
        if ( '\n' == linebuf[strlen(linebuf) - 1] ) {
            linebuf[strlen(linebuf) - 1] = '\0';
        }

	if ( strcmp ( linebuf, "quit" ) == 0)
	    break;
	
	PinyinDefaultParser parser;
	PinyinKeyVector keys;
	PinyinKeyPosVector poses;

	validator.initialize(&largetable);
	
	keys = g_array_new(FALSE, FALSE, sizeof( PinyinKey));
	poses = g_array_new(FALSE, FALSE, sizeof( PinyinKeyPos));
	parser.parse(validator, keys, poses,linebuf);

	if ( 0 == keys->len )
	    continue;
	CandidateConstraints constraints = g_array_new(FALSE, FALSE, sizeof(lookup_constraint_t));

	g_array_set_size(constraints, keys->len);
	for ( size_t i = 0; i < constraints->len; ++i){
	    lookup_constraint_t * constraint = &g_array_index(constraints, lookup_constraint_t, i);
	    constraint->m_type = NO_CONSTRAINT;
	}

	MatchResults results = g_array_new(FALSE, FALSE, sizeof(phrase_token_t));
	
	guint32 start_time = record_time();
	size_t times = 100;
	for ( size_t i = 0; i < times; ++i)
	    pinyin_lookup.get_best_match(keys, constraints, results);
	print_time(start_time, times);
	for ( size_t i = 0; i < results->len; ++i){
	    phrase_token_t * token = &g_array_index(results, phrase_token_t, i);
	    if ( null_token == *token)
		continue;
	    printf("pos:%ld,token:%d\t", i, *token);
	}
	printf("\n");
	char * sentence = NULL;
	pinyin_lookup.convert_to_utf8(results, sentence);
	printf("%s\n", sentence);
	g_array_free(results, TRUE);

	g_array_free(keys, TRUE);
	g_array_free(poses, TRUE);
	g_free(sentence);
    }
    free(linebuf);
}
