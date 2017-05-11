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

/*
 * This header file contains novel types designed for pinyin processing.
 */


#ifndef NOVEL_TYPES_H
#define NOVEL_TYPES_H

#include <glib.h>

G_BEGIN_DECLS

typedef guint32 phrase_token_t;
typedef gunichar ucs4_t;

/*
 *  Phrase Index Library Definition
 *  Reserve 4-bits for future usage.
 */

#define PHRASE_MASK  0x00FFFFFF
#define PHRASE_INDEX_LIBRARY_MASK 0x0F000000
#define PHRASE_INDEX_LIBRARY_COUNT (1<<4)
#define PHRASE_INDEX_LIBRARY_INDEX(token) ((token&PHRASE_INDEX_LIBRARY_MASK)>>24)
#define PHRASE_INDEX_MAKE_TOKEN(phrase_index, token)                    \
    ( ( (phrase_index<<24) & PHRASE_INDEX_LIBRARY_MASK)|(token & PHRASE_MASK))


/* 
 *  PhraseIndexRanges definitions
 */

struct PhraseIndexRange{
    phrase_token_t m_range_begin;
    phrase_token_t m_range_end; /* pass the last item like stl */
};

/* Array of PhraseIndexRange */
typedef GArray * PhraseIndexRanges[PHRASE_INDEX_LIBRARY_COUNT];
/* Array of Token */
typedef GArray * PhraseTokens[PHRASE_INDEX_LIBRARY_COUNT];


/* 
 *  PinYin Table Definition
 */


/* For both PinYin Table and Phrase Table */
enum SearchResult{
    SEARCH_NONE = 0x00,           /* found nothing */
    SEARCH_OK = 0x01 ,            /* found items */
    SEARCH_CONTINUED = 0x02       /* has longer word in the storage to search */
};

/* For Phrase Index */
enum ErrorResult{
    ERROR_OK = 0,                /* operate ok */
    ERROR_INSERT_ITEM_EXISTS,    /* item already exists */
    ERROR_REMOVE_ITEM_DONOT_EXISTS, /* item don't exists */
    ERROR_PHRASE_TOO_LONG,       /* the phrase is too long */
    ERROR_NO_SUB_PHRASE_INDEX,   /* sub phrase index is not loaded */
    ERROR_NO_ITEM,               /* item has a null slot */
    ERROR_OUT_OF_RANGE,          /* beyond the end of the sub phrase index */
    ERROR_FILE_CORRUPTION,       /* file is corrupted */
    ERROR_INTEGER_OVERFLOW,      /* integer is overflowed */
    ERROR_ALREADY_EXISTS,        /* the sub phrase already exists. */
    ERROR_NO_USER_TABLE          /* the user table is not loaded. */
};

/* For N-gram */
enum ATTACH_FLAG{
    ATTACH_READONLY = 1,
    ATTACH_READWRITE = 0x1 << 1,
    ATTACH_CREATE = 0x1 << 2,
};

/*
 *  n-gram Definition
 *  no B parameter(there are duplicated items in uni-gram and bi-gram)
 *  used in system n-gram and user n-gram.
 *  using delta technique.
 */

struct BigramPhraseItem{
    phrase_token_t m_token;
    gfloat         m_freq; /* P(W2|W1) */
};

struct BigramPhraseItemWithCount{
    phrase_token_t m_token;
    guint32        m_count;
    gfloat         m_freq; /* P(W2|W1) */
};

typedef GArray * BigramPhraseArray; /* Array of BigramPhraseItem */
typedef GArray * BigramPhraseWithCountArray; /* Array of BigramPhraseItemWithCount */

#define MAX_PHRASE_LENGTH 16

const phrase_token_t null_token = 0;
const phrase_token_t sentence_start = 1;
const phrase_token_t token_min = 0;
const phrase_token_t token_max = UINT_MAX;

const char c_separate = '#';

typedef guint32 table_offset_t;

typedef double parameter_t;

/* Array of ChewingKey/ChewingKeyRest */
typedef GArray * ChewingKeyVector;
typedef GArray * ChewingKeyRestVector;

/* Array of phrase_token_t */
typedef GArray * TokenVector;
typedef TokenVector MatchResult;

/* Array of lookup_constraint_t */
typedef GArray * CandidateConstraints;

typedef guint32 pinyin_option_t;

typedef guint32 pinyin_standard_option_t;

typedef guint32 pinyin_fuzzy_option_t;

typedef guint32 pinyin_correct_option_t;

typedef enum {
    /* for default tables. */
    RESERVED = 0,
    GB_DICTIONARY = 1,
    TSI_DICTIONARY = 1,
    GBK_DICTIONARY = 2,
    OPENGRAM_DICTIONARY = 3,
    MERGED_DICTIONARY = 4,
    ADDON_DICTIONARY = 5,
    NETWORK_DICTIONARY = 6,
    USER_DICTIONARY = 7,
} PHRASE_INDEX_LIBRARIES;

G_END_DECLS

#endif
