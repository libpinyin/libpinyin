#include <stdio.h>
#include "memory_chunk.h"
#include "novel_types.h"
#include "ngram.h"


int main(int argc, char * argv[]){
    SingleGram single_gram;
    
    const guint32 total_freq = 16;
    assert(single_gram.set_total_freq(total_freq));
    

    phrase_token_t tokens[6] = { 2, 6, 4, 3, 1, 3};
    guint32 freqs[6] = { 1, 2, 4, 8, 16, 32};

    for(int i = 0; i < 6 ;++i){
	single_gram.set_freq(tokens[i], freqs[i]);
    }

    guint32 freq;
    single_gram.get_freq(3, freq);
    assert(freq == 32);

    printf("--------------------------------------------------------\n");
    PhraseIndexRange range;
    BigramPhraseArray array = g_array_new(FALSE, FALSE, sizeof(BigramPhraseItem));
    range.m_range_begin = 0; range.m_range_end = 8;
    single_gram.search(&range,array);
    for ( int i = 0; i < array->len; ++i){
	BigramPhraseItem * item = &g_array_index(array, BigramPhraseItem, i);
	printf("item:%d:%f\n", item->m_token, item->m_freq);
    } 


    assert(single_gram.get_total_freq(freq));
    assert(freq == total_freq);


    Bigram bigram;
    assert(bigram.attach(NULL, "/tmp/system.db"));
    bigram.store(1, &single_gram);
    single_gram.set_freq(5, 8);
    single_gram.set_total_freq(32);
    
    bigram.store(2, &single_gram);

    printf("--------------------------------------------------------\n");
    SingleGram * system, * user;
    bigram.load(1, system, user);
    assert(NULL == system);
    g_array_set_size(array, 0);
    range.m_range_begin = 0; range.m_range_end = 8;
    user->search(&range,array);
    for ( int i = 0; i < array->len; ++i){
	BigramPhraseItem * item = &g_array_index(array, BigramPhraseItem, i);
	printf("item:%d:%f\n", item->m_token, item->m_freq);
    } 
    delete user;

    printf("--------------------------------------------------------\n");
    bigram.load(2, system, user);
    assert(NULL == system);
    g_array_set_size(array, 0);
    range.m_range_begin = 0; range.m_range_end = 8;
    user->search(&range,array);
    for ( int i = 0; i < array->len; ++i){
	BigramPhraseItem * item = &g_array_index(array, BigramPhraseItem, i);
	printf("item:%d:%f\n", item->m_token, item->m_freq);
    } 
    delete user;
    
    bigram.attach("/tmp/system.db", NULL);
    printf("--------------------------------------------------------\n");
    bigram.load(1, system, user);
    assert(NULL == user);
    g_array_set_size(array, 0);
    range.m_range_begin = 0; range.m_range_end = 8;
    system->search(&range,array);
    for ( int i = 0; i < array->len; ++i){
	BigramPhraseItem * item = &g_array_index(array, BigramPhraseItem, i);
	printf("item:%d:%f\n", item->m_token, item->m_freq);
    } 
    delete system;
    
    printf("--------------------------------------------------------\n");
    bigram.load(2, system, user);
    assert(NULL == user);
    g_array_set_size(array, 0);
    range.m_range_begin = 0; range.m_range_end = 8;
    system->search(&range,array);
    for ( int i = 0; i < array->len; ++i){
	BigramPhraseItem * item = &g_array_index(array, BigramPhraseItem, i);
	printf("item:%d:%f\n", item->m_token, item->m_freq);
    }
    delete system;

    printf("--------------------------------------------------------\n");
    single_gram.prune();
    g_array_set_size(array, 0);
    range.m_range_begin = 0; range.m_range_end = 8;
    single_gram.search(&range,array);
    for ( int i = 0; i < array->len; ++i){
        BigramPhraseItem * item = &g_array_index(array, BigramPhraseItem, i);
        printf("item:%d:%f\n", item->m_token, item->m_freq);
    }
    assert(single_gram.get_total_freq(freq));
    printf("total_freq:%d\n", freq);

    g_array_free(array, TRUE);

    GArray * system_items = g_array_new(FALSE, FALSE, sizeof(phrase_token_t));
    GArray * user_items = g_array_new(FALSE, FALSE, sizeof(phrase_token_t));
    bigram.get_all_items(system_items, user_items);

    printf("----------------------system----------------------------\n");
    for ( int i = 0; i < system_items->len; ++i){
	phrase_token_t * token = &g_array_index(system_items, phrase_token_t, i);
	printf("item:%d\n", *token);
    }
    printf("-----------------------user-----------------------------\n");
    for ( int i = 0; i < user_items->len; ++i){
	phrase_token_t * token = &g_array_index(user_items, phrase_token_t, i);
	printf("item:%d\n", *token);
    }
}
