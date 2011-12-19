#include "timer.h"
#include <string.h>
#include "pinyin_internal.h"

size_t bench_times = 100;

int main( int argc, char * argv[]){

    pinyin_option_t options = USE_TONE | PINYIN_CORRECT_ALL | PINYIN_AMB_ALL;
    ChewingLargeTable largetable(options);

    MemoryChunk * new_chunk = new MemoryChunk;
    new_chunk->load("../../data/pinyin_index.bin");
    largetable.load(new_chunk);

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
    
    PinyinLookup pinyin_lookup(options, &largetable, &phrase_index,
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
	
	FullPinyinParser2 parser;
	ChewingKeyVector keys  = g_array_new(FALSE, FALSE, sizeof(ChewingKey));
	ChewingKeyRestVector key_rests =
            g_array_new(FALSE, FALSE, sizeof(ChewingKeyRest));
	parser.parse(options, keys, key_rests, linebuf, strlen(linebuf));

	if ( 0 == keys->len ) /* invalid pinyin */
	    continue;

	CandidateConstraints constraints = g_array_new(FALSE, FALSE, sizeof(lookup_constraint_t));

	g_array_set_size(constraints, keys->len);
	for ( size_t i = 0; i < constraints->len; ++i){
	    lookup_constraint_t * constraint = &g_array_index(constraints, lookup_constraint_t, i);
	    constraint->m_type = NO_CONSTRAINT;
	}

	MatchResults results = g_array_new(FALSE, FALSE, sizeof(phrase_token_t));
	
	guint32 start_time = record_time();
	for ( size_t i = 0; i < bench_times; ++i)
	    pinyin_lookup.get_best_match(keys, constraints, results);
	print_time(start_time, bench_times);
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
	g_array_free(key_rests, TRUE);
	g_free(sentence);
    }
    free(linebuf);
}
