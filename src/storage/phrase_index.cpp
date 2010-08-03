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

#include "phrase_index.h"

bool PhraseItem::set_n_pronunciation(guint8 n_prouns){
    m_chunk.set_content(sizeof(guint8), &n_prouns, sizeof(guint8));
    return true;
}

bool PhraseItem::get_nth_pronunciation(size_t index, PinyinKey * pinyin, guint32 & freq){
    guint8 phrase_length = get_phrase_length();
    table_offset_t offset = phrase_item_header + phrase_length * sizeof( utf16_t) + index * ( phrase_length * sizeof (PinyinKey) + sizeof(guint32));
    bool retval = m_chunk.get_content(offset, pinyin, phrase_length * sizeof(PinyinKey));
    if ( !retval )
	return retval;
    return m_chunk.get_content(offset + phrase_length * sizeof(PinyinKey), &freq , sizeof(guint32));
}

void PhraseItem::append_pronunciation(PinyinKey * pinyin, guint32 freq){
    guint8 phrase_length = get_phrase_length();
    set_n_pronunciation(get_n_pronunciation() + 1);
    m_chunk.set_content(m_chunk.size(), pinyin, phrase_length * sizeof(PinyinKey));
    m_chunk.set_content(m_chunk.size(), &freq, sizeof(guint32));
}

void PhraseItem::remove_nth_pronunciation(size_t index){
    guint8 phrase_length = get_phrase_length();
    set_n_pronunciation(get_n_pronunciation() - 1);
    size_t offset = phrase_item_header + phrase_length * sizeof ( utf16_t ) + index * (phrase_length * sizeof (PinyinKey) + sizeof(guint32));
    m_chunk.remove_content(offset, phrase_length * sizeof(PinyinKey) + sizeof(guint32));
}

bool PhraseItem::get_phrase_string(utf16_t * phrase){
    guint8 phrase_length = get_phrase_length();
    return m_chunk.get_content(phrase_item_header, phrase, phrase_length * sizeof(utf16_t));
}

bool PhraseItem::set_phrase_string(guint8 phrase_length, utf16_t * phrase){
    m_chunk.set_content(0, &phrase_length, sizeof(guint8));
    m_chunk.set_content(phrase_item_header, phrase, phrase_length * sizeof(utf16_t));
    return true;
}

void PhraseItem::increase_pinyin_possibility(PinyinCustomSettings & custom,
					     PinyinKey * pinyin_keys,
					     gint32 delta){
    guint8 phrase_length = get_phrase_length();
    guint8 npron = get_n_pronunciation();
    size_t offset = phrase_item_header + phrase_length * sizeof ( utf16_t );
    char * buf_begin = (char *) m_chunk.begin();
    guint32 total_freq = 0;
    for ( int i = 0 ; i < npron ; ++i){
	char * pinyin_begin = buf_begin + offset +
	    i * ( phrase_length * sizeof(PinyinKey) + sizeof(guint32) );
	guint32 * freq = (guint32 *)(pinyin_begin + phrase_length * sizeof(PinyinKey));
	total_freq += *freq;
	if ( 0 == pinyin_compare_with_ambiguities(custom,
						  (PinyinKey *)pinyin_begin,
						  pinyin_keys,
						  phrase_length)){
	    //protect against total_freq overflow.
	    if ( delta > 0 && total_freq > total_freq + delta )
		return;
	    *freq += delta;
	    total_freq += delta;
	}
    }
}


guint32 SubPhraseIndex::get_phrase_index_total_freq(){
    return m_total_freq;
}

bool SubPhraseIndex::add_unigram_frequency(phrase_token_t token, guint32 delta){
    table_offset_t offset;
    guint32 freq;
    bool result = m_phrase_index.get_content
	((token & PHRASE_MASK) 
	 * sizeof(table_offset_t), &offset, sizeof(table_offset_t));

    if ( !result)
	return result;

    if ( 0 == offset )
	return false;

    result = m_phrase_content.get_content
	(offset + sizeof(guint8) + sizeof(guint8), &freq, sizeof(guint32));
    //protect total_freq overflow
    if ( delta > 0 && m_total_freq > m_total_freq + delta )
	return false;
    freq += delta;
    m_total_freq += delta;
    return m_phrase_content.set_content(offset + sizeof(guint8) + sizeof(guint8), &freq, sizeof(guint32));
}

bool SubPhraseIndex::get_phrase_item(phrase_token_t token, PhraseItem & item){
    table_offset_t offset;
    guint8 phrase_length;
    guint8 n_prons;
    
    bool result = m_phrase_index.get_content
	((token & PHRASE_MASK) 
	 * sizeof(table_offset_t), &offset, sizeof(table_offset_t));

    if ( !result )
	return result;

    if ( 0 == offset )
	return false;

    result = m_phrase_content.get_content(offset, &phrase_length, sizeof(guint8));
    if ( !result ) 
	return result;
    
    result = m_phrase_content.get_content(offset+sizeof(guint8), &n_prons, sizeof(guint8));
    if ( !result ) 
	return result;

    size_t length = phrase_item_header + phrase_length * sizeof ( utf16_t ) + n_prons * ( phrase_length * sizeof (PinyinKey) + sizeof(guint32) );
    item.m_chunk.set_chunk((char *)m_phrase_content.begin() + offset, length, NULL);
    return true;
}

bool SubPhraseIndex::add_phrase_item(phrase_token_t token, PhraseItem * item){
    table_offset_t offset = m_phrase_content.size();
    if ( 0 == offset )
	offset = 8;
    m_phrase_content.set_content(offset, item->m_chunk.begin(), item->m_chunk.size());
    m_phrase_index.set_content((token & PHRASE_MASK) 
			       * sizeof(table_offset_t), &offset, sizeof(table_offset_t));
    m_total_freq += item->get_unigram_frequency();
    return true;
}

bool SubPhraseIndex::remove_phrase_item(phrase_token_t token, PhraseItem * & item){
    table_offset_t offset;
    guint8 phrase_length;
    guint8 n_prons;
    
    bool result = m_phrase_index.get_content
	((token & PHRASE_MASK)
	 * sizeof(table_offset_t), &offset, sizeof(table_offset_t));
    
    if ( !result )
	return result;

    if ( 0 == offset )
	return false;

    result = m_phrase_content.get_content(offset, &phrase_length, sizeof(guint8));
    if ( !result )
	return result;

    result = m_phrase_content.get_content(offset+sizeof(guint8), &n_prons, sizeof(guint8));
    if ( !result )
	return result;
    
    size_t length = phrase_item_header + phrase_length * sizeof ( utf16_t ) + n_prons * ( phrase_length * sizeof (PinyinKey) + sizeof(guint32) );
    item = new PhraseItem;
    //implictly copy data from m_chunk_content.
    item->m_chunk.set_content(0, (char *) m_phrase_content.begin() + offset, length);

    const table_offset_t zero_const = 0;
    m_phrase_index.set_content((token & PHRASE_MASK)
			       * sizeof(table_offset_t), &zero_const, sizeof(table_offset_t));
    m_total_freq -= item->get_unigram_frequency();
    return true;
}

bool FacadePhraseIndex::load(guint8 phrase_index, MemoryChunk * chunk){
    SubPhraseIndex * & sub_phrases = m_sub_phrase_indices[phrase_index];
    if ( !sub_phrases ){
	sub_phrases = new SubPhraseIndex;
    }
    
    bool retval = sub_phrases->load(chunk, 0, chunk->size());
    if ( !retval )
	return retval;
    m_total_freq += sub_phrases->get_phrase_index_total_freq();
    return retval;
}

bool FacadePhraseIndex::store(guint8 phrase_index, MemoryChunk * new_chunk){
    table_offset_t end;
    SubPhraseIndex * & sub_phrases = m_sub_phrase_indices[phrase_index];
    if ( !sub_phrases )
	return false;
    
    sub_phrases->store(new_chunk, 0, end);
    return true;
}

bool FacadePhraseIndex::unload(guint8 phrase_index){
    SubPhraseIndex * & sub_phrases = m_sub_phrase_indices[phrase_index];
    if ( !sub_phrases )
	return false;
    m_total_freq -= sub_phrases->get_phrase_index_total_freq();
    delete sub_phrases;
    sub_phrases = NULL;
    return true;
}

bool SubPhraseIndex::load(MemoryChunk * chunk, 
			  table_offset_t offset, table_offset_t end){
    //save the memory chunk
    if ( m_chunk ){
	delete m_chunk;
	m_chunk = NULL;
    }
    m_chunk = chunk;
    
    char * buf_begin = (char *)chunk->begin();
    chunk->get_content(offset, &m_total_freq, sizeof(guint32));
    offset += sizeof(guint32);
    table_offset_t index_one, index_two, index_three;
    chunk->get_content(offset, &index_one, sizeof(table_offset_t));
    offset += sizeof(table_offset_t);
    chunk->get_content(offset, &index_two, sizeof(table_offset_t));
    offset += sizeof(table_offset_t);
    chunk->get_content(offset, &index_three, sizeof(table_offset_t));
    offset += sizeof(table_offset_t);
    g_return_val_if_fail(*(buf_begin + offset) == c_separate, FALSE);
    g_return_val_if_fail(*(buf_begin + index_two - 1) == c_separate, FALSE);
    g_return_val_if_fail(*(buf_begin + index_three - 1) == c_separate, FALSE);
    m_phrase_index.set_chunk(buf_begin + index_one, 
			     index_two - 1 - index_one, NULL);
    m_phrase_content.set_chunk(buf_begin + index_two, 
				 index_three - 1 - index_two, NULL);
    g_return_val_if_fail( index_three <= end, FALSE);
    return true;
}

bool SubPhraseIndex::store(MemoryChunk * new_chunk, 
			   table_offset_t offset, table_offset_t& end){
    new_chunk->set_content(offset, &m_total_freq, sizeof(guint32));
    table_offset_t index = offset + sizeof(guint32);
        
    offset = index + sizeof(table_offset_t) * 3 ;
    new_chunk->set_content(offset, &c_separate, sizeof(char));
    offset += sizeof(char);
    
    new_chunk->set_content(index, &offset, sizeof(table_offset_t));
    index += sizeof(table_offset_t);
    new_chunk->set_content(offset, m_phrase_index.begin(), m_phrase_index.size());
    offset += m_phrase_index.size();
    new_chunk->set_content(offset, &c_separate, sizeof(char));
    offset += sizeof(char);

    new_chunk->set_content(index, &offset, sizeof(table_offset_t));
    index += sizeof(table_offset_t);
    
    new_chunk->set_content(offset, m_phrase_content.begin(), m_phrase_content.size());
    offset += m_phrase_content.size();
    new_chunk->set_content(offset, &c_separate, sizeof(char));
    offset += sizeof(char);
    new_chunk->set_content(index, &offset, sizeof(table_offset_t));
    return true;
}

bool FacadePhraseIndex::load_text(guint8 phrase_index, FILE * infile){
    SubPhraseIndex * & sub_phrases = m_sub_phrase_indices[phrase_index];
    if ( !sub_phrases ){
	sub_phrases = new SubPhraseIndex;
    }

    char pinyin[256];
    char phrase[256];
    phrase_token_t token;
    size_t freq;
    PhraseItem * item_ptr = new PhraseItem;
    phrase_token_t cur_token = 0;
    while ( !feof(infile)){
        fscanf(infile, "%s", pinyin);
        fscanf(infile, "%s", phrase);
        fscanf(infile, "%ld", &token);
	fscanf(infile, "%ld", &freq);
	if ( feof(infile) )
	    break;

	glong written;
	utf16_t * phrase_utf16 = g_utf8_to_utf16(phrase, -1, NULL, 
					       &written, NULL);
	
	if ( 0 == cur_token ){
	    cur_token = token;
	    item_ptr->set_phrase_string(written, phrase_utf16);
	}

	if ( cur_token != token ){
	    add_phrase_item( cur_token, item_ptr);
	    delete item_ptr;
	    item_ptr = new PhraseItem;
	    cur_token = token;
	    item_ptr->set_phrase_string(written, phrase_utf16);
	}

	PinyinDefaultParser parser;
	NullPinyinValidator validator;
	PinyinKeyVector keys;
	PinyinKeyPosVector poses;
	
	keys = g_array_new(FALSE, FALSE, sizeof( PinyinKey));
	poses = g_array_new(FALSE, FALSE, sizeof( PinyinKeyPos));
	parser.parse(validator, keys, poses, pinyin);
	
	assert ( item_ptr->get_phrase_length() == keys->len );
	item_ptr->append_pronunciation((PinyinKey *)keys->data, freq);

	g_array_free(keys, TRUE);
	g_array_free(poses, TRUE);
	g_free(phrase_utf16);
    }

    add_phrase_item( cur_token, item_ptr);
    delete item_ptr;
    m_total_freq += m_sub_phrase_indices[phrase_index]->get_phrase_index_total_freq();
    return true;
}
