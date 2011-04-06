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

private:
    static bool token_less_than(const ArrayItemWithToken & lhs,
                                const ArrayItemWithToken & rhs){
        return lhs.m_token < rhs.m_token;
    }

public:
    /* Null Constructor */
    FlexibleSingleGram(){
        m_chunk.set_size(sizeof(ArrayHeader));
        memset(m_chunk.begin(), 0, sizeof(ArrayHeader));
    }

    /* retrieve all items */
    bool retrieve_all(/* out */ FlexibleBigramPhraseArray array){
        const ArrayItemWithToken * begin = (const ArrayItemWithToken *)
            ((const char *)(m_chunk.begin()) + sizeof(ArrayHeader));
        const ArrayItemWithToken * end = (const ArrayItemWithToken *)
            m_chunk.end();

        ArrayItemWithToken item;
        for ( const ArrayItemWithToken * cur_item = begin;
              cur_item != end;
              ++cur_item){
            /* Note: optimize this with g_array_append_vals? */
            item.m_token = cur_item->m_token;
            item.m_item = cur_item->m_item;
            g_array_append_val(array, item);
        }

        return true;
    }

    /* search method */
    /* the array result contains many items */
    bool search(/* in */ PhraseIndexRange * range,
                /* out */ FlexibleBigramPhraseArray array){
        const ArrayItemWithToken * begin = (const ArrayItemWithToken *)
            ((const char *)(m_chunk.begin()) + sizeof(ArrayHeader));
        const ArrayItemWithToken * end = (const ArrayItemWithToken *)
            m_chunk.end();

        ArrayItemWithToken compare_item;
        compare_item.m_token = range->m_range_begin;
        const ArrayItemWithToken * cur_item = std_lite::lower_bound
            (begin, end, compare_item, token_less_than);

        ArrayItemWithToken item;
        for ( ; cur_item != end; ++cur_item){
            if ( cur_item->m_token >= range->m_range_end )
                break;
            item.m_token = cur_item->m_token;
            item.m_item = cur_item->m_item;
            g_array_append_val(array, item);
        }

        return true;
    }

    /* get array item */
    bool get_array_item(/* in */ phrase_token_t token,
                        /* out */ ArrayItem & item)
    {
        /* clear retval */
        memset(&item, 0, sizeof(ArrayItem));

        const ArrayItemWithToken * begin = (const ArrayItemWithToken *)
            ((const char *)(m_chunk.begin()) + sizeof(ArrayHeader));
        const ArrayItemWithToken * end = (const ArrayItemWithToken *)
            m_chunk.end();

        ArrayItemWithToken compare_item;
        compare_item.m_token = token;
        const ArrayItemWithToken * cur_item = std_lite::lower_bound
            (begin, end, compare_item, token_less_than);

        for ( ; cur_item != end; ++cur_item){
            if ( cur_item->m_token > token )
                return false;
            if ( cur_item->m_token == token ){
                memcpy(&item, &(cur_item->m_item), sizeof(ArrayItem));
                return true;
            }
        }
        return false;
    }

    /* set array item */
    bool set_array_item(/* in */ phrase_token_t token,
                        /* in */ const ArrayItem & item){
        ArrayItemWithToken * begin = (ArrayItemWithToken *)
            ((const char *)(m_chunk.begin()) + sizeof(ArrayHeader));
        ArrayItemWithToken * end = (ArrayItemWithToken *)
            m_chunk.end();

        ArrayItemWithToken compare_item;
        compare_item.m_token = token;
        ArrayItemWithToken * cur_item = std_lite::lower_bound
            (begin, end, compare_item, token_less_than);

        ArrayItemWithToken insert_item;
        insert_item.m_token = token;
        insert_item.m_item = item;

        for ( ; cur_item != end; ++cur_item ){
            if ( cur_item->m_token > token ){
                size_t offset = sizeof(ArrayHeader) +
                    sizeof(ArrayItemWithToken) * (cur_item - begin);
                m_chunk.insert_content(offset, &insert_item,
                                       sizeof(ArrayItemWithToken));
                return true;
            }
            if ( cur_item->m_token == token ){
                cur_item.m_item = item;
                return true;
            }
        }
        m_chunk.insert_content(m_chunk.size(), &insert_item,
                               sizeof(ArrayItemWithToken));
        return true;
    }

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
