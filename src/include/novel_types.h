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

/*
 * This header file contains novel types designed for pinyin processing.
 */


#ifndef NOVEL_TYPES_H
#define NOVEL_TYPES_H

#include <limits.h>
#include <glib.h>

namespace pinyin{

typedef guint32 phrase_token_t;
typedef gunichar2 utf16_t;

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

/*Array of PhraseIndexRange*/
typedef GArray * PhraseIndexRanges[PHRASE_INDEX_LIBRARY_COUNT];

/* 
 *  PinYin Table Definition
 */
class MemoryChunk;


/* For both PinYin Table and Phrase Table */
enum SearchResult{
    SEARCH_NONE = 0x00,           /* found nothing */
    SEARCH_OK = 0x01 ,            /* found items */
    SEARCH_CONTINUED = 0x02       /* has longer word in the storage to search */
};

enum AddIndexResult{
    INSERT_OK = 0 ,            /* insert ok */         
    INSERT_ITEM_EXISTS         /* item already exists */
};

enum RemoveIndexResult{
    REMOVE_OK = 0,             /* remove ok */
    REMOVE_ITEM_DONOT_EXISTS   /* item don't exists */
};

/* For Phrase Index */
enum PhraseIndexResult{
    ERROR_OK = 0,                /* operate ok */
    ERROR_NO_SUB_PHRASE_INDEX,   /* sub phrase index is not loaded */
    ERROR_NO_ITEM,               /* item has a null slot */
    ERROR_OUT_OF_RANGE,          /* beyond the end of the sub phrase index */
    ERROR_FILE_CORRUPTION,       /* file is corrupted */
    ERROR_INTEGER_OVERFLOW       /* integer is overflowed */
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

/* 
 *  n-gram Definition
 *  n-gram library
 */

enum AttachOption{
    ATTACH_NEW_FILE = 1,
    ATTACH_READ = 2,
    ATTACH_READ_WRITE = 3
};

#define MAX_PHRASE_LENGTH 16

const phrase_token_t null_token = 0;
const phrase_token_t sentence_start = 1;
const phrase_token_t token_min = 0;
const phrase_token_t token_max = UINT_MAX;

const char c_separate = '#';
typedef guint32 table_offset_t;

typedef double parameter_t;

#define LAMBDA_PARAMETER 0.330642

/* Array of phrase_token_t */
typedef GArray * TokenVector;
typedef TokenVector MatchResults;

/* Array of lookup_constraint_t */
typedef GArray * CandidateConstraints;

typedef guint32 pinyin_option_t;

};

#endif
