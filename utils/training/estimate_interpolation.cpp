/* 
 *  libpinyin
 *  Library to deal with pinyin.
 *  
 *  Copyright (C) 2006-2008 Peng Wu
 *  
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 * 
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *  
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>
#include <math.h>
#include <glib.h>
#include "pinyin_internal.h"

parameter_t compute_interpolation(SingleGram * deleted_bigram,
				  FacadePhraseIndex * unigram,
				  SingleGram * bigram){
    bool success;
    parameter_t lambda = 0, next_lambda = 0.6;
    parameter_t epsilon = 0.001;
    
    while ( fabs(lambda - next_lambda) > epsilon){
	lambda = next_lambda;
	next_lambda = 0;
	guint32 table_num = 0;
	parameter_t numerator = 0;
	parameter_t part_of_denominator = 0;
	
	BigramPhraseWithCountArray array = g_array_new(FALSE, FALSE, sizeof(BigramPhraseItemWithCount));
	deleted_bigram->retrieve_all(array);

	for ( int i = 0; i < array->len; ++i){
	    BigramPhraseItemWithCount * item = &g_array_index(array, BigramPhraseItemWithCount, i);
	    //get the phrase token
	    phrase_token_t token = item->m_token;
	    guint32 deleted_count = item->m_count;

	    {
		guint32 freq = 0;
		parameter_t elem_poss = 0;
		if (bigram && bigram->get_freq(token, freq)){
		    guint32 total_freq;
		    assert(bigram->get_total_freq(total_freq));
		    assert(0 != total_freq);
		    elem_poss = freq / (parameter_t) total_freq;
		}
		numerator = lambda * elem_poss;
	    }

	    {
		guint32 freq = 0;
		parameter_t elem_poss = 0;
		PhraseItem item;
		if (!unigram->get_phrase_item(token, item)){
		    guint32 freq = item.get_unigram_frequency();
		    guint32 total_freq = unigram->get_phrase_index_total_freq();
		    elem_poss = freq / (parameter_t)total_freq;
		}
		part_of_denominator = (1 - lambda) * elem_poss;
	    }
	    
	    if (0 == (numerator + part_of_denominator))
		continue;
	    
	    next_lambda += deleted_count * (numerator / (numerator + part_of_denominator));
	}
	assert(deleted_bigram->get_total_freq(table_num));
	next_lambda /= table_num;

	g_array_free(array, TRUE);
    }
    lambda = next_lambda;
    return lambda;
}
    
int main(int argc, char * argv[]){
    FacadePhraseIndex phrase_index;
    
    //gb_char binary file
    MemoryChunk * chunk = new MemoryChunk;
    chunk->load("gb_char.bin");
    phrase_index.load(1, chunk);
    
    //gbk_char binary file
    chunk = new MemoryChunk;
    chunk->load("gbk_char.bin");
    phrase_index.load(2, chunk);

    Bigram bigram;
    bigram.attach("bigram.db", ATTACH_READONLY);

    Bigram deleted_bigram;
    deleted_bigram.attach("deleted_bigram.db", ATTACH_READONLY);

    GArray * deleted_items = g_array_new(FALSE, FALSE, sizeof(phrase_token_t));
    deleted_bigram.get_all_items(deleted_items);

    parameter_t lambda_sum = 0;
    int lambda_count = 0;

    for ( int i = 0; i < deleted_items->len; ++i ){
	phrase_token_t * token = &g_array_index(deleted_items, phrase_token_t, i);
	SingleGram * single_gram = NULL;
	bigram.load(*token, single_gram);

	SingleGram * deleted_single_gram = NULL;
	deleted_bigram.load(*token, deleted_single_gram);
	
	parameter_t lambda = compute_interpolation(deleted_single_gram, &phrase_index, single_gram);
	
	printf("token:%d lambda:%f\n", *token, lambda);

	lambda_sum += lambda;
	lambda_count ++;

	if (single_gram)
            delete single_gram;
	delete deleted_single_gram;
    }

    printf("average lambda:%f\n", (lambda_sum/lambda_count));
    g_array_free(deleted_items, TRUE);
    return 0;
}

