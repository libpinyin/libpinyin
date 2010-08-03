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

#include <assert.h>
#include <string.h>
#include "novel_types.h"
#include "pinyin_base.h"
#include "pinyin_phrase.h"
#include "pinyin_large_table.h"


PinyinBitmapIndexLevel::PinyinBitmapIndexLevel(PinyinCustomSettings * custom)
    :m_custom(custom){
    memset(m_pinyin_length_indexes, 0 , sizeof(m_pinyin_length_indexes));
}

void PinyinBitmapIndexLevel::reset(){
    for ( int k = PINYIN_ZeroInitial; k < PINYIN_Number_Of_Initials; k++)
	for ( int m = PINYIN_ZeroFinal; m < PINYIN_Number_Of_Finals; m++)
	    for ( int n = PINYIN_ZeroTone; n < PINYIN_Number_Of_Tones; n++){
		PinyinLengthIndexLevel * length_array = 
		    m_pinyin_length_indexes[k][m][n];
		if ( length_array )
		    delete length_array;
	    }
}

int PinyinBitmapIndexLevel::search( int phrase_length, /* in */ PinyinKey keys[],
	    /* out */ PhraseIndexRanges ranges) const{
    return initial_level_search(phrase_length, keys, ranges);
}

int PinyinBitmapIndexLevel::initial_level_search(int phrase_length, 
						 /* in */PinyinKey keys[],
						 /* out */ PhraseIndexRanges ranges) const{

#define MATCH(AMBIGUITY, ORIGIN, ANOTHER)  case ORIGIN:			\
    {                                                                   \
	result |= final_level_search((PinyinInitial)first_key.m_initial,\
				    phrase_length, keys, ranges);		\
	if ( custom.use_ambiguities [AMBIGUITY] ){			\
	    result |= final_level_search(ANOTHER,			\
					 phrase_length, keys, ranges);	\
	}								\
	return result;							\
    }
    
    //deal with the ambiguities

    int result = 0;
    PinyinKey& first_key = keys[0];
    PinyinCustomSettings &  custom= *m_custom;
    
    switch(first_key.m_initial){
	
	MATCH(PINYIN_AmbZhiZi, PINYIN_Zi, PINYIN_Zhi);
	MATCH(PINYIN_AmbZhiZi, PINYIN_Zhi, PINYIN_Zi);
	MATCH(PINYIN_AmbChiCi, PINYIN_Ci, PINYIN_Chi);
	MATCH(PINYIN_AmbChiCi, PINYIN_Chi, PINYIN_Ci);
	MATCH(PINYIN_AmbShiSi, PINYIN_Si, PINYIN_Shi);
	MATCH(PINYIN_AmbShiSi, PINYIN_Shi, PINYIN_Si);
	MATCH(PINYIN_AmbLeRi, PINYIN_Ri, PINYIN_Le);
	MATCH(PINYIN_AmbNeLe, PINYIN_Ne, PINYIN_Le);
	MATCH(PINYIN_AmbFoHe, PINYIN_Fo, PINYIN_He);
	MATCH(PINYIN_AmbFoHe, PINYIN_He, PINYIN_Fo);

    case PINYIN_Le:
	{
	    result |= final_level_search((PinyinInitial)first_key.m_initial, 
					phrase_length, keys, ranges);  
	    if ( custom.use_ambiguities [PINYIN_AmbLeRi] )		
		result |= final_level_search(PINYIN_Ri, phrase_length,
					     keys, ranges);	
	    if ( custom.use_ambiguities [PINYIN_AmbNeLe] )		
		result |= final_level_search(PINYIN_Ne, phrase_length, 
					     keys, ranges);
	    return result;
	}
    default:
	{
	    return final_level_search((PinyinInitial)first_key.m_initial,
				      phrase_length, 
				      keys, ranges);
	}
  }
#undef MATCH 
}

int PinyinBitmapIndexLevel::final_level_search(PinyinInitial initial,
					       int phrase_length, 
					       /* in */PinyinKey keys[],
					       /* out */ PhraseIndexRanges ranges) const{
#define MATCH(AMBIGUITY, ORIGIN, ANOTHER) case ORIGIN: 	                \
    {								        \
	result = tone_level_search(initial,(PinyinFinal) first_key.m_final,\
				   phrase_length, keys, ranges);		\
	if ( custom.use_ambiguities [AMBIGUITY] ){			\
	    result |= tone_level_search(initial, ANOTHER,		\
					phrase_length, keys, ranges);	\
	}								\
	return result;							\
    }
    
    int result = 0;
    PinyinKey& first_key = keys[0];
    PinyinCustomSettings &  custom= *m_custom;

    switch(first_key.m_final){
    case PINYIN_ZeroFinal:
	{
	    if (!custom.use_incomplete )
		return result;
	    for ( int i  = PINYIN_A; i < PINYIN_Number_Of_Finals; ++i){
		result |= tone_level_search(initial,(PinyinFinal)i , 
					    phrase_length, keys, ranges);
	    }
	    return result;
	}
	
	MATCH(PINYIN_AmbAnAng, PINYIN_An, PINYIN_Ang);
	MATCH(PINYIN_AmbAnAng, PINYIN_Ang, PINYIN_An);
	MATCH(PINYIN_AmbEnEng, PINYIN_En, PINYIN_Eng);
	MATCH(PINYIN_AmbEnEng, PINYIN_Eng, PINYIN_En);
	MATCH(PINYIN_AmbInIng, PINYIN_In, PINYIN_Ing);
	MATCH(PINYIN_AmbInIng, PINYIN_Ing, PINYIN_In);
	
    default:
	{
	    return tone_level_search(initial,(PinyinFinal)first_key.m_final, 
				     phrase_length, keys, ranges);
	}
    }
#undef MATCH
}

int PinyinBitmapIndexLevel::tone_level_search(PinyinInitial initial, 
					      PinyinFinal final,
					      int phrase_length, 
					      /* in */PinyinKey keys[],
					      /* out */ PhraseIndexRanges ranges) const{
    int result = 0;
    PinyinKey& first_key = keys[0];
    PinyinCustomSettings &  custom= *m_custom;

    switch ( first_key.m_tone ){
    case PINYIN_ZeroTone:
	{
		//deal with ZeroTone in pinyin table files.
	    for ( int i = PINYIN_ZeroTone; i < PINYIN_Number_Of_Tones; ++i){
		PinyinLengthIndexLevel * phrases = 
		    m_pinyin_length_indexes[initial][final][(PinyinTone)i];
		if ( phrases )
		    result |= phrases->search(phrase_length - 1, &custom,
					      keys + 1, ranges);
	    }
	    return result;
	}
    default:
	{
	    PinyinLengthIndexLevel * phrases = 
		m_pinyin_length_indexes[initial][final]
		[PINYIN_ZeroTone];
	    if ( phrases )
		result = phrases->search(phrase_length - 1, &custom,
					 keys + 1, ranges);
	    phrases = m_pinyin_length_indexes[initial][final]
		[(PinyinTone) first_key.m_tone];
	    if ( phrases )
		result |= phrases->search(phrase_length - 1, &custom, 
					  keys + 1, ranges);
	    return result;
	}
    }
	return result;
}

PinyinLengthIndexLevel::PinyinLengthIndexLevel(){
    m_pinyin_array_indexes = g_array_new(FALSE, TRUE, sizeof(void *));
}

PinyinLengthIndexLevel::~PinyinLengthIndexLevel(){
#define CASE(x) case x:							\
    {									\
	PinyinArrayIndexLevel<x> * array = g_array_index		\
	    (m_pinyin_array_indexes, PinyinArrayIndexLevel<x> *, x);	\
	if (array)							\
	    delete array;						\
	break;  							\
    }
    for ( int i = 0 ; i < m_pinyin_array_indexes->len; ++i){
	switch (i){
	    CASE(0);
	    CASE(1);
	    CASE(2);
	    CASE(3);
	    CASE(4);
	    CASE(5);
	    CASE(6);
	    CASE(7);
	    CASE(8);
	    CASE(9);
	    CASE(10);
	    CASE(11);
	    CASE(12);
	    CASE(13);
	    CASE(14);
	    CASE(15);
	default:
	    assert(false);
	}
    }
    g_array_free(m_pinyin_array_indexes, TRUE);
#undef CASE
}

int PinyinLengthIndexLevel::search( int phrase_length,
				    /* in */ PinyinCustomSettings * custom,
				    /* in */ PinyinKey keys[],
				    /* out */ PhraseIndexRanges ranges){
    int result = SEARCH_NONE;
    if(m_pinyin_array_indexes->len < phrase_length + 1)
	return result;
    if (m_pinyin_array_indexes->len > phrase_length + 1)
	result |= SEARCH_CONTINUED;
    
#define CASE(len) case len:						\
    {                                                                   \
	PinyinArrayIndexLevel<len> * array = g_array_index		\
	    (m_pinyin_array_indexes, PinyinArrayIndexLevel<len> *, len); \
	if ( !array )							\
	    return result;						\
	result |= array->search(custom, keys, ranges);			\
	return result;							\
    }

    switch ( phrase_length ){
	CASE(0);
	CASE(1);
	CASE(2);
	CASE(3);
	CASE(4);
	CASE(5);
	CASE(6);
	CASE(7);
	CASE(8);
	CASE(9);
	CASE(10);
	CASE(11);
	CASE(12);
	CASE(13);
	CASE(14);
	CASE(15);
    default:
	assert(false);
    }
#undef CASE
}

template<size_t phrase_length>
int PinyinArrayIndexLevel<phrase_length>::search(/* in */ PinyinCustomSettings * custom, /* in */ PinyinKey keys[], /* out */ PhraseIndexRanges ranges){
  PhraseExactLessThan<phrase_length> m_lessthan;
  PinyinIndexItem<phrase_length> * chunk_begin, * chunk_end;
  chunk_begin = (PinyinIndexItem<phrase_length> *)m_chunk.begin();
  chunk_end = (PinyinIndexItem<phrase_length> *)m_chunk.end();
  //do the search
  PinyinKey left_keys[phrase_length], right_keys[phrase_length];
  compute_lower_value(*custom, keys, left_keys, phrase_length);
  compute_upper_value(*custom, keys, right_keys, phrase_length);
  PinyinIndexItem<phrase_length> left(left_keys, -1), right(right_keys, -1);

  PinyinIndexItem<phrase_length> * begin = std_lite::lower_bound(chunk_begin, chunk_end, left, m_lessthan);
  PinyinIndexItem<phrase_length> * end = std_lite::upper_bound(chunk_begin, chunk_end, right, m_lessthan);

  return convert(custom, keys, begin, end, ranges);
}

template<size_t phrase_length>
int PinyinArrayIndexLevel<phrase_length>::convert(PinyinCustomSettings * custom, PinyinKey keys[], PinyinIndexItem<phrase_length> * begin, PinyinIndexItem<phrase_length> * end, PhraseIndexRanges ranges){
    PinyinIndexItem<phrase_length> * iter;
    PhraseIndexRange cursor;
    GArray * head, *cursor_head = NULL;
    int result = SEARCH_NONE;
    cursor.m_range_begin = -1; cursor.m_range_end = -1;
    for ( iter = begin; iter != end; ++iter){
	if ( ! 0 == 
	     pinyin_compare_with_ambiguities
	     (*custom, keys, iter->m_keys, phrase_length))
	    continue;
	phrase_token_t token = iter->m_token;
	head = ranges[PHRASE_INDEX_LIBRARY_INDEX(token)];
	if ( NULL == head )
	    continue;

        result |= SEARCH_OK;

	if ( cursor.m_range_begin == -1 ){
	    cursor.m_range_begin = token;
	    cursor.m_range_end = token + 1;
	    cursor_head = head;
	}else if (cursor.m_range_end == token && 
		  PHRASE_INDEX_LIBRARY_INDEX(cursor.m_range_end) == 
		  PHRASE_INDEX_LIBRARY_INDEX(token) ){
	    cursor.m_range_end++;
	}else {
	    g_array_append_val(cursor_head, cursor);
	    cursor.m_range_begin = token; cursor.m_range_end = token + 1;
	    cursor_head = head;
	}
    }
    if ( cursor.m_range_begin == -1 )
	return result;

    g_array_append_val(cursor_head, cursor);
    return result;
}

int PinyinBitmapIndexLevel::add_index( int phrase_length, /* in */ PinyinKey keys[] ,/* in */ phrase_token_t token){
    PinyinKey firstkey = keys[0];
    PinyinLengthIndexLevel * &length_array = 
	m_pinyin_length_indexes[firstkey.m_initial][firstkey.m_final][firstkey.m_tone];
    if ( ! length_array ){
	length_array = new PinyinLengthIndexLevel();
    }
    return length_array->add_index(phrase_length - 1, keys + 1, token);
}

int PinyinBitmapIndexLevel::remove_index( int phrase_length, /* in */ PinyinKey keys[] ,/* in */ phrase_token_t token){
    PinyinKey firstkey = keys[0];
    PinyinLengthIndexLevel * &length_array = 
	m_pinyin_length_indexes[firstkey.m_initial][firstkey.m_final][firstkey.m_tone];
    if ( length_array )
	return length_array->add_index(phrase_length - 1, keys + 1, token);
    return REMOVE_ITEM_DONOT_EXISTS;
}

int PinyinLengthIndexLevel::add_index( int phrase_length, /* in */ PinyinKey keys[] ,/* in */ phrase_token_t token){
    assert(phrase_length + 1 < MAX_PHRASE_LENGTH);
    if ( m_pinyin_array_indexes -> len <= phrase_length )
	g_array_set_size(m_pinyin_array_indexes, phrase_length + 1);
#define CASE(x)	case x:						     \
    {                                                                \
	PinyinArrayIndexLevel<x> * &array = g_array_index	     \
	    (m_pinyin_array_indexes, PinyinArrayIndexLevel<x> *, x); \
	if ( !array )						     \
	    array = new PinyinArrayIndexLevel<x>;		     \
	return array->add_index(keys, token);				     \
    }
    switch(phrase_length){
	CASE(0);
	CASE(1);
	CASE(2);
	CASE(3);
	CASE(4);
	CASE(5);
	CASE(6);
	CASE(7);
	CASE(8);
	CASE(9);
	CASE(10);
	CASE(11);
	CASE(12);
	CASE(13);
	CASE(14);
	CASE(15);
    default:
	assert(false);
    }
#undef CASE
}

int PinyinLengthIndexLevel::remove_index( int phrase_length, /* in */ PinyinKey keys[] ,/* in */ phrase_token_t token){
    assert(phrase_length + 1 < MAX_PHRASE_LENGTH);
    if ( m_pinyin_array_indexes -> len <= phrase_length )
	return false;
#define CASE(x)	case x:							\
    {									\
	PinyinArrayIndexLevel<x> * &array = g_array_index		\
	    (m_pinyin_array_indexes, PinyinArrayIndexLevel<x> *, x);	\
	if ( !array )							\
	    return false;						\
	return array->remove_index(keys, token);			\
    }
    switch(phrase_length){
	CASE(0);
	CASE(1);
	CASE(2);
	CASE(3);
	CASE(4);
	CASE(5);
	CASE(6);
	CASE(7);
	CASE(8);
	CASE(9);
	CASE(10);
	CASE(11);
	CASE(12);
	CASE(13);
	CASE(14);
	CASE(15);
    default:
	assert(false);
    }
#undef CASE
}

template<size_t phrase_length>
int PinyinArrayIndexLevel<phrase_length>::add_index(/* in */ PinyinKey keys[] ,/* in */ phrase_token_t token){
    PhraseExactLessThan<phrase_length> m_lessthan;
    PinyinIndexItem<phrase_length> * buf_begin, * buf_end;

    PinyinIndexItem<phrase_length> new_elem(keys, token);
    buf_begin = (PinyinIndexItem<phrase_length> *) m_chunk.begin();
    buf_end = (PinyinIndexItem<phrase_length> *) m_chunk.end();

    std_lite::pair<PinyinIndexItem<phrase_length> *, PinyinIndexItem<phrase_length> *> range;
    range = std_lite::equal_range(buf_begin, buf_end, new_elem, m_lessthan);

    PinyinIndexItem<phrase_length> * cur_elem;
    for ( cur_elem = range.first; 
	  cur_elem != range.second; ++cur_elem){
	if ( cur_elem->m_token == token )
	    return INSERT_ITEM_EXISTS;
	if ( cur_elem->m_token > token )
	    break;
    }

    int offset = (cur_elem - buf_begin) *
	sizeof(PinyinIndexItem<phrase_length>);
    m_chunk.insert_content(offset, &new_elem, 
			   sizeof ( PinyinIndexItem<phrase_length> ));
    return INSERT_OK;
}

template<size_t phrase_length>
int PinyinArrayIndexLevel<phrase_length>::remove_index(/* in */ PinyinKey keys[] ,/* in */ phrase_token_t token){
    PhraseExactLessThan<phrase_length> m_lessthan;
    PinyinIndexItem<phrase_length> * buf_begin, * buf_end;

    PinyinIndexItem<phrase_length> new_elem(keys, token);
    buf_begin = (PinyinIndexItem<phrase_length> *) m_chunk.begin();
    buf_end = (PinyinIndexItem<phrase_length> *) m_chunk.end();

    std_lite::pair<PinyinIndexItem<phrase_length> *, PinyinIndexItem<phrase_length> *> range;
    range = std_lite::equal_range(buf_begin, buf_end, new_elem, m_lessthan);

    PinyinIndexItem<phrase_length> * cur_elem;
    for ( cur_elem = range.first; 
	  cur_elem != range.second; ++cur_elem){
	if ( cur_elem->m_token == token )
	    break;
    }
    if (cur_elem->m_token != token )
	return REMOVE_ITEM_DONOT_EXISTS;

    int offset = (cur_elem - buf_begin) *
	sizeof(PinyinIndexItem<phrase_length>);
    m_chunk.remove_content(offset, sizeof (PinyinIndexItem<phrase_length>));
    return REMOVE_OK;
}

bool PinyinLargeTable::load_text(FILE * infile){
    char pinyin[256];
    char phrase[256];
    phrase_token_t token;
    size_t freq;    
    while ( !feof(infile)){
        fscanf(infile, "%s", pinyin);
        fscanf(infile, "%s", phrase);
        fscanf(infile, "%ld", &token);
	fscanf(infile, "%ld", &freq);	
	
	PinyinDefaultParser parser;
	NullPinyinValidator validator;
	PinyinKeyVector keys;
	PinyinKeyPosVector poses;
	
	keys = g_array_new(FALSE, FALSE, sizeof( PinyinKey));
	poses = g_array_new(FALSE, FALSE, sizeof( PinyinKeyPos));
	parser.parse(validator, keys, poses, pinyin);
	
	add_index( keys->len, (PinyinKey *)keys->data, token);

	g_array_free(keys, true);
	g_array_free(poses, true);
    }
	return true;
}

bool PinyinBitmapIndexLevel::load(MemoryChunk * chunk, table_offset_t offset,
				  table_offset_t end){
    reset();
    char * buf_begin = (char *) chunk->begin();
    table_offset_t phrase_begin, phrase_end;
    table_offset_t * index = (table_offset_t *) (buf_begin + offset);
    phrase_end = *index;
    for ( int m = 0; m < PINYIN_Number_Of_Initials; ++m )
	for ( int n = 0; n < PINYIN_Number_Of_Finals; ++n)
	    for ( int k = 0; k < PINYIN_Number_Of_Tones; ++k){
		phrase_begin = phrase_end;
		index++;
		phrase_end = *index;
		if ( phrase_begin == phrase_end ) //null pointer
		    continue;
		PinyinLengthIndexLevel * phrases = new PinyinLengthIndexLevel;
		m_pinyin_length_indexes[m][n][k] = phrases;
		phrases->load(chunk, phrase_begin, phrase_end - 1);
		assert( phrase_end <= end );
		assert( *(buf_begin + phrase_end - 1) == c_separate);
	    }
    offset += (PINYIN_Number_Of_Initials * PINYIN_Number_Of_Finals * PINYIN_Number_Of_Tones + 1) * sizeof ( table_offset_t);
    assert( c_separate == *(buf_begin + offset));
    return true;
}

bool PinyinBitmapIndexLevel::store(MemoryChunk * new_chunk, 
				   table_offset_t offset,
				   table_offset_t & end){
    table_offset_t phrase_end;
    table_offset_t index = offset;
    offset += (PINYIN_Number_Of_Initials * PINYIN_Number_Of_Finals * PINYIN_Number_Of_Tones + 1) * sizeof ( table_offset_t);
    //add '#'
    new_chunk->set_content(offset, &c_separate, sizeof(char));
    offset += sizeof(char);
    new_chunk->set_content(index, &offset, sizeof(table_offset_t));
    index += sizeof(table_offset_t);
    for ( int m = 0; m < PINYIN_Number_Of_Initials; ++m )
	for ( int n = 0; n < PINYIN_Number_Of_Finals; ++n)
	    for ( int k = 0; k < PINYIN_Number_Of_Tones; ++k){
		PinyinLengthIndexLevel * phrases = m_pinyin_length_indexes[m][n][k];
		if ( !phrases ){ //null pointer
		    new_chunk->set_content(index, &offset, sizeof(table_offset_t));
		    index += sizeof(table_offset_t);
		    continue;
		}
		phrases->store(new_chunk, offset, phrase_end); //has a end '#'
		offset = phrase_end;
		//add '#'
		new_chunk->set_content(offset, &c_separate, sizeof(char));
		offset += sizeof(char);
		new_chunk->set_content(index, &offset, sizeof(table_offset_t));
		index += sizeof(table_offset_t);
	    }
    end = offset;
    return true;
}

bool PinyinLengthIndexLevel::load(MemoryChunk * chunk, table_offset_t offset, table_offset_t end){
    char * buf_begin = (char *) chunk->begin();
    guint32 nindex = *((guint32 *)(buf_begin + offset));
    table_offset_t * index = (table_offset_t *)
	(buf_begin + offset + sizeof(guint32));

    table_offset_t phrase_begin, phrase_end = *index;
    m_pinyin_array_indexes = g_array_new(FALSE, TRUE, sizeof(void *));
    for ( size_t i = 1; i <= nindex; i++){
	phrase_begin = phrase_end;
	index++;
	phrase_end = *index;
	if ( phrase_begin == phrase_end ){
	    void * null = NULL;
	    g_array_append_val(m_pinyin_array_indexes , null);
	    continue;
	}

#define CASE(x) case x - 1:						\
	{								\
	    PinyinArrayIndexLevel<x> * phrase = new PinyinArrayIndexLevel<x>; \
	    phrase->load(chunk, phrase_begin, phrase_end - 1);		\
	    assert( *(buf_begin + phrase_end - 1) == c_separate);	\
	    assert( phrase_end <= end );				\
	    g_array_append_val(m_pinyin_array_indexes, phrase);		\
	    break;							\
	}
	switch ( i ){
	    CASE(0);
	    CASE(1);
	    CASE(2);
	    CASE(3);
	    CASE(4);
	    CASE(5);
	    CASE(6);
	    CASE(7);
	    CASE(8);
	    CASE(9);
	    CASE(10);
	    CASE(11);
	    CASE(12);
	    CASE(13);
	    CASE(14);
	    CASE(15);
	default:
	    assert(false);
	}
#undef CASE
    }
    offset += sizeof(guint32) + (nindex + 1) * sizeof(table_offset_t);
    assert ( c_separate == * (buf_begin + offset) );
    return true;
}

bool PinyinLengthIndexLevel::store(MemoryChunk * new_chunk, table_offset_t offset, table_offset_t& end){
    guint32 nindex = m_pinyin_array_indexes->len;
    new_chunk->set_content(offset, &nindex, sizeof(guint32));
    table_offset_t index = offset + sizeof(guint32);

    offset += sizeof(guint32) + (nindex + 1) * sizeof(table_offset_t);
    new_chunk->set_content(offset, &c_separate, sizeof(char));
    offset += sizeof(char);
    new_chunk->set_content(index, &offset, sizeof(table_offset_t));
    index += sizeof(table_offset_t);
    table_offset_t phrase_end;
    for ( size_t i = 0 ; i < m_pinyin_array_indexes->len; ++i){
#define CASE(x) case x:	{						\
	    PinyinArrayIndexLevel<x> * phrase = g_array_index		\
		(m_pinyin_array_indexes, PinyinArrayIndexLevel<x> * , i); \
	    if ( !phrase ){						\
		new_chunk->set_content					\
		    (index, &offset, sizeof(table_offset_t));		\
		index += sizeof(table_offset_t);			\
		continue;						\
	    }								\
	    phrase->store(new_chunk, offset, phrase_end);		\
	    offset = phrase_end;					\
	    /*add '#'*/							\
	    new_chunk->set_content(offset, &c_separate, sizeof(char));	\
	    offset += sizeof(char);					\
	    new_chunk->set_content(index, &offset, sizeof(table_offset_t));\
	    index += sizeof(table_offset_t);				\
	    break;							\
	}
	switch ( i ){
	    CASE(0);
	    CASE(1);
	    CASE(2);
	    CASE(3);
	    CASE(4);
	    CASE(5);
	    CASE(6);
	    CASE(7);
	    CASE(8);
	    CASE(9);
	    CASE(10);
	    CASE(11);
	    CASE(12);
	    CASE(13);
	    CASE(14);
	    CASE(15);
	default:
	    assert(false);
	}
#undef CASE							
    }
    end = offset;
    return true;
}

template<size_t phrase_length>
bool PinyinArrayIndexLevel<phrase_length>::
load(MemoryChunk * chunk, table_offset_t offset, table_offset_t end){
    char * buf_begin = (char *) chunk->begin();
    m_chunk.set_chunk(buf_begin + offset, end - offset, NULL);
    return true;
}

template<size_t phrase_length>
bool PinyinArrayIndexLevel<phrase_length>::
store(MemoryChunk * new_chunk, table_offset_t offset, table_offset_t& end){
    new_chunk->set_content(offset, m_chunk.begin(), m_chunk.size());
    end = offset + m_chunk.size();
    return true;
}
