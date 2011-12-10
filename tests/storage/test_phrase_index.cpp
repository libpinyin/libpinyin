#include "timer.h"
#include <stdio.h>
#include <errno.h>
#include "pinyin_internal.h"

size_t bench_times = 100000;

int main(int argc, char * argv[]){
    PhraseItem phrase_item;
    utf16_t string1 = 2;
    ChewingKey key1 = ChewingKey(CHEWING_CH, CHEWING_ZERO_MIDDLE, CHEWING_ENG);
    ChewingKey key2 = ChewingKey(CHEWING_SH, CHEWING_ZERO_MIDDLE, CHEWING_ANG);


    phrase_item.set_phrase_string(1, &string1);
    phrase_item.append_pronunciation(&key1, 100);
    phrase_item.append_pronunciation(&key2, 300);

    assert(phrase_item.get_phrase_length() == 1);

    ChewingKey key3;
    guint32 freq;
    phrase_item.get_nth_pronunciation(0, &key3, freq);
    assert(key3 == key1);
    assert(freq == 100);
    phrase_item.get_nth_pronunciation(1, &key3, freq);
    assert(key3 == key2);
    assert(freq == 300);

    pinyin_option_t options = 0;
    gfloat poss = phrase_item.get_pronunciation_possibility(options, &key1);
    printf("pinyin possiblitiy:%f\n", poss);

    assert(phrase_item.get_unigram_frequency() == 0);

    utf16_t string2;
    phrase_item.get_phrase_string(&string2);
    assert(string1 == string2);

    FacadePhraseIndex phrase_index;
    assert(!phrase_index.add_phrase_item(1, &phrase_item));

    MemoryChunk* chunk = new MemoryChunk;
    assert(phrase_index.store(0, chunk));
    assert(phrase_index.load(0, chunk));

    PhraseItem item2;
    guint32 time = record_time();
    for ( size_t i = 0; i < bench_times; ++i){
	phrase_index.get_phrase_item(1, item2);
	assert(item2.get_unigram_frequency() == 0);
	assert(item2.get_n_pronunciation() == 2);
	assert(item2.get_phrase_length() == 1);
	assert(item2.get_pronunciation_possibility(options, &key2) == 0.75);
    }
    print_time(time, bench_times);

    {
        PhraseItem item3;
        phrase_index.get_phrase_item(1, item3);
        item3.increase_pronunciation_possibility(options, &key1, 200);
        assert(item3.get_pronunciation_possibility(options, &key1) == 0.5) ;
    }

    {
        PhraseItem item5;
        phrase_index.get_phrase_item(1, item5);
        gfloat poss = item5.get_pronunciation_possibility(options, &key1);
        printf("pinyin poss:%f\n", poss);
        assert(poss == 0.5);
    }

    FacadePhraseIndex phrase_index_load;

    FILE* infile = fopen("../../data/gb_char.table", "r");
    if ( NULL == infile ){
	fprintf(stderr, "open gb_char.table failed!\n");
	exit(ENOENT);
    }

    phrase_index_load.load_text(1, infile);
    fclose(infile);

    infile = fopen("../../data/gbk_char.table", "r");
    if ( NULL == infile ){
	fprintf(stderr, "open gbk_char.table failed!\n");
	exit(ENOENT);
    }

    phrase_index_load.load_text(2, infile);
    fclose(infile);

    phrase_index.compat();

    MemoryChunk* store1 = new MemoryChunk;
    phrase_index_load.store(1, store1);
    phrase_index_load.load(1, store1);

    MemoryChunk* store2 = new MemoryChunk;
    phrase_index_load.store(2, store2);
    phrase_index_load.load(2, store2);

    phrase_index.compat();

    phrase_index_load.get_phrase_item(16870553, item2);
    assert( item2.get_phrase_length() == 14);
    assert( item2.get_n_pronunciation() == 1);

    gunichar2 buf[1024];
    item2.get_phrase_string(buf);
    char * string = g_utf16_to_utf8( buf, 14, NULL, NULL, NULL);
    printf("%s\n", string);
    g_free(string);

    guint32 delta = 3;
    phrase_index_load.add_unigram_frequency(16870553, delta);
    phrase_index_load.get_phrase_item(16870553, item2);
    assert( item2.get_unigram_frequency() == 3);

    phrase_index_load.get_phrase_item(16777222, item2);
    assert(item2.get_phrase_length() == 1);
    assert(item2.get_n_pronunciation() == 6);

    return 0;
}
