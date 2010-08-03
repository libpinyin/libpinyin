/* 
 *  novel-pinyin,
 *  A Simplified Chinese Sentence-Based Pinyin Input Method Engine
 *  Based On Markov Model.
 *  
 *  Copyright (C) 2006-2007 Peng Wu
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
#include "memory_chunk.h"
#include "novel_types.h"
#include "ngram.h"

struct SingleGramItem{
    phrase_token_t m_token;
    guint32 m_freq;
};

SingleGram::SingleGram(){
    m_chunk.set_size(sizeof(guint32));
    memset(m_chunk.begin(), 0, sizeof(guint32));
}

SingleGram::SingleGram(void * buffer, size_t length){
    m_chunk.set_chunk(buffer, length, NULL);
}

bool SingleGram::set_total_freq(guint32 m_total){
    char * buf_begin = (char *)m_chunk.begin();
    *((guint32 *)buf_begin) = m_total;
    return true;
}

bool SingleGram::get_total_freq(guint32 & m_total){
    char * buf_begin = (char *)m_chunk.begin();
    m_total = *((guint32 *)buf_begin);
    return true;
}

bool SingleGram::prune(){
#if 1
    SingleGramItem * begin = (SingleGramItem *)
	((const char *)(m_chunk.begin()) + sizeof(guint32));
    SingleGramItem * end = (SingleGramItem *)m_chunk.end();
    
    size_t nitem = 0;
    for ( SingleGramItem * cur = begin; cur != end; ++cur){
	cur->m_freq--;
	nitem++;
	if ( cur->m_freq == 0 ){
	    size_t offset = sizeof(guint32) + (cur - begin)
		* sizeof(SingleGramItem) ;
	    m_chunk.remove_content(offset, sizeof(SingleGramItem));
	}
    }
    guint32 total_freq;
    assert(get_total_freq(total_freq));
    assert(set_total_freq(total_freq - nitem));
#endif
	return true;
}

bool token_less_than(const SingleGramItem & lhs,const SingleGramItem & rhs){
    return lhs.m_token < rhs.m_token;
}

bool SingleGram::search(/* in */ PhraseIndexRange * range, 
			/* out */ BigramPhraseArray array){
    const SingleGramItem * begin = (const SingleGramItem *)
	((const char *)(m_chunk.begin()) + sizeof(guint32));
    const SingleGramItem * end = (const SingleGramItem *)m_chunk.end();
    SingleGramItem compare_item;
    compare_item.m_token = range->m_range_begin;
    const SingleGramItem * cur_item = std_lite::lower_bound(begin, end, compare_item, token_less_than);

    guint32 total_freq;
    BigramPhraseItem bigram_item;
    assert(get_total_freq(total_freq));
    for ( ; cur_item != end; ++cur_item){
	if ( cur_item->m_token >= range->m_range_end )
	    break;
	bigram_item.m_token = cur_item->m_token;
	bigram_item.m_freq = cur_item->m_freq / (gfloat)total_freq;
	g_array_append_val(array, bigram_item);
    }
    return true;
}

bool SingleGram::get_freq(/* in */ phrase_token_t token,
			/* out */ guint32 & freq){
    freq = 0;
    const SingleGramItem * begin = (const SingleGramItem *)
	((const char *)(m_chunk.begin()) + sizeof(guint32));
    const SingleGramItem * end = (const SingleGramItem *)m_chunk.end();
    SingleGramItem compare_item;
    compare_item.m_token = token;
    const SingleGramItem * cur_item = std_lite::lower_bound(begin, end, compare_item, token_less_than);
    
    for ( ; cur_item != end; ++cur_item){
	if ( cur_item->m_token > token )
	    return false;
	if ( cur_item->m_token == token ){
	    freq = cur_item -> m_freq;
	    return true;
	}
    }
    return false;
}

bool SingleGram::set_freq(/* in */ phrase_token_t token,
			      guint32 freq){
    SingleGramItem * begin = (SingleGramItem *)
	((const char *)(m_chunk.begin()) + sizeof(guint32));
    SingleGramItem * end = (SingleGramItem *)m_chunk.end();
    SingleGramItem compare_item;
    compare_item.m_token = token;
    SingleGramItem * cur_item = std_lite::lower_bound(begin, end, compare_item, token_less_than);
    
    SingleGramItem insert_item;
    insert_item.m_token = token;
    insert_item.m_freq = freq;
    for ( ;cur_item != end; ++cur_item){
	if ( cur_item->m_token > token ){
	    size_t offset  = sizeof(guint32) + 
		sizeof(SingleGramItem) * (cur_item - begin);
	    m_chunk.insert_content(offset, &insert_item, 
				   sizeof(SingleGramItem));
	    return true;
	}
	if ( cur_item->m_token == token ){
	    cur_item -> m_freq = freq;
	    return true;
	}
    }
    m_chunk.insert_content(m_chunk.size(), &insert_item, 
			   sizeof(SingleGramItem));
    return true;
}


bool Bigram::attach(const char * systemfile, const char * userfile){
    reset();
    if ( systemfile ){
	int ret = db_create(&m_system, NULL, 0);
	if ( ret != 0 )
	    assert(false);
	
	m_system->open(m_system, NULL, systemfile, NULL, 
		       DB_HASH, DB_RDONLY, 0664);
	if ( ret != 0)
	    return false;
    }

    if ( userfile ){
	int ret = db_create(&m_user, NULL, 0);
	if ( ret != 0 )
	    assert(false);
	
	m_user->open(m_user, NULL, userfile, NULL, DB_HASH, DB_CREATE, 0664);
	if ( ret != 0)
	    return false;	
    }
    return true;
}

bool Bigram::load(phrase_token_t index, SingleGram * & system_gram, SingleGram * & user_gram){
    DBT db_key;
    memset(&db_key, 0, sizeof(DBT));
    db_key.data = &index;
    db_key.size = sizeof(phrase_token_t);
    
    system_gram = NULL; user_gram = NULL;
    if ( m_system ){
	DBT db_data;
	memset(&db_data, 0, sizeof(DBT));
	int ret = m_system->get(m_system, NULL, &db_key, &db_data, 0);
	if ( ret == 0 )
	    system_gram = new SingleGram(db_data.data, db_data.size);
    }
    if ( m_user ){
	DBT db_data;
	memset(&db_data, 0, sizeof(DBT));
	int ret = m_user->get(m_user, NULL, &db_key, &db_data, 0);
	if ( ret == 0 )
	    user_gram = new SingleGram(db_data.data, db_data.size);
    }
    return true;
}

bool Bigram::store(phrase_token_t index, SingleGram * user_gram){
    if ( !m_user )
	return false;
    DBT db_key;
    memset(&db_key, 0, sizeof(DBT));
    db_key.data = &index;
    db_key.size = sizeof(phrase_token_t);
    DBT db_data;
    memset(&db_data, 0, sizeof(DBT));
    db_data.data = user_gram->m_chunk.begin();
    db_data.size = user_gram->m_chunk.size();
    
    int ret = m_user->put(m_user, NULL, &db_key, &db_data, 0);
    return ret == 0;
}

bool Bigram::get_all_items(GArray * system, GArray * user){
    bool retval = false;
    g_array_set_size(system, 0);
    g_array_set_size(user, 0);
    if ( m_system ){
	DBC * cursorp;
	DBT key, data;
	int ret;
	/* Get a cursor */
	m_system->cursor(m_system, NULL, &cursorp, 0); 
	
	/* Initialize our DBTs. */
	memset(&key, 0, sizeof(DBT));
	memset(&data, 0, sizeof(DBT));
	
	/* Iterate over the database, retrieving each record in turn. */
	while ((ret = cursorp->c_get(cursorp, &key, &data, DB_NEXT)) == 0) {
	    assert(key.size == sizeof(phrase_token_t));
	    phrase_token_t * token = (phrase_token_t *)key.data;
	    g_array_append_val(system, *token);
	}
	
	if (ret != DB_NOTFOUND) {
	    fprintf(stderr, "system db error, exit!");
	    exit(1);
	}

	/* Cursors must be closed */
	if (cursorp != NULL) 
	    cursorp->c_close(cursorp); 

	retval = true;
    }
    if ( m_user ){
	DBC * cursorp;
	DBT key, data;
	int ret;
	/* Get a cursor */
	m_user->cursor(m_user, NULL, &cursorp, 0);

	/* Initialize out DBTs. */
	memset(&key, 0, sizeof(DBT));
	memset(&data, 0, sizeof(DBT));
	
	/* Iterate over the database, retrieving each record in turn. */
	while((ret = cursorp->c_get(cursorp, &key, &data, DB_NEXT)) == 0) {
	    assert(key.size == sizeof(phrase_token_t));
	    phrase_token_t * token = (phrase_token_t *) key.data;
	    g_array_append_val(user, *token);
	}
	
	if (ret != DB_NOTFOUND){
	    fprintf(stderr, "user db error, exit!");
	    exit(1);
	}
	
	/* Cursor must be closed */
	if ( cursorp != NULL)
	    cursorp->c_close(cursorp);

	retval = true;
    }
    return retval;
}
