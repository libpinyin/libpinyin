/* 
 *  libpinyin
 *  Library to deal with pinyin.
 *  
 *  Copyright (C) 2011 Peng Wu <alexepico@gmail.com>
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



#ifndef FLEXIBLE_NGRAM_H
#define FLEXIBLE_NGRAM_H

#include <db.h>

/* Note: the signature of the template parameters.
 * struct MagicHeader, ArrayHeader, ArrayItem.
 */

typedef GArray * FlexibleBigramPhraseArray;

template<typename ArrayHeader, typename ArrayItem>
class FlexibleSingleGram{
    template<typename MH, typename AH,
             typename AI>
    friend class FlexibleBigram;
private:
    MemoryChunk m_chunk;
    FlexibleSingleGram(void * buffer, size_t length){
        m_chunk.set_chunk(buffer, length, NULL);
    }
public:
    /* item typedefs */
    typedef struct{
        phrase_token_t m_token;
        ArrayItem m_item;
    } ArrayItemWithToken;

    /* Null Constructor */
    FlexibleSingleGram(){
        m_chunk.set_size(sizeof(ArrayHeader));
        memset(m_chunk.begin(), 0, sizeof(ArrayHeader));
    }

    /* retrieve all items */
    bool retrieve_all(/* out */ FlexibleBigramPhraseArray array);

    /* search method */
    /* the array result contains many items */
    bool search(/* in */ PhraseIndexRange * range,
                /* out */ FlexibleBigramPhraseArray array);

    /* get array item */
    bool get_array_item(/* in */ phrase_token_t token,
                        /* out */ ArrayItem & item);
    /* set array item */
    bool set_array_item(/* in */ phrase_token_t token,
                        /* in */ const ArrayItem & item);

    /* get array header */
    bool get_array_header(/* out */ ArrayHeader & header){
        char * buf_begin = (char *)m_chunk.begin();
        memcpy(&header, buf_begin, sizeof(ArrayHeader));
        return true;
    }

    /* set array header */
    bool set_array_header(/* in */ const ArrayHeader & header){
        char * buf_begin = (char *)m_chunk.begin();
        memcpy(buf_begin, &header, sizeof(ArrayHeader));
        return true;
    }
};

template<typename MagicHeader, typename ArrayHeader,
         typename ArrayItem>
class FlexibleBigram{
private:
    DB * m_db;

    void reset(){
        if ( m_db ){
            m_db->close(m_db, 0);
            m_db = NULL;
        }
    }

public:
    FlexibleBigram(){
        m_db = NULL;
    }

    ~FlexibleBigram(){
        reset();
    }

    /* attach berkeley db on filesystem for training purpose. */
    bool attach(const char * dbfile);
    /* load/store one array. */
    bool load(phrase_token_t index,
              FlexibleSingleGram<ArrayHeader, ArrayItem> * & single_gram);
    bool store(phrase_token_t index,
               FlexibleSingleGram<ArrayHeader, ArrayItem> * & single_gram);
    /* array of phrase_token_t items, for parameter estimation. */
    bool get_all_items(GArray * items);

    /* get/set magic header. */
    bool get_magic_header(MagicHeader & header);
    bool set_magic_header(const MagicHeader & header);
};

#endif
