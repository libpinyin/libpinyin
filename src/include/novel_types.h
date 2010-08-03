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

#ifndef NOVEL_TYPES_H
#define NOVEL_TYPES_H

#include <limits.h>
#include <glib.h>

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
#define PHRASE_INDEX_MAKE_TOKEN(phrase_index, token) \
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

typedef GArray * BigramPhraseArray; /* Array of HighLevelPhraseItem */

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

const phrase_token_t sentence_start = 1;
const phrase_token_t token_min = 0;
const phrase_token_t token_max = UINT_MAX;

const char c_separate = '#';
typedef guint32 table_offset_t;

typedef double parameter_t;

#define LAMBDA_PARAMETER 0.588792

#endif
