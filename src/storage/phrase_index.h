/* 
 *  libpinyin
 *  Library to deal with pinyin.
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
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef PHRASE_INDEX_H
#define PHRASE_INDEX_H

#include <stdio.h>
#include <glib.h>
#include "novel_types.h"
#include "chewing_key.h"
#include "pinyin_parser2.h"
#include "pinyin_phrase2.h"
#include "memory_chunk.h"
#include "phrase_index_logger.h"

/**
 * Phrase Index File Format
 *
 * Indirect Index: Index by Token
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * + Phrase Offset + Phrase Offset + Phrase Offset + ......  +
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * Phrase Content:
 * ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * + Phrase Length + number of  Pronunciations  + Uni-gram Frequency+
 * ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * + n Pronunciations + Phrase String(UCS2) +
 * ++++++++++++++++++++++++++++++++++++++++++
 */

namespace pinyin{

class PinyinLookup;

/* Store delta info by phrase index logger in user home directory.
 */

const size_t phrase_item_header = sizeof(guint8) + sizeof(guint8) + sizeof(guint32);

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

#if 0
    PhraseItem(MemoryChunk & chunk){
        m_chunk.set_content(0, chunk->begin(), chunk->size());
        assert ( m_chunk.size() >= phrase_item_header);
    }
#endif

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

    gfloat get_pronunciation_possibility(pinyin_option_t options,
				  ChewingKey * keys){
	guint8 phrase_length = get_phrase_length();
	guint8 npron = get_n_pronunciation();
	size_t offset = phrase_item_header + phrase_length * sizeof (utf16_t);
	char * buf_begin = (char *)m_chunk.begin();
	guint32 matched = 0, total_freq =0;
	for ( int i = 0 ; i < npron ; ++i){
	    char * chewing_begin = buf_begin + offset +
		i * (phrase_length * sizeof(ChewingKey) + sizeof(guint32));
	    guint32 * freq = (guint32 *)(chewing_begin +
                                         phrase_length * sizeof(ChewingKey));
	    total_freq += *freq;
	    if ( 0 == pinyin_compare_with_ambiguities2
                 (options,  keys,
                  (ChewingKey *)chewing_begin,phrase_length) ){
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
    
    void increase_pronunciation_possibility(pinyin_option_t options,
				     ChewingKey * keys,
				     gint32 delta);

    bool get_phrase_string(utf16_t * phrase);
    bool set_phrase_string(guint8 phrase_length, utf16_t * phrase);
    bool get_nth_pronunciation(size_t index, 
			       /* out */ ChewingKey * keys,
			       /* out */ guint32 & freq);
    /* Normally don't change the first pronunciation,
     * which decides the token number.
     */
    void append_pronunciation(ChewingKey * keys, guint32 freq);
    void remove_nth_pronunciation(size_t index);

    bool operator == (const PhraseItem & rhs) const{
        if (m_chunk.size() != rhs.m_chunk.size())
            return false;
        return memcmp(m_chunk.begin(), rhs.m_chunk.begin(),
                      m_chunk.size()) == 0;
    }

    bool operator != (const PhraseItem & rhs) const{
        return ! (*this == rhs);
    }
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
    
    /* binary memory chunk load/store method */
    bool load(MemoryChunk * chunk, 
	      table_offset_t offset, table_offset_t end);
    bool store(MemoryChunk * new_chunk, 
	       table_offset_t offset, table_offset_t & end);

    /* switch to logger format to reduce user storage */
    bool diff(SubPhraseIndex * oldone, PhraseIndexLogger * logger);
    bool merge(PhraseIndexLogger * logger);

    /* get token range in this sub phrase */
    int get_range(/* out */ PhraseIndexRange & range);
    
    /* Zero-gram */
    guint32 get_phrase_index_total_freq();
    int add_unigram_frequency(phrase_token_t token, guint32 delta);

    /* get_phrase_item function can't modify the phrase item size,
     * but can increment the freq of the special pronunciation,
     * or change the content without size increasing.
     */
    int get_phrase_item(phrase_token_t token, PhraseItem & item);
    int add_phrase_item(phrase_token_t token, PhraseItem * item);
    /* remove_phrase_item will substract item->get_unigram_frequency()
     * from m_total_freq
     */
    int remove_phrase_item(phrase_token_t token, /* out */ PhraseItem * & item);

};

class FacadePhraseIndex{
    friend class PinyinLookup;
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

    /* load/store logger format.
       the ownership of oldchunk and log is transfered to here. */
    bool diff(guint8 phrase_index, MemoryChunk * oldchunk,
              MemoryChunk * newlog);
    bool merge(guint8 phrase_index, MemoryChunk * log);

    /* compat all SubPhraseIndex m_phrase_content memory usage. */
    bool compat();

    /* get all available sub phrase indices. */
    int get_sub_phrase_range(guint8 & min_index, guint8 & max_index);

    /* get each sub phrase token range with phrase_index added */
    int get_range(guint8 phrase_index, /* out */ PhraseIndexRange & range);

    /* Zero-gram */
    guint32 get_phrase_index_total_freq(){
	return m_total_freq;
    }

    int add_unigram_frequency(phrase_token_t token, guint32 delta){
	guint8 index = PHRASE_INDEX_LIBRARY_INDEX(token);
	SubPhraseIndex * sub_phrase = m_sub_phrase_indices[index];
	if ( !sub_phrase )
	    return ERROR_NO_SUB_PHRASE_INDEX;
	m_total_freq += delta;
	return sub_phrase->add_unigram_frequency(token, delta);
    }

    /* get_phrase_item function can't modify the phrase item */
    int get_phrase_item(phrase_token_t token, PhraseItem & item){
	guint8 index = PHRASE_INDEX_LIBRARY_INDEX(token);
	SubPhraseIndex * sub_phrase = m_sub_phrase_indices[index];
	if ( !sub_phrase )
	    return ERROR_NO_SUB_PHRASE_INDEX;
	return sub_phrase->get_phrase_item(token, item);
    }

    int add_phrase_item(phrase_token_t token, PhraseItem * item){
	guint8 index = PHRASE_INDEX_LIBRARY_INDEX(token);
	SubPhraseIndex * & sub_phrase = m_sub_phrase_indices[index];
	if ( !sub_phrase ){
	    sub_phrase = new SubPhraseIndex;
	}   
	m_total_freq += item->get_unigram_frequency();
	return sub_phrase->add_phrase_item(token, item);
    }

    int remove_phrase_item(phrase_token_t token, PhraseItem * & item){
	guint8 index = PHRASE_INDEX_LIBRARY_INDEX(token);
	SubPhraseIndex * & sub_phrase = m_sub_phrase_indices[index];
	if ( !sub_phrase ){
	    return ERROR_NO_SUB_PHRASE_INDEX;
	}
	int result = sub_phrase->remove_phrase_item(token, item);
	if ( result )
	    return result;
	m_total_freq -= item->get_unigram_frequency();
	return result;
    }

};
 
};

#endif
