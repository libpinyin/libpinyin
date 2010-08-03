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

#ifndef PHRASE_INDEX_H
#define PHRASE_INDEX_H

#include <stdio.h>
#include <glib.h>
#include "novel_types.h"
#include "pinyin_base.h"
#include "pinyin_phrase.h"
#include "memory_chunk.h"

class PinyinLookup;

namespace novel{

/* Because this is not large,
 * Store this in user home directory.
 */

const int phrase_item_header = sizeof(guint8) + sizeof(guint8) + sizeof(guint32);

class PhraseItem{
    friend class SubPhraseIndex;
private:
    MemoryChunk m_chunk;
    bool set_n_pronunciation(guint8 n_prouns);
public:
    /* Null Constructor */
    PhraseItem(){
	m_chunk.set_size(phrase_item_header);
	memset(m_chunk.begin(), 0, m_chunk.size());
    }

    PhraseItem(MemoryChunk chunk){
	m_chunk = chunk;
	assert ( m_chunk.size() >= phrase_item_header);
    }

    /* functions */
    guint8 get_phrase_length(){
	char * buf_begin = (char *)m_chunk.begin();
	return (*(guint8 *)buf_begin);
    }

    guint8 get_n_pronunciation(){
	char * buf_begin = ( char *) m_chunk.begin();
	return (*(guint8 *)(buf_begin + sizeof(guint8)));
    }

    guint32 get_unigram_frequency(){
	char * buf_begin = (char *)m_chunk.begin();
	return (*(guint32 *)(buf_begin + sizeof(guint8) + sizeof(guint8)));
    }

    gfloat get_pinyin_possibility(PinyinCustomSettings & custom, 
				  PinyinKey * pinyin_keys){
	guint8 phrase_length = get_phrase_length();
	guint8 npron = get_n_pronunciation();
	size_t offset = phrase_item_header + phrase_length * sizeof ( utf16_t );
	char * buf_begin = (char *)m_chunk.begin();
	guint32 matched = 0, total_freq =0;
	for ( int i = 0 ; i < npron ; ++i){
	    char * pinyin_begin = buf_begin + offset + 
		i * ( phrase_length * sizeof(PinyinKey) + sizeof(guint32) );
	    guint32 * freq = (guint32 *)(pinyin_begin + phrase_length * sizeof(PinyinKey));
	    total_freq += *freq;
	    if ( 0 == pinyin_compare_with_ambiguities(custom, 
						      (PinyinKey *)pinyin_begin,
						      pinyin_keys,
						      phrase_length)){
		matched += *freq;
	    }
	}
	// use preprocessor to avoid zero freq, in gen_pinyin_table.
	/*
	if ( 0 == total_freq )
	    return 0.1;
	*/
	gfloat retval = matched / (gfloat) total_freq;
	/*
	if ( 0 == retval )
	    return 0.03;
	*/
	return retval;
    }
    
    void increase_pinyin_possibility(PinyinCustomSettings & custom,
				     PinyinKey * pinyin_keys,
				     gint32 delta);

    bool get_phrase_string(utf16_t * phrase);
    bool set_phrase_string(guint8 phrase_length, utf16_t * phrase);
    bool get_nth_pronunciation(size_t index, 
			       /* out */ PinyinKey * pinyin, 
			       /* out */ guint32 & freq);
    /* Normally don't change the first pronunciation,
     * which decides the token number.
     */
    void append_pronunciation(PinyinKey * pinyin, guint32 freq);
    void remove_nth_pronunciation(size_t index);
};

/*
 *  In Sub Phrase Index, token == (token & PHRASE_MASK).
 */

class SubPhraseIndex{
private:
    guint32 m_total_freq;
    MemoryChunk m_phrase_index;
    MemoryChunk m_phrase_content;
    MemoryChunk * m_chunk;
public:
    SubPhraseIndex():m_total_freq(0){
	m_chunk = NULL;
    }

    ~SubPhraseIndex(){
	reset();
    }

    void reset(){
	if ( m_chunk ){
	    delete m_chunk;
	    m_chunk = NULL;
	}
    }    
    
    bool load(MemoryChunk * chunk, 
	      table_offset_t offset, table_offset_t end);
    bool store(MemoryChunk * new_chunk, 
	       table_offset_t offset, table_offset_t & end);
    
    /* Zero-gram */
    guint32 get_phrase_index_total_freq();
    bool add_unigram_frequency(phrase_token_t token, guint32 delta);
    /* get_phrase_item function can't modify the phrase item, 
     * but can increment the freq of the special pronunciation.
     */
    bool get_phrase_item(phrase_token_t token, PhraseItem & item);
    bool add_phrase_item(phrase_token_t token, PhraseItem * item);
    /* remove_phrase_item will substract item->get_unigram_frequency()
     * from m_total_freq
     */
    bool remove_phrase_item(phrase_token_t token, /* out */ PhraseItem * & item);
};

class FacadePhraseIndex{
    friend class ::PinyinLookup;
private:
    guint32 m_total_freq;
    SubPhraseIndex * m_sub_phrase_indices[PHRASE_INDEX_LIBRARY_COUNT];
public:
    FacadePhraseIndex(){
	m_total_freq = 0;
	memset(m_sub_phrase_indices, 0, sizeof(m_sub_phrase_indices));
    }

    ~FacadePhraseIndex(){
	for ( size_t i = 0; i < PHRASE_INDEX_LIBRARY_COUNT; ++i){
	    if ( m_sub_phrase_indices[i] ){
		delete m_sub_phrase_indices[i];
		m_sub_phrase_indices[i] = NULL;
	    }
	}
    }

    /* load/store single sub phrase index, according to the config files. */
    bool load_text(guint8 phrase_index, FILE * infile);
    bool load(guint8 phrase_index, MemoryChunk * chunk);
    bool store(guint8 phrase_index, MemoryChunk * new_chunk);
    bool unload(guint8 phrase_index);

    /* Zero-gram */
    guint32 get_phrase_index_total_freq(){
	return m_total_freq;
    }

    bool add_unigram_frequency(phrase_token_t token, guint32 delta){
	guint8 index = PHRASE_INDEX_LIBRARY_INDEX(token);
	SubPhraseIndex * sub_phrase = m_sub_phrase_indices[index];
	if ( !sub_phrase )
	    return false;
	m_total_freq += delta;
	return sub_phrase->add_unigram_frequency(token, delta);
    }

    /* get_phrase_item function can't modify the phrase item */
    bool get_phrase_item(phrase_token_t token, PhraseItem & item){
	guint8 index = PHRASE_INDEX_LIBRARY_INDEX(token);
	SubPhraseIndex * sub_phrase = m_sub_phrase_indices[index];
	if ( !sub_phrase )
	    return false;
	return sub_phrase->get_phrase_item(token, item);
    }

    bool add_phrase_item(phrase_token_t token, PhraseItem * item){
	guint8 index = PHRASE_INDEX_LIBRARY_INDEX(token);
	SubPhraseIndex * & sub_phrase = m_sub_phrase_indices[index];
	if ( !sub_phrase ){
	    sub_phrase = new SubPhraseIndex;
	}   
	m_total_freq += item->get_unigram_frequency();
	return sub_phrase->add_phrase_item(token, item);
    }

    bool remove_phrase_item(phrase_token_t token, PhraseItem * & item){
	guint8 index = PHRASE_INDEX_LIBRARY_INDEX(token);
	SubPhraseIndex * & sub_phrase = m_sub_phrase_indices[index];
	if ( !sub_phrase ){
	    return false;
	}
	bool result = sub_phrase->remove_phrase_item(token, item);
	if ( !result )
	    return result;
	m_total_freq -= item->get_unigram_frequency();
	return result;
    }
};
 
};

using namespace novel;





#endif
