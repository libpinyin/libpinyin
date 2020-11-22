/* 
 *  libpinyin
 *  Library to deal with pinyin.
 *  
 *  Copyright (C) 2011 Peng Wu <alexepico@gmail.com>
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

#include "chewing_large_table.h"
#include <assert.h>
#include "pinyin_phrase2.h"
#include "pinyin_phrase3.h"
#include "pinyin_parser2.h"
#include "zhuyin_parser2.h"


/* internal class definition */

namespace pinyin{
class ChewingLengthIndexLevel{

protected:
    GArray * m_chewing_array_indexes;

public:
    /* constructor/destructor */
    ChewingLengthIndexLevel();
    ~ChewingLengthIndexLevel();

    /* load/store method */
    bool load(MemoryChunk * chunk, table_offset_t offset, table_offset_t end);
    bool store(MemoryChunk * new_chunk, table_offset_t offset,
               table_offset_t & end);

    /* search method */
    int search(pinyin_option_t options, int phrase_length,
               /* in */ const ChewingKey keys[],
               /* out */ PhraseIndexRanges ranges) const;

    /* add/remove index method */
    int add_index(int phrase_length, /* in */ const ChewingKey keys[],
                  /* in */ phrase_token_t token);
    int remove_index(int phrase_length, /* in */ const ChewingKey keys[],
                     /* in */ phrase_token_t token);

    /* get length method */
    int get_length() const;

    /* mask out method */
    bool mask_out(phrase_token_t mask, phrase_token_t value);
};


template<size_t phrase_length>
class ChewingArrayIndexLevel{
protected:
    typedef PinyinIndexItem2<phrase_length> IndexItem;

protected:
    MemoryChunk m_chunk;

    /* compress consecutive tokens */
    int convert(pinyin_option_t options,
                const ChewingKey keys[],
                IndexItem * begin,
                IndexItem * end,
                PhraseIndexRanges ranges) const;

public:
    /* load/store method */
    bool load(MemoryChunk * chunk, table_offset_t offset, table_offset_t end);
    bool store(MemoryChunk * new_chunk, table_offset_t offset,
               table_offset_t & end);

    /* search method */
    int search(pinyin_option_t options, /* in */const ChewingKey keys[],
               /* out */ PhraseIndexRanges ranges) const;

    /* add/remove index method */
    int add_index(/* in */ const ChewingKey keys[], /* in */ phrase_token_t token);
    int remove_index(/* in */ const ChewingKey keys[],
                     /* in */ phrase_token_t token);

    /* get length method */
    int get_length() const;

    /* mask out method */
    bool mask_out(phrase_token_t mask, phrase_token_t value);
};

};


using namespace pinyin;

/* class implementation */

ChewingBitmapIndexLevel::ChewingBitmapIndexLevel(pinyin_option_t options)
    : m_options(options) {
    memset(m_chewing_length_indexes, 0, sizeof(m_chewing_length_indexes));
}

void ChewingBitmapIndexLevel::reset() {
    for (int k = CHEWING_ZERO_INITIAL; k < CHEWING_NUMBER_OF_INITIALS; ++k)
        for (int l = CHEWING_ZERO_MIDDLE; l < CHEWING_NUMBER_OF_MIDDLES; ++l)
            for (int m = CHEWING_ZERO_FINAL; m < CHEWING_NUMBER_OF_FINALS; ++m)
                for (int n = CHEWING_ZERO_TONE; n < CHEWING_NUMBER_OF_TONES;
                     ++n) {
                    ChewingLengthIndexLevel * & length_array =
                        m_chewing_length_indexes[k][l][m][n];
                    if (length_array)
                        delete length_array;
                    length_array = NULL;
                }
}


/* search method */

int ChewingBitmapIndexLevel::search(int phrase_length,
                                    /* in */ const ChewingKey keys[],
                                    /* out */ PhraseIndexRanges ranges) const {
    assert(phrase_length > 0);
    return initial_level_search(phrase_length, keys, ranges);
}

int ChewingBitmapIndexLevel::initial_level_search (int phrase_length,
    /* in */ const ChewingKey keys[], /* out */ PhraseIndexRanges ranges) const {

/* macros */
#define MATCH(AMBIGUITY, ORIGIN, ANOTHER) case ORIGIN:                  \
    {                                                                   \
        result |= middle_and_final_level_search(ORIGIN, phrase_length,  \
                                                keys, ranges);          \
        if (m_options & AMBIGUITY) {                                    \
            result |= middle_and_final_level_search(ANOTHER,            \
                                                    phrase_length,      \
                                                    keys, ranges);      \
        }                                                               \
        return result;                                                  \
    }

    /* deal with ambiguities */
    int result = SEARCH_NONE;
    const ChewingKey & first_key = keys[0];

    switch(first_key.m_initial) {
        MATCH(PINYIN_AMB_C_CH, CHEWING_C, CHEWING_CH);
        MATCH(PINYIN_AMB_C_CH, CHEWING_CH, CHEWING_C);
        MATCH(PINYIN_AMB_Z_ZH, CHEWING_Z, CHEWING_ZH);
        MATCH(PINYIN_AMB_Z_ZH, CHEWING_ZH, CHEWING_Z);
        MATCH(PINYIN_AMB_S_SH, CHEWING_S, CHEWING_SH);
        MATCH(PINYIN_AMB_S_SH, CHEWING_SH, CHEWING_S);
        MATCH(PINYIN_AMB_L_R, CHEWING_R, CHEWING_L);
        MATCH(PINYIN_AMB_L_N, CHEWING_N, CHEWING_L);
        MATCH(PINYIN_AMB_F_H, CHEWING_F, CHEWING_H);
        MATCH(PINYIN_AMB_F_H, CHEWING_H, CHEWING_F);
        MATCH(PINYIN_AMB_G_K, CHEWING_G, CHEWING_K);
        MATCH(PINYIN_AMB_G_K, CHEWING_K, CHEWING_G);

    case CHEWING_L:
        {
            result |= middle_and_final_level_search
                (CHEWING_L, phrase_length, keys, ranges);

            if (m_options & PINYIN_AMB_L_N)
                result |= middle_and_final_level_search
                    (CHEWING_N, phrase_length, keys,ranges);

            if (m_options & PINYIN_AMB_L_R)
                result |= middle_and_final_level_search
                    (CHEWING_R, phrase_length, keys, ranges);
            return result;
        }
    default:
        {
            result |= middle_and_final_level_search
                ((ChewingInitial) first_key.m_initial,
                 phrase_length, keys, ranges);
            return result;
        }
    }
#undef MATCH
    return result;
}


int ChewingBitmapIndexLevel::middle_and_final_level_search
(ChewingInitial initial, int phrase_length, /* in */ const ChewingKey keys[],
 /* out */ PhraseIndexRanges ranges) const {

/* macros */
#define MATCH(AMBIGUITY, ORIGIN, ANOTHER) case ORIGIN:                  \
    {                                                                   \
        result = tone_level_search                                      \
            (initial, middle,                                           \
             ORIGIN, phrase_length, keys, ranges);                      \
        if (m_options & AMBIGUITY) {                                    \
            result |= tone_level_search                                 \
                (initial, middle,                                       \
                 ANOTHER, phrase_length, keys, ranges);                 \
        }                                                               \
        return result;                                                  \
    }

    int result = SEARCH_NONE;
    const ChewingKey & first_key = keys[0];
    const ChewingMiddle middle = (ChewingMiddle)first_key.m_middle;

    switch(first_key.m_final) {
    case CHEWING_ZERO_FINAL:
        {
            if (middle == CHEWING_ZERO_MIDDLE) { /* in-complete pinyin */
                if (!(m_options & PINYIN_INCOMPLETE))
                    return result;
                for (int m = CHEWING_ZERO_MIDDLE;
                     m < CHEWING_NUMBER_OF_MIDDLES; ++m)
                    for (int n = CHEWING_ZERO_FINAL;
                         n < CHEWING_NUMBER_OF_FINALS; ++n) {

                        if (CHEWING_ZERO_MIDDLE == m &&
                            CHEWING_ZERO_FINAL == n)
                            continue;

                        result |= tone_level_search
                            (initial, (ChewingMiddle) m, (ChewingFinal) n,
                             phrase_length, keys, ranges);
                    }
                return result;
            } else { /* normal pinyin */
                result |= tone_level_search
                    (initial, middle, CHEWING_ZERO_FINAL,
                     phrase_length, keys, ranges);
                return result;
            }
        }

        MATCH(PINYIN_AMB_AN_ANG, CHEWING_AN, CHEWING_ANG);
	MATCH(PINYIN_AMB_AN_ANG, CHEWING_ANG, CHEWING_AN);
	MATCH(PINYIN_AMB_EN_ENG, CHEWING_EN, CHEWING_ENG);
	MATCH(PINYIN_AMB_EN_ENG, CHEWING_ENG, CHEWING_EN);
	MATCH(PINYIN_AMB_IN_ING, PINYIN_IN, PINYIN_ING);
	MATCH(PINYIN_AMB_IN_ING, PINYIN_ING, PINYIN_IN);

    default:
        {
            result |= tone_level_search
                (initial, middle, (ChewingFinal) first_key.m_final,
                 phrase_length, keys, ranges);
            return result;
        }
    }
#undef MATCH
    return result;
}


int ChewingBitmapIndexLevel::tone_level_search
(ChewingInitial initial, ChewingMiddle middle, ChewingFinal final,
 int phrase_length, /* in */ const ChewingKey keys[],
 /* out */ PhraseIndexRanges ranges) const {

    int result = SEARCH_NONE;
    const ChewingKey & first_key = keys[0];

    switch (first_key.m_tone) {
    case CHEWING_ZERO_TONE:
        {
            /* deal with zero tone in chewing large table. */
            for (int i = CHEWING_ZERO_TONE; i < CHEWING_NUMBER_OF_TONES; ++i) {
                ChewingLengthIndexLevel * phrases =
                    m_chewing_length_indexes
                    [initial][middle][final][(ChewingTone)i];
                if (phrases)
                    result |= phrases->search
                        (m_options, phrase_length - 1, keys + 1, ranges);
            }
            return result;
        }
    default:
        {
            ChewingLengthIndexLevel * phrases =
                m_chewing_length_indexes
                [initial][middle][final][CHEWING_ZERO_TONE];
            if (phrases)
                result |= phrases->search
                    (m_options, phrase_length - 1, keys + 1, ranges);

            phrases = m_chewing_length_indexes
                [initial][middle][final][(ChewingTone) first_key.m_tone];
            if (phrases)
                result |= phrases->search
                    (m_options, phrase_length - 1, keys + 1, ranges);
            return result;
        }
    }
    return result;
}


ChewingLengthIndexLevel::ChewingLengthIndexLevel() {
    m_chewing_array_indexes = g_array_new(FALSE, TRUE, sizeof(void *));
}

ChewingLengthIndexLevel::~ChewingLengthIndexLevel() {
#define CASE(len) case len:                                             \
    {                                                                   \
        ChewingArrayIndexLevel<len> * & array = g_array_index           \
            (m_chewing_array_indexes, ChewingArrayIndexLevel<len> *, len); \
        if (array)                                                      \
            delete array;                                               \
        array = NULL;                                                   \
        break;                                                          \
    }

    for (guint i = 0; i < m_chewing_array_indexes->len; ++i) {
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
#undef CASE
    g_array_free(m_chewing_array_indexes, TRUE);
}


int ChewingLengthIndexLevel::search(pinyin_option_t options, int phrase_length,
                                    /* in */ const ChewingKey keys[],
                                    /* out */ PhraseIndexRanges ranges) const {
    int result = SEARCH_NONE;
    if ((int) m_chewing_array_indexes->len < phrase_length + 1)
        return result;
    if ((int) m_chewing_array_indexes->len > phrase_length + 1)
        result |= SEARCH_CONTINUED;

#define CASE(len) case len:                                             \
    {                                                                   \
        ChewingArrayIndexLevel<len> * & array = g_array_index           \
            (m_chewing_array_indexes, ChewingArrayIndexLevel<len> *, len); \
        if (!array)                                                     \
            return result;                                              \
        result |= array->search(options, keys, ranges);                 \
        return result;                                                  \
    }

    switch (phrase_length) {
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
int ChewingArrayIndexLevel<phrase_length>::search
(pinyin_option_t options, /* in */ const ChewingKey keys[],
 /* out */ PhraseIndexRanges ranges) const {
    IndexItem * chunk_begin = NULL, * chunk_end = NULL;
    chunk_begin = (IndexItem *) m_chunk.begin();
    chunk_end = (IndexItem *) m_chunk.end();

    /* do the search */
    ChewingKey left_keys[phrase_length], right_keys[phrase_length];
    compute_lower_value2(options, keys, left_keys, phrase_length);
    compute_upper_value2(options, keys, right_keys, phrase_length);

    IndexItem left(left_keys, -1), right(right_keys, -1);

    IndexItem * begin = std_lite::lower_bound
        (chunk_begin, chunk_end, left,
         phrase_exact_less_than2<phrase_length>);
    IndexItem * end   = std_lite::upper_bound
        (chunk_begin, chunk_end, right,
         phrase_exact_less_than2<phrase_length>);

    return convert(options, keys, begin, end, ranges);
}

/* compress consecutive tokens */
template<size_t phrase_length>
int ChewingArrayIndexLevel<phrase_length>::convert
(pinyin_option_t options, const ChewingKey keys[],
 IndexItem * begin, IndexItem * end,
 PhraseIndexRanges ranges) const {
    IndexItem * iter = NULL;
    PhraseIndexRange cursor;
    GArray * head, * cursor_head = NULL;

    int result = SEARCH_NONE;
    /* TODO: check the below code */
    cursor.m_range_begin = null_token; cursor.m_range_end = null_token;
    for (iter = begin; iter != end; ++iter) {
        if (0 != pinyin_compare_with_ambiguities2
            (options, keys, iter->m_keys, phrase_length))
            continue;

        phrase_token_t token = iter->m_token;
        head = ranges[PHRASE_INDEX_LIBRARY_INDEX(token)];
        if (NULL == head)
            continue;

        result |= SEARCH_OK;

        if (null_token == cursor.m_range_begin) {
            cursor.m_range_begin = token;
            cursor.m_range_end   = token + 1;
            cursor_head = head;
        } else if (cursor.m_range_end == token &&
                   PHRASE_INDEX_LIBRARY_INDEX(cursor.m_range_begin) ==
                   PHRASE_INDEX_LIBRARY_INDEX(token)) {
            ++cursor.m_range_end;
        } else {
            g_array_append_val(cursor_head, cursor);
            cursor.m_range_begin = token; cursor.m_range_end = token + 1;
            cursor_head = head;
        }
    }

    if (null_token == cursor.m_range_begin)
        return result;

    g_array_append_val(cursor_head, cursor);
    return result;
}


/* add/remove index method */

int ChewingBitmapIndexLevel::add_index(int phrase_length,
                                       /* in */ const ChewingKey keys[],
                                       /* in */ phrase_token_t token) {
    const ChewingKey first_key = keys[0];
    ChewingLengthIndexLevel * & length_array = m_chewing_length_indexes
        [first_key.m_initial][first_key.m_middle]
        [first_key.m_final][first_key.m_tone];

    if (NULL == length_array) {
        length_array = new ChewingLengthIndexLevel();
    }

    return length_array->add_index(phrase_length - 1, keys + 1, token);
}

int ChewingBitmapIndexLevel::remove_index(int phrase_length,
                                          /* in */ const ChewingKey keys[],
                                          /* in */ phrase_token_t token) {
    const ChewingKey first_key = keys[0];
    ChewingLengthIndexLevel * & length_array = m_chewing_length_indexes
        [first_key.m_initial][first_key.m_middle]
        [first_key.m_final][first_key.m_tone];

    if (NULL == length_array)
        return ERROR_REMOVE_ITEM_DONOT_EXISTS;

    int retval = length_array->remove_index(phrase_length - 1, keys + 1, token);

    /* remove empty array. */
    if (0 == length_array->get_length()) {
        delete length_array;
        length_array = NULL;
    }

    return retval;
}

int ChewingLengthIndexLevel::add_index(int phrase_length,
                                       /* in */ const ChewingKey keys[],
                                       /* in */ phrase_token_t token) {
    if (!(phrase_length + 1 < MAX_PHRASE_LENGTH))
        return ERROR_PHRASE_TOO_LONG;

    if ((int) m_chewing_array_indexes->len <= phrase_length)
        g_array_set_size(m_chewing_array_indexes, phrase_length + 1);

#define CASE(len) case len:                                     \
    {                                                           \
        ChewingArrayIndexLevel<len> * & array = g_array_index   \
            (m_chewing_array_indexes,                           \
             ChewingArrayIndexLevel<len> *, len);               \
        if (NULL == array)                                      \
            array = new ChewingArrayIndexLevel<len>;            \
        return array->add_index(keys, token);                   \
    }

    switch(phrase_length) {
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

int ChewingLengthIndexLevel::remove_index(int phrase_length,
                                          /* in */ const ChewingKey keys[],
                                          /* in */ phrase_token_t token) {
    if (!(phrase_length + 1 < MAX_PHRASE_LENGTH))
        return ERROR_PHRASE_TOO_LONG;

    if ((int) m_chewing_array_indexes->len <= phrase_length)
        return ERROR_REMOVE_ITEM_DONOT_EXISTS;

#define CASE(len) case len:                                     \
    {                                                           \
        ChewingArrayIndexLevel<len> * & array = g_array_index   \
            (m_chewing_array_indexes,                           \
             ChewingArrayIndexLevel<len> *, len);               \
        if (NULL == array)                                      \
            return ERROR_REMOVE_ITEM_DONOT_EXISTS;              \
        int retval = array->remove_index(keys, token);          \
                                                                \
        /* remove empty array. */                               \
        if (0 == array->get_length()) {                         \
            delete array;                                       \
            array = NULL;                                       \
                                                                \
            /* shrink self array. */                            \
            g_array_set_size(m_chewing_array_indexes,           \
                             get_length());                     \
        }                                                       \
        return retval;                                          \
    }

    switch (phrase_length) {
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
int ChewingArrayIndexLevel<phrase_length>::add_index
(/* in */ const ChewingKey keys[], /* in */ phrase_token_t token) {
    IndexItem * begin, * end;

    IndexItem add_elem(keys, token);
    begin = (IndexItem *) m_chunk.begin();
    end   = (IndexItem *) m_chunk.end();

    std_lite::pair<IndexItem *, IndexItem *> range;
    range = std_lite::equal_range
        (begin, end, add_elem, phrase_exact_less_than2<phrase_length>);

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
int ChewingArrayIndexLevel<phrase_length>::remove_index
(/* in */ const ChewingKey keys[], /* in */ phrase_token_t token) {
    IndexItem * begin, * end;

    IndexItem remove_elem(keys, token);
    begin = (IndexItem *) m_chunk.begin();
    end   = (IndexItem *) m_chunk.end();

    std_lite::pair<IndexItem *, IndexItem *> range;
    range = std_lite::equal_range
        (begin, end, remove_elem, phrase_exact_less_than2<phrase_length>);

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
bool ChewingLargeTable::load_text(FILE * infile, TABLE_PHONETIC_TYPE type) {
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

        if(feof(infile))
            break;

        glong len = g_utf8_strlen(phrase, -1);

        ChewingKeyVector keys;
        ChewingKeyRestVector key_rests;

        keys = g_array_new(FALSE, FALSE, sizeof(ChewingKey));
        key_rests = g_array_new(FALSE, FALSE, sizeof(ChewingKeyRest));

        switch (type) {
        case PINYIN_TABLE: {
            PinyinDirectParser2 parser;
            pinyin_option_t options = USE_TONE;
            parser.parse(options, keys, key_rests, pinyin, strlen(pinyin));
            break;
        }

        case ZHUYIN_TABLE: {
            ZhuyinDirectParser2 parser;
            pinyin_option_t options = USE_TONE | FORCE_TONE;
            parser.parse(options, keys, key_rests, pinyin, strlen(pinyin));
            break;
        }
        };

        if (len != keys->len) {
            fprintf(stderr, "ChewingLargeTable::load_text:%s\t%s\t%u\t%ld\n",
                    pinyin, phrase, token, freq);
            continue;
        }

        add_index(keys->len, (ChewingKey *)keys->data, token);

        g_array_free(keys, TRUE);
        g_array_free(key_rests, TRUE);
    }

    return true;
}


/* load/store method */

bool ChewingBitmapIndexLevel::load(MemoryChunk * chunk, table_offset_t offset,
                                   table_offset_t end) {
    reset();
    char * begin = (char *) chunk->begin();
    table_offset_t phrase_begin, phrase_end;
    table_offset_t * index = (table_offset_t *) (begin + offset);
    phrase_end = *index;

    for (int k = 0; k < CHEWING_NUMBER_OF_INITIALS; ++k)
        for (int l = 0; l < CHEWING_NUMBER_OF_MIDDLES; ++l)
            for (int m = 0; m < CHEWING_NUMBER_OF_FINALS; ++m)
                for (int n = 0; n < CHEWING_NUMBER_OF_TONES; ++n) {
                    phrase_begin = phrase_end;
                    index++;
                    phrase_end = *index;

                    if (phrase_begin == phrase_end) /* null pointer */
                        continue;

                    /* after reset() all phrases are null pointer. */
                    ChewingLengthIndexLevel * phrases = new ChewingLengthIndexLevel;
                    m_chewing_length_indexes[k][l][m][n] = phrases;

                    phrases->load(chunk, phrase_begin, phrase_end - 1);
                    assert(phrase_end <= end);
                    assert(*(begin + phrase_end - 1)  == c_separate);
                }

    offset += (CHEWING_NUMBER_OF_INITIALS * CHEWING_NUMBER_OF_MIDDLES * CHEWING_NUMBER_OF_FINALS * CHEWING_NUMBER_OF_TONES + 1) * sizeof(table_offset_t);
    assert(c_separate == *(begin + offset));
    return true;
}

bool ChewingBitmapIndexLevel::store(MemoryChunk * new_chunk,
                                    table_offset_t offset,
                                    table_offset_t & end) {
    table_offset_t phrase_end;
    table_offset_t index = offset;
    offset += (CHEWING_NUMBER_OF_INITIALS * CHEWING_NUMBER_OF_MIDDLES * CHEWING_NUMBER_OF_FINALS * CHEWING_NUMBER_OF_TONES + 1) * sizeof(table_offset_t);

    /* add '#' */
    new_chunk->set_content(offset, &c_separate, sizeof(char));
    offset += sizeof(char);
    new_chunk->set_content(index, &offset, sizeof(table_offset_t));
    index += sizeof(table_offset_t);

    for (int k = 0; k < CHEWING_NUMBER_OF_INITIALS; ++k)
        for (int l = 0; l < CHEWING_NUMBER_OF_MIDDLES; ++l)
            for (int m = 0; m < CHEWING_NUMBER_OF_FINALS; ++m)
                for (int n = 0; n < CHEWING_NUMBER_OF_TONES; ++n) {
                    ChewingLengthIndexLevel * phrases =
                        m_chewing_length_indexes[k][l][m][n];

                    if (NULL == phrases) { /* null pointer */
                        new_chunk->set_content(index, &offset,
                                               sizeof(table_offset_t));
                        index += sizeof(table_offset_t);
                        continue;
                    }

                    /* has a end '#' */
                    phrases->store(new_chunk, offset, phrase_end);
                    offset = phrase_end;

                    /* add '#' */
                    new_chunk->set_content(offset, &c_separate, sizeof(char));
                    offset += sizeof(char);
                    new_chunk->set_content(index, &offset,
                                           sizeof(table_offset_t));
                    index += sizeof(table_offset_t);
                }

    end = offset;
    return true;
}

bool ChewingLengthIndexLevel::load(MemoryChunk * chunk, table_offset_t offset,
                                   table_offset_t end) {
    char * begin = (char *) chunk->begin();
    guint32 nindex = *((guint32 *)(begin + offset)); /* number of index */
    table_offset_t * index = (table_offset_t *)
        (begin + offset + sizeof(guint32));

    table_offset_t phrase_begin, phrase_end = *index;
    g_array_set_size(m_chewing_array_indexes, 0);
    for (guint32 i = 0; i < nindex; ++i) {
        phrase_begin = phrase_end;
        index++;
        phrase_end = *index;

        if (phrase_begin == phrase_end) {
            void * null = NULL;
            g_array_append_val(m_chewing_array_indexes, null);
            continue;
        }

#define CASE(len) case len:                                             \
        {                                                               \
            ChewingArrayIndexLevel<len> * phrase =                      \
                new ChewingArrayIndexLevel<len>;                        \
            phrase->load(chunk, phrase_begin, phrase_end - 1);          \
            assert(*(begin + phrase_end - 1) == c_separate);            \
            assert(phrase_end <= end);                                  \
            g_array_append_val(m_chewing_array_indexes, phrase);        \
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

    /* check '#' */
    offset += sizeof(guint32) + (nindex + 1) * sizeof(table_offset_t);
    assert(c_separate == *(begin + offset));
    return true;
}

bool ChewingLengthIndexLevel::store(MemoryChunk * new_chunk,
                                    table_offset_t offset,
                                    table_offset_t & end) {
    guint32 nindex = m_chewing_array_indexes->len; /* number of index */
    new_chunk->set_content(offset, &nindex, sizeof(guint32));
    table_offset_t index = offset + sizeof(guint32);

    offset += sizeof(guint32) + (nindex + 1) * sizeof(table_offset_t);
    new_chunk->set_content(offset, &c_separate, sizeof(char));
    offset += sizeof(char);
    new_chunk->set_content(index, &offset, sizeof(table_offset_t));
    index += sizeof(table_offset_t);

    table_offset_t phrase_end;
    for (guint32 i = 0; i < nindex; ++i) {
#define CASE(len) case len:                                             \
        {                                                               \
            ChewingArrayIndexLevel<len> * phrase = g_array_index        \
                (m_chewing_array_indexes, ChewingArrayIndexLevel<len> *, len); \
            if (NULL == phrase) {                                       \
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
#undef CASE

        /* add '#' */
        new_chunk->set_content(offset, &c_separate, sizeof(char));
        offset += sizeof(char);
        new_chunk->set_content(index, &offset, sizeof(table_offset_t));
        index += sizeof(table_offset_t);
    }

    end = offset;
    return true;
}

template<size_t phrase_length>
bool ChewingArrayIndexLevel<phrase_length>::
load(MemoryChunk * chunk, table_offset_t offset, table_offset_t end) {
    char * begin = (char *) chunk->begin();
    m_chunk.set_chunk(begin + offset, end - offset, NULL);
    return true;
}

template<size_t phrase_length>
bool ChewingArrayIndexLevel<phrase_length>::
store(MemoryChunk * new_chunk, table_offset_t offset, table_offset_t & end) {
    new_chunk->set_content(offset, m_chunk.begin(), m_chunk.size());
    end = offset + m_chunk.size();
    return true;
}


/* get length method */

int ChewingLengthIndexLevel::get_length() const {
    int length = m_chewing_array_indexes->len;

    /* trim trailing zero. */
    for (int i = length - 1; i >= 0; --i) {
        void * array = g_array_index(m_chewing_array_indexes, void *, i);

        if (NULL != array)
            break;

        --length;
    }

    return length;
}

template<size_t phrase_length>
int ChewingArrayIndexLevel<phrase_length>::get_length() const {
    IndexItem * chunk_begin = NULL, * chunk_end = NULL;
    chunk_begin = (IndexItem *) m_chunk.begin();
    chunk_end = (IndexItem *) m_chunk.end();

    return chunk_end - chunk_begin;
}


/* mask out method */

bool ChewingBitmapIndexLevel::mask_out(phrase_token_t mask,
                                       phrase_token_t value) {
    for (int k = CHEWING_ZERO_INITIAL; k < CHEWING_NUMBER_OF_INITIALS; ++k)
        for (int l = CHEWING_ZERO_MIDDLE; l < CHEWING_NUMBER_OF_MIDDLES; ++l)
            for (int m = CHEWING_ZERO_FINAL; m < CHEWING_NUMBER_OF_FINALS; ++m)
                for (int n = CHEWING_ZERO_TONE; n < CHEWING_NUMBER_OF_TONES;
                     ++n) {
                    ChewingLengthIndexLevel * & length_array =
                        m_chewing_length_indexes[k][l][m][n];

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

bool ChewingLengthIndexLevel::mask_out(phrase_token_t mask,
                                       phrase_token_t value) {
#define CASE(len) case len:                                     \
    {                                                           \
        ChewingArrayIndexLevel<len> * & array = g_array_index   \
            (m_chewing_array_indexes,                           \
             ChewingArrayIndexLevel<len> *, len);               \
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

    for (guint i = 0; i < m_chewing_array_indexes->len; ++i) {
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
#undef CASE
    g_array_set_size(m_chewing_array_indexes, get_length());
    return true;
}

template<size_t phrase_length>
bool ChewingArrayIndexLevel<phrase_length>::mask_out
(phrase_token_t mask, phrase_token_t value) {
    IndexItem * begin = NULL, * end = NULL;
    begin = (IndexItem *) m_chunk.begin();
    end   = (IndexItem *) m_chunk.end();

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
