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

#include <assert.h>
#include <string.h>
#include "phrase_large_table2.h"


/* class definition */

namespace pinyin{

class PhraseLengthIndexLevel2{
protected:
    GArray * m_phrase_array_indexes;
public:
    PhraseLengthIndexLevel2();
    ~PhraseLengthIndexLevel2();

    /* load/store method */
    bool load(MemoryChunk * chunk, table_offset_t offset, table_offset_t end);
    bool store(MemoryChunk * new_chunk, table_offset_t offset, table_offset_t & end);

    /* search method */
    int search(int phrase_length, /* in */ const ucs4_t phrase[],
               /* out */ PhraseTokens tokens) const;

    /* add_index/remove_index method */
    int add_index(int phrase_length, /* in */ const ucs4_t phrase[],
                  /* in */ phrase_token_t token);
    int remove_index(int phrase_length, /* in */ const ucs4_t phrase[],
                     /* in */ phrase_token_t token);

    /* get length method */
    int get_length() const;

    /* mask out method */
    bool mask_out(phrase_token_t mask, phrase_token_t value);
};


template<size_t phrase_length>
struct PhraseIndexItem2{
    phrase_token_t m_token;
    ucs4_t m_phrase[phrase_length];
public:
    PhraseIndexItem2<phrase_length>(const ucs4_t phrase[], phrase_token_t token){
        memmove(m_phrase, phrase, sizeof(ucs4_t) * phrase_length);
        m_token = token;
    }
};


template<size_t phrase_length>
class PhraseArrayIndexLevel2{
protected:
    typedef PhraseIndexItem2<phrase_length> IndexItem;

protected:
    MemoryChunk m_chunk;
public:
    bool load(MemoryChunk * chunk, table_offset_t offset, table_offset_t end);
    bool store(MemoryChunk * new_chunk, table_offset_t offset, table_offset_t & end);

    /* search method */
    int search(/* in */ const ucs4_t phrase[], /* out */ PhraseTokens tokens) const;

    /* add_index/remove_index method */
    int add_index(/* in */ const ucs4_t phrase[], /* in */ phrase_token_t token);
    int remove_index(/* in */ const ucs4_t phrase[], /* in */ phrase_token_t token);

    /* get length method */
    int get_length() const;

    /* mask out method */
    bool mask_out(phrase_token_t mask, phrase_token_t value);
};

};

using namespace pinyin;

/* class implementation */

template<size_t phrase_length>
static int phrase_compare2(const PhraseIndexItem2<phrase_length> &lhs,
                           const PhraseIndexItem2<phrase_length> &rhs){
    ucs4_t * phrase_lhs = (ucs4_t *) lhs.m_phrase;
    ucs4_t * phrase_rhs = (ucs4_t *) rhs.m_phrase;

    return memcmp(phrase_lhs, phrase_rhs, sizeof(ucs4_t) * phrase_length);
}

template<size_t phrase_length>
static bool phrase_less_than2(const PhraseIndexItem2<phrase_length> & lhs,
                              const PhraseIndexItem2<phrase_length> & rhs){
    return 0 > phrase_compare2(lhs, rhs);
}

PhraseBitmapIndexLevel2::PhraseBitmapIndexLevel2(){
    memset(m_phrase_length_indexes, 0, sizeof(m_phrase_length_indexes));
}

void PhraseBitmapIndexLevel2::reset(){
    for ( size_t i = 0; i < PHRASE_NUMBER_OF_BITMAP_INDEX; i++){
        PhraseLengthIndexLevel2 * & length_array =
            m_phrase_length_indexes[i];
        if ( length_array )
            delete length_array;
        length_array = NULL;
    }
}


/* search method */

int PhraseBitmapIndexLevel2::search(int phrase_length,
                                    /* in */ const ucs4_t phrase[],
                                    /* out */ PhraseTokens tokens) const {
    assert(phrase_length > 0);

    int result = SEARCH_NONE;
    /* use the first 8-bit of the lower 16-bit for bitmap index,
     * as most the higher 16-bit are zero.
     */
    guint8 first_key = (phrase[0] & 0xFF00) >> 8;

    PhraseLengthIndexLevel2 * phrase_array = m_phrase_length_indexes[first_key];
    if ( phrase_array )
        return phrase_array->search(phrase_length, phrase, tokens);
    return result;
}

PhraseLengthIndexLevel2::PhraseLengthIndexLevel2(){
    m_phrase_array_indexes = g_array_new(FALSE, TRUE, sizeof(void *));
}

PhraseLengthIndexLevel2::~PhraseLengthIndexLevel2(){
#define CASE(len) case len:                                             \
    {                                                                   \
        PhraseArrayIndexLevel2<len> * & array = g_array_index           \
            (m_phrase_array_indexes,                                    \
             PhraseArrayIndexLevel2<len> *, len - 1);                   \
        if ( array ) {                                                  \
            delete array;                                               \
            array = NULL;                                               \
        }                                                               \
        break;                                                          \
    }

    for (size_t i = 1; i <= m_phrase_array_indexes->len; ++i){
        switch (i){
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
	    CASE(16);
	default:
	    assert(false);
        }
    }
    g_array_free(m_phrase_array_indexes, TRUE);
#undef CASE
}

int PhraseLengthIndexLevel2::search(int phrase_length,
                                    /* in */ const ucs4_t phrase[],
                                    /* out */ PhraseTokens tokens) const {
    int result = SEARCH_NONE;
    if ((int) m_phrase_array_indexes->len < phrase_length)
        return result;
    if ((int) m_phrase_array_indexes->len > phrase_length)
        result |= SEARCH_CONTINUED;

#define CASE(len) case len:                                             \
    {                                                                   \
        PhraseArrayIndexLevel2<len> * array = g_array_index             \
            (m_phrase_array_indexes, PhraseArrayIndexLevel2<len> *, len - 1); \
        if ( !array )                                                   \
            return result;                                              \
        result |= array->search(phrase, tokens);                        \
        return result;                                                  \
    }

    switch ( phrase_length ){
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
	CASE(16);
    default:
	assert(false);
    }
#undef CASE
}

template<size_t phrase_length>
int PhraseArrayIndexLevel2<phrase_length>::search
(/* in */ const ucs4_t phrase[], /* out */ PhraseTokens tokens) const {
    int result = SEARCH_NONE;

    IndexItem * chunk_begin = NULL, * chunk_end = NULL;
    chunk_begin = (IndexItem *) m_chunk.begin();
    chunk_end = (IndexItem *) m_chunk.end();

    /* do the search */
    IndexItem search_elem(phrase, -1);
    std_lite::pair<IndexItem *, IndexItem *> range;
    range = std_lite::equal_range
        (chunk_begin, chunk_end, search_elem,
         phrase_less_than2<phrase_length>);

    const IndexItem * const begin = range.first;
    const IndexItem * const end = range.second;
    if (begin == end)
        return result;

    const IndexItem * iter = NULL;
    GArray * array = NULL;

    for (iter = begin; iter != end; ++iter) {
        phrase_token_t token = iter->m_token;

        /* filter out disabled sub phrase indices. */
        array = tokens[PHRASE_INDEX_LIBRARY_INDEX(token)];
        if (NULL == array)
            continue;

        result |= SEARCH_OK;

        g_array_append_val(array, token);
    }

    return result;
}


/* add/remove index method */

int PhraseBitmapIndexLevel2::add_index(int phrase_length,
                                       /* in */ const ucs4_t phrase[],
                                       /* in */ phrase_token_t token){
    guint8 first_key =  (phrase[0] & 0xFF00) >> 8;

    PhraseLengthIndexLevel2 * & length_array =
        m_phrase_length_indexes[first_key];

    if ( !length_array ){
        length_array = new PhraseLengthIndexLevel2();
    }
    return length_array->add_index(phrase_length, phrase, token);
}

int PhraseBitmapIndexLevel2::remove_index(int phrase_length,
                                         /* in */ const ucs4_t phrase[],
                                         /* in */ phrase_token_t token){
    guint8 first_key = (phrase[0] & 0xFF00) >> 8;

    PhraseLengthIndexLevel2 * & length_array =
        m_phrase_length_indexes[first_key];

    if (NULL == length_array)
        return ERROR_REMOVE_ITEM_DONOT_EXISTS;

    int retval = length_array->remove_index(phrase_length, phrase, token);

    /* remove empty array. */
    if (0 == length_array->get_length()) {
        delete length_array;
        length_array = NULL;
    }

    return retval;
}

int PhraseLengthIndexLevel2::add_index(int phrase_length,
                                       /* in */ const ucs4_t phrase[],
                                       /* in */ phrase_token_t token) {
    if (phrase_length >= MAX_PHRASE_LENGTH)
        return ERROR_PHRASE_TOO_LONG;

    if ((int) m_phrase_array_indexes->len < phrase_length)
        g_array_set_size(m_phrase_array_indexes, phrase_length);

#define CASE(len) case len:                                             \
    {                                                                   \
        PhraseArrayIndexLevel2<len> * & array = g_array_index           \
            (m_phrase_array_indexes, PhraseArrayIndexLevel2<len> *, len - 1); \
        if ( !array )                                                   \
            array = new PhraseArrayIndexLevel2<len>;                    \
        return array->add_index(phrase, token);                         \
    }

    switch(phrase_length){
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
        CASE(16);
    default:
	assert(false);
    }

#undef CASE
}

int PhraseLengthIndexLevel2::remove_index(int phrase_length,
                                          /* in */ const ucs4_t phrase[],
                                          /* in */ phrase_token_t token) {
    if (phrase_length >= MAX_PHRASE_LENGTH)
        return ERROR_PHRASE_TOO_LONG;

    if ((int) m_phrase_array_indexes->len < phrase_length)
        return ERROR_REMOVE_ITEM_DONOT_EXISTS;

#define CASE(len) case len:                                             \
    {                                                                   \
        PhraseArrayIndexLevel2<len> * & array = g_array_index           \
            (m_phrase_array_indexes,                                    \
             PhraseArrayIndexLevel2<len> *, len - 1);                   \
        if (NULL == array)                                              \
            return ERROR_REMOVE_ITEM_DONOT_EXISTS;                      \
        int retval = array->remove_index(phrase, token);                \
                                                                        \
        /* remove empty array. */                                       \
        if (0 == array->get_length()) {                                 \
            delete array;                                               \
            array = NULL;                                               \
                                                                        \
            /* shrink self array. */                                    \
            g_array_set_size(m_phrase_array_indexes,                    \
                             get_length());                             \
        }                                                               \
        return retval;                                                  \
    }

    switch(phrase_length){
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
	CASE(16);
    default:
	assert(false);
    }
#undef CASE
}

template<size_t phrase_length>
int PhraseArrayIndexLevel2<phrase_length>::add_index
(/* in */ const ucs4_t phrase[], /* in */ phrase_token_t token){
    IndexItem * begin, * end;

    IndexItem add_elem(phrase, token);
    begin = (IndexItem *) m_chunk.begin();
    end   = (IndexItem *) m_chunk.end();

    std_lite::pair<IndexItem *, IndexItem *> range;
    range = std_lite::equal_range
        (begin, end, add_elem, phrase_less_than2<phrase_length>);

    IndexItem * cur_elem;
    for (cur_elem = range.first;
         cur_elem != range.second; ++cur_elem) {
        if (cur_elem->m_token == token)
            return ERROR_INSERT_ITEM_EXISTS;
        if (cur_elem->m_token > token)
            break;
    }

    int offset = (cur_elem - begin) * sizeof(IndexItem);
    m_chunk.insert_content(offset, &add_elem, sizeof(IndexItem));
    return ERROR_OK;
}

template<size_t phrase_length>
int PhraseArrayIndexLevel2<phrase_length>::remove_index
(/* in */ const ucs4_t phrase[], /* in */ phrase_token_t token) {
    IndexItem * begin, * end;

    IndexItem remove_elem(phrase, token);
    begin = (IndexItem *) m_chunk.begin();
    end   = (IndexItem *) m_chunk.end();

    std_lite::pair<IndexItem *, IndexItem *> range;
    range = std_lite::equal_range
        (begin, end, remove_elem, phrase_less_than2<phrase_length>);

    IndexItem * cur_elem;
    for (cur_elem = range.first;
         cur_elem != range.second; ++cur_elem) {
        if (cur_elem->m_token == token)
            break;
    }

    if (cur_elem == range.second)
        return ERROR_REMOVE_ITEM_DONOT_EXISTS;

    int offset = (cur_elem - begin) * sizeof(IndexItem);
    m_chunk.remove_content(offset, sizeof(IndexItem));
    return ERROR_OK;
}


/* load text method */

bool PhraseLargeTable2::load_text(FILE * infile){
    char pinyin[256];
    char phrase[256];
    phrase_token_t token;
    size_t freq;

    while (!feof(infile)) {
#ifdef __APPLE__
        int num = fscanf(infile, "%255s %255[^ \t] %u %ld",
                         pinyin, phrase, &token, &freq);
#else
        int num = fscanf(infile, "%255s %255s %u %ld",
                         pinyin, phrase, &token, &freq);
#endif

        if (4 != num)
            continue;

        if (feof(infile))
            break;

        glong phrase_len = g_utf8_strlen(phrase, -1);
        ucs4_t * new_phrase = g_utf8_to_ucs4(phrase, -1, NULL, NULL, NULL);
        add_index(phrase_len, new_phrase, token);

        g_free(new_phrase);
    }
    return true;
}


/* load/store method */

bool PhraseBitmapIndexLevel2::load(MemoryChunk * chunk,
                                   table_offset_t offset,
                                   table_offset_t end){
    reset();
    char * buf_begin = (char *) chunk->begin();
    table_offset_t phrase_begin, phrase_end;
    table_offset_t * index = (table_offset_t *) (buf_begin + offset);
    phrase_end = *index;

    for ( size_t i = 0; i < PHRASE_NUMBER_OF_BITMAP_INDEX; ++i) {
        phrase_begin = phrase_end;
        index++;
        phrase_end = *index;
        if ( phrase_begin == phrase_end ) //null pointer
            continue;

        /* after reset() all phrases are null pointer. */
        PhraseLengthIndexLevel2 * phrases = new PhraseLengthIndexLevel2;
        m_phrase_length_indexes[i] = phrases;

        phrases->load(chunk, phrase_begin, phrase_end - 1);
        assert( phrase_end <= end );
        assert( *(buf_begin + phrase_end - 1) == c_separate);
    }
    offset += (PHRASE_NUMBER_OF_BITMAP_INDEX + 1) * sizeof(table_offset_t);
    assert( c_separate == *(buf_begin + offset) );
    return true;
}

bool PhraseBitmapIndexLevel2::store(MemoryChunk * new_chunk,
                                    table_offset_t offset,
                                    table_offset_t & end){
    table_offset_t phrase_end;
    table_offset_t index = offset;
    offset += (PHRASE_NUMBER_OF_BITMAP_INDEX + 1) * sizeof(table_offset_t);
    //add '#'
    new_chunk->set_content(offset, &c_separate, sizeof(char));
    offset +=sizeof(char);
    new_chunk->set_content(index, &offset, sizeof(table_offset_t));
    index += sizeof(table_offset_t);
    for ( size_t i = 0; i < PHRASE_NUMBER_OF_BITMAP_INDEX; ++i) {
        PhraseLengthIndexLevel2 * phrases = m_phrase_length_indexes[i];
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

bool PhraseLengthIndexLevel2::load(MemoryChunk * chunk,
                                   table_offset_t offset,
                                   table_offset_t end) {
    char * buf_begin = (char *) chunk->begin();
    guint32 nindex = *((guint32 *)(buf_begin + offset));
    table_offset_t * index = (table_offset_t *)
        (buf_begin + offset + sizeof(guint32));

    table_offset_t phrase_begin, phrase_end = *index;
    g_array_set_size(m_phrase_array_indexes, 0);
    for (size_t i = 1; i <= nindex; ++i) {
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
            PhraseArrayIndexLevel2<len> * phrase =                      \
                new PhraseArrayIndexLevel2<len>;                        \
            phrase->load(chunk, phrase_begin, phrase_end - 1);          \
            assert( *(buf_begin + phrase_end - 1) == c_separate );      \
            assert( phrase_end <= end );                                \
            g_array_append_val(m_phrase_array_indexes, phrase);         \
            break;                                                      \
        }
        switch ( i ){
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
	    CASE(16);
	default:
	    assert(false);
        }
#undef CASE
    }
    offset += sizeof(guint32) + (nindex + 1) * sizeof(table_offset_t);
    assert ( c_separate == * (buf_begin + offset) );
    return true;
}

bool PhraseLengthIndexLevel2::store(MemoryChunk * new_chunk,
                                    table_offset_t offset,
                                    table_offset_t & end) {
    guint32 nindex = m_phrase_array_indexes->len;
    new_chunk->set_content(offset, &nindex, sizeof(guint32));
    table_offset_t index = offset + sizeof(guint32);

    offset += sizeof(guint32) + (nindex + 1) * sizeof(table_offset_t);
    new_chunk->set_content(offset, &c_separate, sizeof(char));
    offset += sizeof(char);
    new_chunk->set_content(index, &offset, sizeof(table_offset_t));
    index += sizeof(table_offset_t);

    table_offset_t phrase_end;
    for (size_t i = 1; i <= m_phrase_array_indexes->len; ++i) {
#define CASE(len) case len:                                             \
        {                                                               \
            PhraseArrayIndexLevel2<len> * phrase = g_array_index        \
                (m_phrase_array_indexes, PhraseArrayIndexLevel2<len> *, len - 1); \
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
	    CASE(16);
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
bool PhraseArrayIndexLevel2<phrase_length>::
load(MemoryChunk * chunk, table_offset_t offset, table_offset_t end){
    char * buf_begin = (char *) chunk->begin();
    m_chunk.set_chunk(buf_begin + offset, end - offset, NULL);
    return true;
}

template<size_t phrase_length>
bool PhraseArrayIndexLevel2<phrase_length>::
store(MemoryChunk * new_chunk, table_offset_t offset, table_offset_t & end) {
    new_chunk->set_content(offset, m_chunk.begin(), m_chunk.size());
    end = offset + m_chunk.size();
    return true;
}


/* get length method */

int PhraseLengthIndexLevel2::get_length() const {
    int length = m_phrase_array_indexes->len;

    /* trim trailing zero. */
    for (int i = length - 1; i >= 0; --i) {
        void * array = g_array_index(m_phrase_array_indexes, void *, i);

        if (NULL != array)
            break;

        --length;
    }

    return length;
}

template<size_t phrase_length>
int PhraseArrayIndexLevel2<phrase_length>::get_length() const {
    IndexItem * chunk_begin = NULL, * chunk_end = NULL;
    chunk_begin = (IndexItem *) m_chunk.begin();
    chunk_end = (IndexItem *) m_chunk.end();

    return chunk_end - chunk_begin;
}


/* mask out method */

bool PhraseBitmapIndexLevel2::mask_out(phrase_token_t mask,
                                       phrase_token_t value){
    for (size_t i = 0; i < PHRASE_NUMBER_OF_BITMAP_INDEX; ++i) {
        PhraseLengthIndexLevel2 * & length_array =
            m_phrase_length_indexes[i];

        if (NULL == length_array)
            continue;

        length_array->mask_out(mask, value);

        if (0 == length_array->get_length()) {
            delete length_array;
            length_array = NULL;
        }
    }

    return true;
}

bool PhraseLengthIndexLevel2::mask_out(phrase_token_t mask,
                                       phrase_token_t value){
#define CASE(len) case len:                                     \
    {                                                           \
        PhraseArrayIndexLevel2<len> * & array = g_array_index   \
            (m_phrase_array_indexes,                            \
             PhraseArrayIndexLevel2<len> *, len - 1);           \
                                                                \
        if (NULL == array)                                      \
            continue;                                           \
                                                                \
        array->mask_out(mask, value);                           \
                                                                \
        if (0 == array->get_length()) {                         \
            delete array;                                       \
            array = NULL;                                       \
        }                                                       \
        break;                                                  \
    }

    for (size_t i = 1; i <= m_phrase_array_indexes->len; ++i) {
        switch (i) {
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
	    CASE(16);
	default:
	    assert(false);
        }
    }
    /* shrink self array. */
    g_array_set_size(m_phrase_array_indexes, get_length());
#undef CASE
    return true;
}

template<size_t phrase_length>
bool PhraseArrayIndexLevel2<phrase_length>::mask_out
(phrase_token_t mask, phrase_token_t value) {
    IndexItem * begin = NULL, * end = NULL;
    begin = (IndexItem *) m_chunk.begin();
    end = (IndexItem *) m_chunk.end();

    for (IndexItem * cur = begin; cur != end; ++cur) {
        if ((cur->m_token & mask) != value)
            continue;

        int offset = (cur - begin) * sizeof(IndexItem);
        m_chunk.remove_content(offset, sizeof(IndexItem));

        /* update chunk end. */
        end = (IndexItem *) m_chunk.end();
        --cur;
    }

    return true;
}
