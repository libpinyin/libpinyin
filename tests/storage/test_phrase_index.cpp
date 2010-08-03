#include <stdio.h>
#include <sys/time.h>
#include <glib.h>
#include "memory_chunk.h"
#include "pinyin_base.h"
#include "phrase_index.h"

size_t bench_times = 100000;

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


int main(int argc, char * argv[]){
    PhraseItem phrase_item;
    utf16_t string1 = 2;
    PinyinKey key1 = PinyinKey((PinyinInitial)3,(PinyinFinal)3,(PinyinTone)3);
    PinyinKey key2 = PinyinKey((PinyinInitial)4,(PinyinFinal)4,(PinyinTone)4);


    phrase_item.set_phrase_string(1, &string1);
    phrase_item.append_pronunciation(&key1, 100);
    phrase_item.append_pronunciation(&key2, 300);

    assert(phrase_item.get_phrase_length() == 1);

    PinyinKey key3;
    guint32 freq;
    phrase_item.get_nth_pronunciation(0, &key3, freq);
    assert(key3 == key1);
    assert(freq == 100);
    phrase_item.get_nth_pronunciation(1, &key3, freq);
    assert(key3 == key2);
    assert(freq == 300);

    PinyinCustomSettings custom;
    gfloat poss = phrase_item.get_pinyin_possibility(custom, &key1);
    printf("pinyin possiblitiy:%f\n", poss);

    assert(phrase_item.get_unigram_frequency() == 0);

    utf16_t string2;
    phrase_item.get_phrase_string(&string2);
    assert(string1 == string2);

    FacadePhraseIndex phrase_index;
    assert(phrase_index.add_phrase_item(1, &phrase_item));

    MemoryChunk* chunk = new MemoryChunk;
    assert(phrase_index.store(0, chunk));
    assert(phrase_index.load(0, chunk));

    PhraseItem item2;
    guint32 time = record_time();
    for ( int i = 0; i < bench_times; ++i){
	phrase_index.get_phrase_item(1, item2);
	assert(item2.get_unigram_frequency() == 0);
	assert(item2.get_n_pronunciation() == 2);
	assert(item2.get_phrase_length() == 1);
	assert(item2.get_pinyin_possibility(custom, &key2) == 0.75);
    }
    print_time(time, bench_times);

    {
    PhraseItem item3;
    phrase_index.get_phrase_item(1, item3);
    item3.increase_pinyin_possibility(custom, &key1, 200);
    assert(item3.get_pinyin_possibility(custom, &key1) == 0.5) ;
    }

    {
    PhraseItem item5;
    phrase_index.get_phrase_item(1, item5);
    gfloat poss = item5.get_pinyin_possibility(custom, &key1);
    printf("pinyin poss:%f\n", poss);
    assert(poss == 0.5);
    }

    FacadePhraseIndex phrase_index_load;

    FILE* infile = fopen("../../data/gb_char.table", "r");
    if ( NULL == infile ){
	printf("open gb_char.table failed!\n");
	exit(1);
    }

    phrase_index_load.load_text(1, infile);
    fclose(infile);

    infile = fopen("../../data/gbk_char.table", "r");
    if ( NULL == infile ){
	printf("open gbk_char.table failed!\n");
	exit(1);
    }

    phrase_index_load.load_text(2, infile);
    fclose(infile);

    MemoryChunk* store1 = new MemoryChunk;
    phrase_index_load.store(1, store1);
    phrase_index_load.load(1, store1);

    MemoryChunk* store2 = new MemoryChunk;
    phrase_index_load.store(2, store2);
    phrase_index_load.load(2, store2);

    phrase_index_load.get_phrase_item(16870555, item2);
    assert( item2.get_phrase_length() == 14);
    assert( item2.get_n_pronunciation() == 1);

    gunichar2 buf[1024];
    item2.get_phrase_string(buf);
    char * string = g_utf16_to_utf8( buf, 14, NULL, NULL, NULL);
    printf("%s\n", string);
    g_free(string);

    guint32 delta = 3;
    phrase_index_load.add_unigram_frequency(16870555, delta);
    phrase_index_load.get_phrase_item(16870555, item2);
    assert( item2.get_unigram_frequency() == 3);

    phrase_index_load.get_phrase_item(16777222, item2);
    assert(item2.get_phrase_length() == 1);
    assert(item2.get_n_pronunciation() == 5);

    return 0;
}
