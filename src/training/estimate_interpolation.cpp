/* 
 *  novel-pinyin,
 *  A Simplified Chinese Sentence-Based Pinyin Input Method Engine
 *  Based On Markov Model.
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
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>
#include <math.h>
#include <glib.h>
#include "memory_chunk.h"
#include "novel_types.h"
#include "phrase_index.h"
#include "ngram.h"

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
	
	PhraseIndexRange range;
	range.m_range_begin = token_min;
	range.m_range_end = token_max;

	BigramPhraseArray array = g_array_new(FALSE, FALSE, sizeof(BigramPhraseItem));
	deleted_bigram->search(&range, array);

	for ( int i = 0; i < array->len; ++i){
	    BigramPhraseItem * item = &g_array_index(array, BigramPhraseItem, i);
	    //get the phrase token
	    phrase_token_t token = item->m_token;
	    guint32 deleted_freq = 0;
	    assert(deleted_bigram->get_freq(token, deleted_freq));
	    {
		guint32 freq = 0;
		parameter_t elem_poss = 0;
		if ( bigram && bigram->get_freq(token, freq)){
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
		if (unigram->get_phrase_item(token, item)){
		    guint32 freq = item.get_unigram_frequency();
		    guint32 total_freq = unigram->get_phrase_index_total_freq();
		    elem_poss = freq / (parameter_t)total_freq;
		}
		part_of_denominator = ( 1 - lambda) * elem_poss;
	    }
	    
	    if ( 0 == (numerator + part_of_denominator))
		continue;
	    
	    next_lambda += deleted_freq * (numerator / (numerator + part_of_denominator));
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
    chunk->load("../../data/gb_char.bin");
    phrase_index.load(1, chunk);
    
    //gbk_char binary file
    chunk = new MemoryChunk;
    chunk->load("../../data/gbk_char.bin");
    phrase_index.load(2, chunk);

    Bigram bigram;
    bigram.attach("../../data/bigram.db", NULL);

    Bigram deleted_bigram;
    deleted_bigram.attach("../../data/deleted_bigram.db", NULL);

    GArray * system_items = g_array_new(FALSE, FALSE, sizeof(phrase_token_t));
    GArray * user_items = g_array_new(FALSE, FALSE, sizeof(phrase_token_t));

    deleted_bigram.get_all_items(system_items, user_items);
    assert(0 == user_items->len);
    g_array_free(user_items, TRUE);

    parameter_t lambda_sum = 0;
    int lambda_count = 0;

    for ( int i = 0; i < system_items->len; ++i ){
	phrase_token_t * token = &g_array_index(system_items, phrase_token_t, i);
	SingleGram * system = NULL, * user = NULL;
	bigram.load(*token, system, user);
	assert(NULL == user);
	SingleGram * deleted_system = NULL, * deleted_user = NULL;
	deleted_bigram.load(*token, deleted_system, deleted_user);
	assert(NULL == deleted_user);
	
	parameter_t lambda = compute_interpolation(deleted_system, &phrase_index, system);
	
	printf("lambda:%f\n", lambda);

	lambda_sum += lambda;
	lambda_count ++;

	if (system) delete system;
	delete deleted_system;
    }

    printf("average lambda:%f\n", (lambda_sum/lambda_count));
    g_array_free(system_items, TRUE);
}

