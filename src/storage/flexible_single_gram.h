/* 
 *  libpinyin
 *  Library to deal with pinyin.
 *  
 *  Copyright (C) 2015 Peng Wu <alexepico@gmail.com>
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

#ifndef FLEXIBLE_SINGLE_GRAM_H
#define FLEXIBLE_SINGLE_GRAM_H

namespace pinyin{

typedef GArray * FlexibleBigramPhraseArray;

/**
 * FlexibleSingleGram:
 * @ArrayHeader: the struct ArrayHeader.
 * @ArrayItem: the struct ArrayItem.
 *
 * The flexible single gram is mainly used for training purpose.
 *
 */

template<typename ArrayHeader, typename ArrayItem>
class FlexibleSingleGram{
    template<typename MH, typename AH,
             typename AI>
    friend class FlexibleBigram;
private:
    MemoryChunk m_chunk;
    FlexibleSingleGram(void * buffer, size_t length, bool copy){
        if (copy)
            m_chunk.set_content(0, buffer, length);
        else
            m_chunk.set_chunk(buffer, length, NULL);
    }
public:
    /**
     * ArrayItemWithToken:
     *
     * Define the struct ArrayItemWithToken type.
     *
     */
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
    /**
     * FlexibleSingleGram::FlexibleSingleGram:
     *
     * The constructor of the FlexibleSingleGram.
     *
     */
    FlexibleSingleGram(){
        m_chunk.set_size(sizeof(ArrayHeader));
        memset(m_chunk.begin(), 0, sizeof(ArrayHeader));
    }

    /**
     * FlexibleSingleGram::retrieve_all:
     * @array: the array to store all items in this single gram.
     * @returns: whether the retrieve operation is successful.
     *
     * Retrieve all items in this single gram.
     *
     */
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

    /**
     * FlexibleSingleGram::search:
     * @range: the token range.
     * @array: the array to store the array items with token in the range.
     * @returns: whether the search operation is successful.
     *
     * Search the array items with token in the range.
     *
     * Note: The array result may contain many items.
     *
     */
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

    /**
     * FlexibleSingleGram::insert_array_item:
     * @token: the phrase token to be inserted.
     * @item: the array item of this token.
     * @returns: whether the insert operation is successful.
     *
     * Insert the array item of the token.
     *
     */
    bool insert_array_item(/* in */ phrase_token_t token,
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
                return false;
            }
        }
        m_chunk.insert_content(m_chunk.size(), &insert_item,
                               sizeof(ArrayItemWithToken));
        return true;
    }

    /**
     * FlexibleSingleGram::remove_array_item:
     * @token: the phrase token to be removed.
     * @item: the content of the removed array item.
     * @returns: whether the remove operation is successful.
     *
     * Remove the array item of the token.
     *
     */
    bool remove_array_item(/* in */ phrase_token_t token,
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
                size_t offset = sizeof(ArrayHeader) +
                    sizeof(ArrayItemWithToken) * (cur_item - begin);
                m_chunk.remove_content(offset, sizeof(ArrayItemWithToken));
                return true;
            }
        }
        return false;
    }

    /**
     * FlexibleSingleGram::get_array_item:
     * @token: the phrase token.
     * @item: the array item of the token.
     * @returns: whether the get operation is successful.
     *
     * Get the array item of the token.
     *
     */
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

    /**
     * FlexibleSingleGram::set_array_item:
     * @token: the phrase token.
     * @item: the array item of the token.
     * @returns: whether the set operation is successful.
     *
     * Set the array item of the token.
     *
     */
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

        for ( ; cur_item != end; ++cur_item ){
            if ( cur_item->m_token > token ){
                return false;
            }
            if ( cur_item->m_token == token ){
                memcpy(&(cur_item->m_item), &item, sizeof(ArrayItem));
                return true;
            }
        }
        return false;
    }

    /**
     * FlexibleSingleGram::get_array_header:
     * @header: the array header of this single gram.
     * @returns: whether the get operation is successful.
     *
     * Get the array header of this single gram.
     *
     */
    bool get_array_header(/* out */ ArrayHeader & header){
        /* clear retval */
        memset(&header, 0, sizeof(ArrayHeader));
        char * buf_begin = (char *)m_chunk.begin();
        memcpy(&header, buf_begin, sizeof(ArrayHeader));
        return true;
    }

    /**
     * FlexibleSingleGram::set_array_header:
     * @header: the array header of this single gram.
     * @returns: whether the set operation is successful.
     *
     * Set the array header of this single gram.
     *
     */
    bool set_array_header(/* in */ const ArrayHeader & header){
        char * buf_begin = (char *)m_chunk.begin();
        memcpy(buf_begin, &header, sizeof(ArrayHeader));
        return true;
    }
};

};

#endif
