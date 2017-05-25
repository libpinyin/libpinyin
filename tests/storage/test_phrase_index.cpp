#include "timer.h"
#include <stdio.h>
#include <errno.h>
#include "pinyin_internal.h"
#include "tests_helper.h"

size_t bench_times = 100000;

int main(int argc, char * argv[]){
    PhraseItem phrase_item;
    ucs4_t string1 = 2;
    ChewingKey key1 = ChewingKey(CHEWING_CH, CHEWING_ZERO_MIDDLE, CHEWING_ENG);
    ChewingKey key2 = ChewingKey(CHEWING_SH, CHEWING_ZERO_MIDDLE, CHEWING_ANG);


    phrase_item.set_phrase_string(1, &string1);
    phrase_item.add_pronunciation(&key1, 100);
    phrase_item.add_pronunciation(&key2, 300);

    assert(phrase_item.get_phrase_length() == 1);

    ChewingKey key3;
    guint32 freq;
    phrase_item.get_nth_pronunciation(0, &key3, freq);
    assert(key3 == key1);
    assert(freq == 100);
    phrase_item.get_nth_pronunciation(1, &key3, freq);
    assert(key3 == key2);
    assert(freq == 300);

    gfloat poss = phrase_item.get_pronunciation_possibility(&key1);
    printf("pinyin possiblitiy:%f\n", poss);

    assert(phrase_item.get_unigram_frequency() == 0);

    ucs4_t string2;
    phrase_item.get_phrase_string(&string2);
    assert(string1 == string2);

    FacadePhraseIndex phrase_index_test;
    assert(!phrase_index_test.add_phrase_item(1, &phrase_item));

    MemoryChunk* chunk = new MemoryChunk;
    assert(phrase_index_test.store(0, chunk));
    assert(phrase_index_test.load(0, chunk));

    PhraseItem item2;
    guint32 time = record_time();
    for ( size_t i = 0; i < bench_times; ++i){
	phrase_index_test.get_phrase_item(1, item2);
	assert(item2.get_unigram_frequency() == 0);
	assert(item2.get_n_pronunciation() == 2);
	assert(item2.get_phrase_length() == 1);
	assert(item2.get_pronunciation_possibility(&key2) == 0.75);
    }
    print_time(time, bench_times);

    {
        PhraseItem item3;
        phrase_index_test.get_phrase_item(1, item3);
        item3.increase_pronunciation_possibility(&key1, 200);
        assert(item3.get_pronunciation_possibility(&key1) == 0.5) ;
    }

    {
        PhraseItem item5;
        phrase_index_test.get_phrase_item(1, item5);
        gfloat poss = item5.get_pronunciation_possibility(&key1);
        printf("pinyin poss:%f\n", poss);
        assert(poss == 0.5);
    }

    SystemTableInfo2 system_table_info;

    bool retval = system_table_info.load("../../data/table.conf");
    if (!retval) {
        fprintf(stderr, "load table.conf failed.\n");
        exit(ENOENT);
    }

    FacadePhraseIndex phrase_index;

    const pinyin_table_info_t * phrase_files =
        system_table_info.get_default_tables();

    TABLE_PHONETIC_TYPE type = system_table_info.get_table_phonetic_type();
    if (!load_phrase_table(phrase_files, NULL,
                           NULL, &phrase_index, type))
        exit(ENOENT);

    phrase_index.compact();

    MemoryChunk* store1 = new MemoryChunk;
    phrase_index.store(1, store1);
    phrase_index.load(1, store1);

    MemoryChunk* store2 = new MemoryChunk;
    phrase_index.store(2, store2);
    phrase_index.load(2, store2);

    phrase_index.compact();

    phrase_index.get_phrase_item(16870553, item2);
    assert( item2.get_phrase_length() == 14);
    assert( item2.get_n_pronunciation() == 1);

    ucs4_t buf[1024];
    item2.get_phrase_string(buf);
    char * string = g_ucs4_to_utf8( buf, 14, NULL, NULL, NULL);
    printf("%s\n", string);
    g_free(string);

    guint32 delta = 3;
    phrase_index.add_unigram_frequency(16870553, delta);
    phrase_index.get_phrase_item(16870553, item2);
    assert( item2.get_unigram_frequency() == 3);

    phrase_index.get_phrase_item(16777222, item2);
    assert(item2.get_phrase_length() == 1);
    assert(item2.get_n_pronunciation() == 2);

    return 0;
}
