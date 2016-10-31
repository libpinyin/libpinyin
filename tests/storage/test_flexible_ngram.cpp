/* 
 *  libpinyin
 *  Library to deal with pinyin.
 *  
 *  Copyright (C) 2012 Peng Wu <alexepico@gmail.com>
 *  
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */


#include "pinyin_internal.h"

int main(int argc, char * argv[]) {
    FlexibleSingleGram<guint32, guint32> single_gram;
    typedef FlexibleSingleGram<guint32, guint32>::ArrayItemWithToken array_item_t;

    const guint32 total_freq = 16;
    assert(single_gram.set_array_header(total_freq));

    phrase_token_t tokens[6] = { 2, 6, 4, 3, 1, 3 };
    guint32 freqs[6] = { 1, 2, 4, 8, 16, 32};

    guint32 freq;

    for ( size_t i = 0; i < G_N_ELEMENTS(tokens); ++i ){
        if ( single_gram.get_array_item(tokens[i], freq) )
            assert(single_gram.set_array_item(tokens[i], freqs[i]));
        else
            assert(single_gram.insert_array_item(tokens[i], freqs[i]));
    }

    single_gram.get_array_item(3, freq);
    assert(freq == 32);

    printf("--------------------------------------------------------\n");
    PhraseIndexRange range;
    FlexibleBigramPhraseArray array = g_array_new(FALSE, FALSE, sizeof(array_item_t));
    range.m_range_begin = 0; range.m_range_end = 8;
    single_gram.search(&range, array);
    for ( size_t i = 0; i < array->len; ++i ){
        array_item_t * item = &g_array_index(array, array_item_t, i);
        printf("item:%d:%d\n", item->m_token, item->m_item);
    }

    assert(single_gram.get_array_header(freq));
    assert(freq == total_freq);

    FlexibleBigram<guint32, guint32, guint32> bigram("TEST");
    assert(bigram.attach("/tmp/training.db", ATTACH_READWRITE|ATTACH_CREATE));
    bigram.store(1, &single_gram);
    assert(single_gram.insert_array_item(5, 8));
    assert(single_gram.remove_array_item(1, freq));
    assert(single_gram.set_array_header(32));
    assert(single_gram.get_array_header(freq));
    printf("new array header:%d\n", freq);
    bigram.store(2, &single_gram);

    for (int m = 1; m <= 2; ++m ){
        printf("--------------------------------------------------------\n");
        FlexibleSingleGram<guint32, guint32> * train_gram;
        bigram.load(m, train_gram);
        g_array_set_size(array, 0);
        range.m_range_begin = 0; range.m_range_end = 8;
        train_gram->search(&range, array);
        for ( size_t i = 0; i < array->len; ++i ){
            array_item_t * item = &g_array_index(array, array_item_t, i);
            printf("item:%d:%d\n", item->m_token, item->m_item);
        }
        delete train_gram;
    }

    GArray * items = g_array_new(FALSE, FALSE, sizeof(phrase_token_t));
    bigram.get_all_items(items);
    printf("-----------------------items----------------------------\n");
    for ( size_t i = 0; i < items->len; ++i ){
        phrase_token_t * token = &g_array_index(items, phrase_token_t, i);
        printf("item:%d\n", *token);
    }

    printf("-----------------------magic header---------------------\n");
    bigram.set_magic_header(total_freq);
    bigram.get_magic_header(freq);
    assert(total_freq == freq);
    printf("magic header:%d\n", freq);

    printf("-----------------------array header---------------------\n");
    for ( int i = 1; i <= 2; ++i){
        bigram.get_array_header(i, freq);
        printf("single gram: %d, freq:%d\n", i, freq);
    }

    bigram.set_array_header(1, 1);

    printf("-----------------------array header---------------------\n");
    for ( int i = 1; i <= 2; ++i){
        bigram.get_array_header(i, freq);
        printf("single gram: %d, freq:%d\n", i, freq);
    }

    for (int m = 1; m <= 2; ++m ){
        printf("--------------------------------------------------------\n");
        FlexibleSingleGram<guint32, guint32> * train_gram;
        bigram.load(m, train_gram);
        g_array_set_size(array, 0);
        range.m_range_begin = 0; range.m_range_end = 8;
        train_gram->search(&range, array);
        for ( size_t i = 0; i < array->len; ++i ){
            array_item_t * item = &g_array_index(array, array_item_t, i);
            printf("item:%d:%d\n", item->m_token, item->m_item);
        }
        delete train_gram;
    }

    assert(bigram.remove(1));

    bigram.get_all_items(items);
    printf("-----------------------items----------------------------\n");
    for ( size_t i = 0; i < items->len; ++i ){
        phrase_token_t * token = &g_array_index(items, phrase_token_t, i);
        printf("item:%d\n", *token);
    }

    g_array_free(items, TRUE);
    g_array_free(array, TRUE);
    return 0;
}
