/* 
 *  libpinyin
 *  Library to deal with pinyin.
 *  
 *  Copyright (C) 2010 Peng Wu
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

#include <assert.h>
#include <string.h>
#include "phrase_large_table.h"


/* class definition */

namespace pinyin{

class PhraseLengthIndexLevel{
protected:
    GArray* m_phrase_array_indexes;
public:
    PhraseLengthIndexLevel();
    ~PhraseLengthIndexLevel();

    /* load/store method */
    bool load(MemoryChunk * chunk, table_offset_t offset, table_offset_t end);
    bool store(MemoryChunk * new_chunk, table_offset_t offset, table_offset_t & end);

    /* search/add_index/remove_index method */
    int search( int phrase_length, /* in */ utf16_t phrase[],
                /* out */ phrase_token_t & token);

    int add_index( int phrase_length, /* in */ utf16_t phrase[], /* in */ phrase_token_t token);
    int remove_index( int phrase_length, /* in */ utf16_t phrase[], /* out */ phrase_token_t & token);
};

template<size_t phrase_length>
class PhraseArrayIndexLevel{
protected:
    MemoryChunk m_chunk;
public:
    bool load(MemoryChunk * chunk, table_offset_t offset, table_offset_t end);
    bool store(MemoryChunk * new_chunk, table_offset_t offset, table_offset_t & end);

    /* search/add_index/remove_index method */
    int search( /* in */ utf16_t phrase[],
                /* out */ phrase_token_t & token);

    int add_index(/* in */ utf16_t phrase[], /* in */ phrase_token_t token);
    int remove_index(/* in */ utf16_t phrase[], /* out */ phrase_token_t & token);
};

};

using namespace pinyin;

/* class implementation */

template<size_t phrase_length>
struct PhraseIndexItem{
    phrase_token_t m_token;
    utf16_t m_phrase[phrase_length];
public:
    PhraseIndexItem<phrase_length>(utf16_t phrase[], phrase_token_t token){
        memmove(m_phrase, phrase, sizeof(utf16_t) * phrase_length);
        m_token = token;
    }
};

template<size_t phrase_length>
static int phrase_compare(const PhraseIndexItem<phrase_length> &lhs,
                          const PhraseIndexItem<phrase_length> &rhs){
    utf16_t * phrase_lhs = (utf16_t *) lhs.m_phrase;
    utf16_t * phrase_rhs = (utf16_t *) rhs.m_phrase;

    return memcmp(phrase_lhs, phrase_rhs, sizeof(utf16_t) * phrase_length);
}

template<size_t phrase_length>
static bool phrase_less_than(const PhraseIndexItem<phrase_length> & lhs,
                             const PhraseIndexItem<phrase_length> & rhs){
    return 0 > phrase_compare(lhs, rhs);
}

PhraseBitmapIndexLevel::PhraseBitmapIndexLevel(){
    memset(m_phrase_length_indexes, 0, sizeof(m_phrase_length_indexes));
}

void PhraseBitmapIndexLevel::reset(){
    for ( size_t i = 0; i < PHRASE_Number_Of_Bitmap_Index; i++){
        PhraseLengthIndexLevel * length_array =
            m_phrase_length_indexes[i];
        if ( length_array )
            delete length_array;
    }
}

int PhraseBitmapIndexLevel::search( int phrase_length, /* in */ utf16_t phrase[], /* out */ phrase_token_t & token){
    assert(phrase_length > 0);

    int result = SEARCH_NONE;
    utf16_t first_key = phrase[0];

    PhraseLengthIndexLevel * phrase_array = m_phrase_length_indexes[first_key];
    if ( phrase_array )
        return phrase_array->search(phrase_length - 1, phrase + 1, token);
    return result;
}

PhraseLengthIndexLevel::PhraseLengthIndexLevel(){
    m_phrase_array_indexes = g_array_new(FALSE, TRUE, sizeof(void *));
}

PhraseLengthIndexLevel::~PhraseLengthIndexLevel(){
#define CASE(x) case x:                                                 \
    {                                                                   \
        PhraseArrayIndexLevel<x> * array =  g_array_index               \
            (m_phrase_array_indexes, PhraseArrayIndexLevel<x> *, x);    \
        if ( array )                                                    \
            delete array;                                               \
        break;                                                          \
    }

    for ( size_t i = 0 ; i < m_phrase_array_indexes->len; ++i){
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
    g_array_free(m_phrase_array_indexes, TRUE);
#undef CASE
}

int PhraseLengthIndexLevel::search(int phrase_length,
                                   /* in */ utf16_t phrase[],
                                   /* out */ phrase_token_t & token){
    int result = SEARCH_NONE;
    if(m_phrase_array_indexes->len < phrase_length + 1)
        return result;
    if (m_phrase_array_indexes->len > phrase_length + 1)
        result |= SEARCH_CONTINUED;

#define CASE(len) case len:                                             \
    {                                                                   \
        PhraseArrayIndexLevel<len> * array = g_array_index              \
            (m_phrase_array_indexes, PhraseArrayIndexLevel<len> *, len); \
        if ( !array )                                                   \
            return result;                                              \
        result |= array->search(phrase, token);                         \
        return result;                                                  \
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
int PhraseArrayIndexLevel<phrase_length>::search(/* in */ utf16_t phrase[], /* out */ phrase_token_t & token){
    PhraseIndexItem<phrase_length> * chunk_begin, * chunk_end;
    chunk_begin = (PhraseIndexItem<phrase_length> *)m_chunk.begin();
    chunk_end = (PhraseIndexItem<phrase_length> *)m_chunk.end();
    PhraseIndexItem<phrase_length> search_elem(phrase, -1);

    //do the search
    std_lite::pair<PhraseIndexItem<phrase_length> *, PhraseIndexItem<phrase_length> *> range;
    range = std_lite::equal_range(chunk_begin, chunk_end, search_elem, phrase_less_than<phrase_length>);

    if ( range.first == range.second )
        return SEARCH_NONE;

    assert(range.second - range.first == 1);
    token = range.first->m_token;
    return SEARCH_OK;
}

int PhraseBitmapIndexLevel::add_index( int phrase_length, /* in */ utf16_t phrase[], /* in */ phrase_token_t token){
    utf16_t first_key =  phrase[0];
    PhraseLengthIndexLevel * & length_array = m_phrase_length_indexes[first_key];
    if ( !length_array ){
        length_array = new PhraseLengthIndexLevel();
    }
    return length_array->add_index(phrase_length - 1, phrase + 1, token);
}

int PhraseBitmapIndexLevel::remove_index( int phrase_length, /* in */ utf16_t phrase[], /* out */ phrase_token_t & token){
    utf16_t first_key = phrase[0];
    PhraseLengthIndexLevel * &length_array = m_phrase_length_indexes[first_key];
    if ( length_array )
        return length_array->remove_index(phrase_length - 1, phrase + 1, token);
    return REMOVE_ITEM_DONOT_EXISTS;
}

int PhraseLengthIndexLevel::add_index( int phrase_length, /* in */ utf16_t phrase[], /* in */ phrase_token_t token){
    assert(phrase_length + 1 < MAX_PHRASE_LENGTH);
    if ( m_phrase_array_indexes -> len <= phrase_length )
        g_array_set_size(m_phrase_array_indexes, phrase_length + 1);

#define CASE(len) case len:                                             \
    {                                                                   \
        PhraseArrayIndexLevel<len> * &array = g_array_index             \
            (m_phrase_array_indexes, PhraseArrayIndexLevel<len> *, len); \
        if ( !array )                                                   \
            array = new PhraseArrayIndexLevel<len>;                     \
        return array->add_index(phrase, token);                         \
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

int PhraseLengthIndexLevel::remove_index( int phrase_length, /* in */ utf16_t phrase[], /* out */ phrase_token_t & token){
    assert(phrase_length + 1 < MAX_PHRASE_LENGTH);
    if ( m_phrase_array_indexes -> len <= phrase_length )
        return REMOVE_ITEM_DONOT_EXISTS;
#define CASE(len) case len:                                             \
    {                                                                   \
        PhraseArrayIndexLevel<len> * &array =  g_array_index            \
            (m_phrase_array_indexes, PhraseArrayIndexLevel<len> *, len); \
        if ( !array )                                                   \
            return REMOVE_ITEM_DONOT_EXISTS;                            \
        return array->remove_index(phrase, token);                      \
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
int PhraseArrayIndexLevel<phrase_length>::add_index(/* in */ utf16_t phrase[], /* in */ phrase_token_t token){
    PhraseIndexItem<phrase_length> * buf_begin, * buf_end;

    PhraseIndexItem<phrase_length> new_elem(phrase, token);
    buf_begin = (PhraseIndexItem<phrase_length> *) m_chunk.begin();
    buf_end = (PhraseIndexItem<phrase_length> *) m_chunk.end();

    std_lite::pair<PhraseIndexItem<phrase_length> *, PhraseIndexItem<phrase_length> *> range;
    range = std_lite::equal_range(buf_begin, buf_end, new_elem, phrase_less_than<phrase_length>);

    assert(range.second - range.first <= 1);
    if ( range.second - range.first == 1 )
        return INSERT_ITEM_EXISTS;

    PhraseIndexItem<phrase_length> * cur_elem = range.first;
    int offset = (cur_elem - buf_begin) *
        sizeof(PhraseIndexItem<phrase_length>);
    m_chunk.insert_content(offset, &new_elem,
                           sizeof(PhraseIndexItem<phrase_length> ));
    return INSERT_OK;
}

template<size_t phrase_length>
int PhraseArrayIndexLevel<phrase_length>::remove_index(/* in */ utf16_t phrase[], /* out */ phrase_token_t & token){
    PhraseIndexItem<phrase_length> * buf_begin, * buf_end;

    PhraseIndexItem<phrase_length> remove_elem(phrase, -1);
    buf_begin = (PhraseIndexItem<phrase_length> *) m_chunk.begin();
    buf_end = (PhraseIndexItem<phrase_length> *) m_chunk.end();

    std_lite::pair<PhraseIndexItem<phrase_length> *, PhraseIndexItem<phrase_length> *> range;
    range = std_lite::equal_range(buf_begin, buf_end, remove_elem, phrase_less_than<phrase_length>);

    assert(range.second - range.first <= 1);
    PhraseIndexItem<phrase_length> * cur_elem = range.first;
    if ( range.first == range.second || cur_elem == buf_end)
        return REMOVE_ITEM_DONOT_EXISTS;

    token = cur_elem->m_token;
    int offset = (cur_elem -  buf_begin) *
        sizeof(PhraseIndexItem<phrase_length>);
    m_chunk.remove_content(offset, sizeof (PhraseIndexItem<phrase_length>));
    return REMOVE_OK;
}

bool PhraseLargeTable::load_text(FILE * infile){
    char pinyin[256];
    char phrase[256];
    phrase_token_t token;
    size_t freq;

    while ( !feof(infile) ) {
        fscanf(infile, "%s", pinyin);
        fscanf(infile, "%s", phrase);
        fscanf(infile, "%u", &token);
        fscanf(infile, "%ld", &freq);

        if ( feof(infile) )
            break;

        glong phrase_len = g_utf8_strlen(phrase, -1);
        utf16_t * new_phrase = g_utf8_to_utf16(phrase, -1, NULL, NULL, NULL);
        add_index(phrase_len, new_phrase, token);

        g_free(new_phrase);
    }
    return true;
}

bool PhraseBitmapIndexLevel::load(MemoryChunk * chunk, table_offset_t offset,
                                  table_offset_t end){
    reset();
    char * buf_begin = (char *) chunk->begin();
    table_offset_t phrase_begin, phrase_end;
    table_offset_t * index = (table_offset_t *) (buf_begin + offset);
    phrase_end = *index;

    for ( size_t i = 0; i < PHRASE_Number_Of_Bitmap_Index; ++i) {
        phrase_begin = phrase_end;
        index++;
        phrase_end = *index;
        if ( phrase_begin == phrase_end ) //null pointer
            continue;
        PhraseLengthIndexLevel * phrases = new PhraseLengthIndexLevel;
        m_phrase_length_indexes[i] = phrases;
        phrases->load(chunk, phrase_begin, phrase_end - 1);
        assert( phrase_end <= end );
        assert( *(buf_begin + phrase_end - 1) == c_separate);
    }
    offset += (PHRASE_Number_Of_Bitmap_Index + 1) * sizeof(table_offset_t);
    assert( c_separate == *(buf_begin + offset) );
    return true;
}

bool PhraseBitmapIndexLevel::store(MemoryChunk * new_chunk,
                                   table_offset_t offset,
                                   table_offset_t & end){
    table_offset_t phrase_end;
    table_offset_t index = offset;
    offset += (PHRASE_Number_Of_Bitmap_Index + 1) * sizeof(table_offset_t);
    //add '#'
    new_chunk->set_content(offset, &c_separate, sizeof(char));
    offset +=sizeof(char);
    new_chunk->set_content(index, &offset, sizeof(table_offset_t));
    index += sizeof(table_offset_t);
    for ( size_t i = 0; i < PHRASE_Number_Of_Bitmap_Index; ++i) {
        PhraseLengthIndexLevel * phrases = m_phrase_length_indexes[i];
        if ( !phrases ) { //null pointer
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

bool PhraseLengthIndexLevel::load(MemoryChunk * chunk, table_offset_t offset, table_offset_t end){
    char * buf_begin = (char *) chunk->begin();
    guint32 nindex = *((guint32 *)(buf_begin + offset));
    table_offset_t * index = (table_offset_t *)
        (buf_begin + offset + sizeof(guint32));

    table_offset_t phrase_begin, phrase_end = *index;
    m_phrase_array_indexes = g_array_new(FALSE, TRUE, sizeof(void *));
    for ( size_t i = 0; i < nindex; ++i) {
        phrase_begin = phrase_end;
        index++;
        phrase_end = *index;
        if ( phrase_begin == phrase_end ){
            void * null = NULL;
            g_array_append_val(m_phrase_array_indexes, null);
            continue;
        }

#define CASE(len) case len:                                             \
        {                                                               \
            PhraseArrayIndexLevel<len> * phrase = new PhraseArrayIndexLevel<len>; \
            phrase->load(chunk, phrase_begin, phrase_end - 1);          \
            assert( *(buf_begin + phrase_end - 1) == c_separate);       \
            assert( phrase_end <= end );                                \
            g_array_append_val(m_phrase_array_indexes, phrase);         \
            break;                                                      \
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

bool PhraseLengthIndexLevel::store(MemoryChunk * new_chunk, table_offset_t offset, table_offset_t & end) {
    guint32 nindex = m_phrase_array_indexes->len;
    new_chunk->set_content(offset, &nindex, sizeof(guint32));
    table_offset_t index = offset + sizeof(guint32);

    offset += sizeof(guint32) + (nindex + 1) * sizeof(table_offset_t);
    new_chunk->set_content(offset, &c_separate, sizeof(char));
    offset += sizeof(char);
    new_chunk->set_content(index, &offset, sizeof(table_offset_t));
    index += sizeof(table_offset_t);

    table_offset_t phrase_end;
    for ( size_t i = 0; i < m_phrase_array_indexes->len; ++i) {
#define CASE(len) case len:                                             \
        {                                                               \
            PhraseArrayIndexLevel<len> * phrase = g_array_index         \
                (m_phrase_array_indexes, PhraseArrayIndexLevel<len> *, i); \
            if ( !phrase ){                                             \
                new_chunk->set_content                                  \
                    (index, &offset, sizeof(table_offset_t));           \
                index += sizeof(table_offset_t);                        \
                continue;                                               \
            }                                                           \
            phrase->store(new_chunk, offset, phrase_end);               \
            offset = phrase_end;                                        \
            break;                                                      \
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
        //add '#'
        new_chunk->set_content(offset, &c_separate, sizeof(char));
        offset += sizeof(char);
        new_chunk->set_content(index, &offset, sizeof(table_offset_t));
        index += sizeof(table_offset_t);

#undef CASE
    }
    end = offset;
    return true;
}

template<size_t phrase_length>
bool PhraseArrayIndexLevel<phrase_length>::
load(MemoryChunk * chunk, table_offset_t offset, table_offset_t end){
    char * buf_begin = (char *) chunk->begin();
    m_chunk.set_chunk(buf_begin + offset, end - offset, NULL);
    return true;
}

template<size_t phrase_length>
bool PhraseArrayIndexLevel<phrase_length>::
store(MemoryChunk * new_chunk, table_offset_t offset, table_offset_t & end) {
    new_chunk->set_content(offset, m_chunk.begin(), m_chunk.size());
    end = offset + m_chunk.size();
    return true;
}
