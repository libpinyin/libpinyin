#include <stdio.h>
#include "pinyin_internal.h"


int main(int argc, char * argv[]){
    SingleGram single_gram;
    
    const guint32 total_freq = 16;
    assert(single_gram.set_total_freq(total_freq));

    phrase_token_t tokens[6] = { 2, 6, 4, 3, 1, 3};
    guint32 freqs[6] = { 1, 2, 4, 8, 16, 32};

    guint32 freq;

    for(size_t i = 0; i < 6 ;++i){
        if ( single_gram.get_freq(tokens[i], freq))
            assert(single_gram.set_freq(tokens[i], freqs[i]));
        else
            assert(single_gram.insert_freq(tokens[i], freqs[i]));
    }

    single_gram.get_freq(3, freq);
    assert(freq == 32);

    printf("--------------------------------------------------------\n");
    PhraseIndexRange range;
    BigramPhraseArray array = g_array_new(FALSE, FALSE, sizeof(BigramPhraseItem));
    range.m_range_begin = 0; range.m_range_end = 8;
    single_gram.search(&range,array);
    for ( size_t i = 0; i < array->len; ++i){
        BigramPhraseItem * item = &g_array_index(array, BigramPhraseItem, i);
        printf("item:%d:%f\n", item->m_token, item->m_freq);
    }

    assert(single_gram.get_total_freq(freq));
    assert(freq == total_freq);

    Bigram bigram;
    assert(bigram.attach("/tmp/test.db", ATTACH_CREATE|ATTACH_READWRITE));
    bigram.store(1, &single_gram);
    assert(single_gram.insert_freq(5, 8));
    assert(single_gram.remove_freq(1, freq));
    single_gram.set_total_freq(32);
    
    bigram.store(2, &single_gram);


    SingleGram * gram = NULL;
    for ( int m = 1; m <= 2; ++m ){
        printf("--------------------------------------------------------\n");
        bigram.load(m, gram);
        g_array_set_size(array, 0);
        range.m_range_begin = 0; range.m_range_end = 8;
        gram->search(&range,array);
        for ( size_t i = 0; i < array->len; ++i){
            BigramPhraseItem * item = &g_array_index(array, BigramPhraseItem, i);
            printf("item:%d:%f\n", item->m_token, item->m_freq);
        } 
        delete gram;
    }
    
    printf("--------------------------------------------------------\n");
    assert(single_gram.get_total_freq(freq));
    printf("total_freq:%d\n", freq);

    g_array_free(array, TRUE);

    GArray * items = g_array_new(FALSE, FALSE, sizeof(phrase_token_t));
    bigram.get_all_items(items);

    printf("----------------------system----------------------------\n");
    for ( size_t i = 0; i < items->len; ++i){
	phrase_token_t * token = &g_array_index(items, phrase_token_t, i);
	printf("item:%d\n", *token);
    }

    assert(bigram.save_db("/tmp/snapshot.db"));
    assert(bigram.load_db("/tmp/snapshot.db"));

    g_array_free(items, TRUE);

    /* mask out all index items. */
    bigram.mask_out(0x0, 0x0);

    return 0;
}
