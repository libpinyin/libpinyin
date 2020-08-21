/* 
 *  libpinyin
 *  Library to deal with pinyin.
 *  
 *  Copyright (C) 2006-2007 Peng Wu
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

#include <stdio.h>
#include <errno.h>
#include <glib.h>
#include <glib/gstdio.h>
#include "memory_chunk.h"
#include "novel_types.h"
#include "ngram.h"

using namespace pinyin;

struct SingleGramItem{
    phrase_token_t m_token;
    guint32 m_freq;
};

SingleGram::SingleGram(){
    m_chunk.set_size(sizeof(guint32));
    memset(m_chunk.begin(), 0, sizeof(guint32));
}

SingleGram::SingleGram(void * buffer, size_t length, bool copy){
    if (copy)
        m_chunk.set_content(0, buffer, length);
    else
        m_chunk.set_chunk(buffer, length, NULL);
}

bool SingleGram::get_total_freq(guint32 & total) const{
    char * buf_begin = (char *)m_chunk.begin();
    total = *((guint32 *)buf_begin);
    return true;
}

bool SingleGram::set_total_freq(guint32 total){
    char * buf_begin = (char *)m_chunk.begin();
    *((guint32 *)buf_begin) = total;
    return true;
}

guint32 SingleGram::get_length(){
    /* get the number of items. */
    const SingleGramItem * begin = (const SingleGramItem *)
        ((const char *)(m_chunk.begin()) + sizeof(guint32));
    const SingleGramItem * end = (const SingleGramItem *) m_chunk.end();

    const guint32 length = end - begin;

    if (0 == length) {
        /* no items here, total freq should be zero. */
        guint32 total_freq = 0;
        assert(get_total_freq(total_freq));
        assert(0 == total_freq);
    }

    return length;
}

guint32 SingleGram::mask_out(phrase_token_t mask, phrase_token_t value){
    guint32 removed_items = 0;

    guint32 total_freq = 0;
    assert(get_total_freq(total_freq));

    const SingleGramItem * begin = (const SingleGramItem *)
        ((const char *)(m_chunk.begin()) + sizeof(guint32));
    const SingleGramItem * end = (const SingleGramItem *) m_chunk.end();

    for (const SingleGramItem * cur = begin; cur != end; ++cur) {
        if ((cur->m_token & mask) != value)
            continue;

        total_freq -= cur->m_freq;
        size_t offset = sizeof(guint32) +
            sizeof(SingleGramItem) * (cur - begin);
        m_chunk.remove_content(offset, sizeof(SingleGramItem));

        /* update chunk end. */
        end = (const SingleGramItem *) m_chunk.end();
        ++removed_items;
        --cur;
    }

    assert(set_total_freq(total_freq));
    return removed_items;
}

bool SingleGram::prune(){
    assert(false);
#if 0
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

static bool token_less_than(const SingleGramItem & lhs,const SingleGramItem & rhs){
    return lhs.m_token < rhs.m_token;
}

bool SingleGram::retrieve_all(/* out */ BigramPhraseWithCountArray array)
    const {
    const SingleGramItem * begin = (const SingleGramItem *)
        ((const char *)(m_chunk.begin()) + sizeof(guint32));
    const SingleGramItem * end = (const SingleGramItem *) m_chunk.end();

    guint32 total_freq;
    BigramPhraseItemWithCount bigram_item_with_count;
    assert(get_total_freq(total_freq));

    for ( const SingleGramItem * cur_item = begin; cur_item != end; ++cur_item){
        bigram_item_with_count.m_token = cur_item->m_token;
        bigram_item_with_count.m_count = cur_item->m_freq;
        bigram_item_with_count.m_freq = cur_item->m_freq / (gfloat)total_freq;
        g_array_append_val(array, bigram_item_with_count);
    }

    return true;
}

bool SingleGram::search(/* in */ PhraseIndexRange * range,
			/* out */ BigramPhraseArray array) const {
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

bool SingleGram::insert_freq( /* in */ phrase_token_t token,
                              /* in */ guint32 freq){
    SingleGramItem * begin = (SingleGramItem *)
        ((const char *)(m_chunk.begin()) + sizeof(guint32));
    SingleGramItem * end = (SingleGramItem *) m_chunk.end();
    SingleGramItem compare_item;
    compare_item.m_token = token;
    SingleGramItem * cur_item = std_lite::lower_bound(begin, end, compare_item, token_less_than);

    SingleGramItem insert_item;
    insert_item.m_token = token;
    insert_item.m_freq = freq;
    for ( ; cur_item != end; ++cur_item ){
        if ( cur_item->m_token > token ){
            size_t offset = sizeof(guint32) +
                sizeof(SingleGramItem) * (cur_item - begin);
            m_chunk.insert_content(offset, &insert_item,
                                   sizeof(SingleGramItem));
            return true;
        }
        if ( cur_item->m_token == token ){
            return false;
        }
    }
    m_chunk.insert_content(m_chunk.size(), &insert_item,
                           sizeof(SingleGramItem));
    return true;
}

bool SingleGram::remove_freq( /* in */ phrase_token_t token,
                              /* out */ guint32 & freq){
    freq = 0;
    const SingleGramItem * begin = (const SingleGramItem *)
        ((const char *)(m_chunk.begin()) + sizeof(guint32));
    const SingleGramItem * end = (const SingleGramItem *)m_chunk.end();
    SingleGramItem compare_item;
    compare_item.m_token = token;
    const SingleGramItem * cur_item = std_lite::lower_bound(begin, end, compare_item, token_less_than);

    for ( ; cur_item != end; ++cur_item ){
        if ( cur_item->m_token > token )
            return false;
        if ( cur_item->m_token == token ){
            freq = cur_item -> m_freq;
            size_t offset = sizeof(guint32) +
                sizeof(SingleGramItem) * (cur_item - begin);
            m_chunk.remove_content(offset, sizeof(SingleGramItem));
            return true;
        }
    }
    return false;
}

bool SingleGram::get_freq(/* in */ phrase_token_t token,
                          /* out */ guint32 & freq) const {
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

bool SingleGram::set_freq( /* in */ phrase_token_t token,
			   /* in */ guint32 freq){
    SingleGramItem * begin = (SingleGramItem *)
	((const char *)(m_chunk.begin()) + sizeof(guint32));
    SingleGramItem * end = (SingleGramItem *)m_chunk.end();
    SingleGramItem compare_item;
    compare_item.m_token = token;
    SingleGramItem * cur_item = std_lite::lower_bound(begin, end, compare_item, token_less_than);
    
    for ( ;cur_item != end; ++cur_item){
	if ( cur_item->m_token > token ){
	    return false;
	}
	if ( cur_item->m_token == token ){
	    cur_item -> m_freq = freq;
	    return true;
	}
    }
    return false;
}


namespace pinyin{

/* merge origin system info and delta user info */
bool merge_single_gram(SingleGram * merged, const SingleGram * system,
                       const SingleGram * user){
    if (NULL == system && NULL == user)
        return false;

    MemoryChunk & merged_chunk = merged->m_chunk;

    merged_chunk.set_size(0);

    if (NULL == system) {
        merged_chunk.set_content(0, user->m_chunk.begin(),
                                 user->m_chunk.size());
        return true;
    }

    if (NULL == user) {
        merged_chunk.set_content(0, system->m_chunk.begin(),
                                 system->m_chunk.size());
        return true;
    }

    /* clear merged. */
    merged_chunk.set_size(sizeof(guint32));

    /* merge the origin info and delta info */
    guint32 system_total, user_total;
    assert(system->get_total_freq(system_total));
    assert(user->get_total_freq(user_total));
    const guint32 merged_total = system_total + user_total;
    merged_chunk.set_content(0, &merged_total, sizeof(guint32));

    const SingleGramItem * cur_system = (const SingleGramItem *)
        (((const char *)(system->m_chunk.begin())) + sizeof(guint32));
    const SingleGramItem * system_end = (const SingleGramItem *)
        system->m_chunk.end();

    const SingleGramItem * cur_user = (const SingleGramItem *)
        (((const char *)(user->m_chunk.begin())) + sizeof(guint32));
    const SingleGramItem * user_end = (const SingleGramItem *)
        user->m_chunk.end();

    while (cur_system < system_end && cur_user < user_end) {

        if (cur_system->m_token < cur_user->m_token) {
            /* do append operation here */
            merged_chunk.append_content(cur_system, sizeof(SingleGramItem));
            cur_system++;
        } else if (cur_system->m_token > cur_user->m_token) {
            /* do append operation here */
            merged_chunk.append_content(cur_user, sizeof(SingleGramItem));
            cur_user++;
        } else {
            assert(cur_system->m_token == cur_user->m_token);

            SingleGramItem merged_item;
            merged_item.m_token = cur_system->m_token;
            merged_item.m_freq = cur_system->m_freq + cur_user->m_freq;

            merged_chunk.append_content(&merged_item, sizeof(SingleGramItem));
            cur_system++; cur_user++;
        }
    }

    /* add remained items. */
    while (cur_system < system_end) {
        merged_chunk.append_content(cur_system, sizeof(SingleGramItem));
        cur_system++;
    }

    while (cur_user < user_end) {
        merged_chunk.append_content(cur_user, sizeof(SingleGramItem));
        cur_user++;
    }

    return true;
}

};
