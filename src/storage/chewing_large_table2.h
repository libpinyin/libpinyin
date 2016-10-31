/* 
 *  libpinyin
 *  Library to deal with pinyin.
 *  
 *  Copyright (C) 2016 Peng Wu <alexepico@gmail.com>
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

#ifndef CHEWING_LARGE_TABLE2_H
#define CHEWING_LARGE_TABLE2_H

#include "novel_types.h"
#include "memory_chunk.h"
#include "chewing_key.h"
#include "pinyin_phrase3.h"

#ifdef HAVE_BERKELEY_DB
#include "chewing_large_table2_bdb.h"
#endif

#ifdef HAVE_KYOTO_CABINET
#include "chewing_large_table2_kyotodb.h"
#endif

namespace pinyin{

class MaskOutVisitor2;

/* As this is a template class, the code will be in the header file. */
template<int phrase_length>
class ChewingTableEntry{
    friend class ChewingLargeTable2;
    friend class MaskOutVisitor2;
protected:
    typedef PinyinIndexItem2<phrase_length> IndexItem;

protected:
    MemoryChunk m_chunk;

private:
    /* Disallow used outside. */
    ChewingTableEntry() {}

public:
    /* convert method. */
    /* compress consecutive tokens */
    int convert(const ChewingKey keys[],
     const IndexItem * begin, const IndexItem * end,
     PhraseIndexRanges ranges) const {
        const IndexItem * iter = NULL;
        PhraseIndexRange cursor;
        GArray * head, * cursor_head = NULL;

        int result = SEARCH_NONE;
        /* TODO: check the below code */
        cursor.m_range_begin = null_token; cursor.m_range_end = null_token;
        for (iter = begin; iter != end; ++iter) {
            if (0 != pinyin_compare_with_tones
                (keys, iter->m_keys, phrase_length))
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

    /* search method */
    int search(/* in */ const ChewingKey keys[],
               /* out */ PhraseIndexRanges ranges) const {
        IndexItem item;
        if (contains_incomplete_pinyin(keys, phrase_length)) {
            compute_incomplete_chewing_index(keys, item.m_keys, phrase_length);
        } else {
            compute_chewing_index(keys, item.m_keys, phrase_length);
        }

        const IndexItem * begin = (IndexItem *) m_chunk.begin();
        const IndexItem * end = (IndexItem *) m_chunk.end();

        std_lite::pair<const IndexItem *, const IndexItem *> range =
            std_lite::equal_range(begin, end, item,
                                  phrase_less_than_with_tones<phrase_length>);

        return convert(keys, range.first, range.second, ranges);
    }

    /* add/remove index method */
    int add_index(/* in */ const ChewingKey keys[],
                  /* in */ phrase_token_t token) {
        const IndexItem * begin = (IndexItem *) m_chunk.begin();
        const IndexItem * end = (IndexItem *) m_chunk.end();

        const IndexItem add_elem(keys, token);

        std_lite::pair<const IndexItem *, const IndexItem *> range =
            std_lite::equal_range(begin, end, add_elem,
                                  phrase_exact_less_than2<phrase_length>);

        const IndexItem * cur_elem;
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

    int remove_index(/* in */ const ChewingKey keys[],
                     /* in */ phrase_token_t token) {
        const IndexItem * begin = (IndexItem *) m_chunk.begin();
        const IndexItem * end = (IndexItem *) m_chunk.end();

        const IndexItem remove_elem(keys, token);

        std_lite::pair<const IndexItem *, const IndexItem *> range =
            std_lite::equal_range(begin, end, remove_elem,
                                  phrase_exact_less_than2<phrase_length>);

        const IndexItem * cur_elem;
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

    /* get length method */
    int get_length() const {
        const IndexItem * begin = (IndexItem *) m_chunk.begin();
        const IndexItem * end = (IndexItem *) m_chunk.end();

        return end - begin;
    }

    /* mask out method */
    bool mask_out(phrase_token_t mask, phrase_token_t value) {
        const IndexItem * begin = (IndexItem *) m_chunk.begin();
        const IndexItem * end = (IndexItem *) m_chunk.end();

        const IndexItem * cur_elem;
        for (cur_elem = begin; cur_elem != end; ++cur_elem) {
            /* not match. */
            if ((cur_elem->m_token & mask) != value)
                continue;

            int offset = (cur_elem - begin) * sizeof(IndexItem);
            m_chunk.remove_content(offset, sizeof(IndexItem));

            /* update chunk end. */
            end = (IndexItem *) m_chunk.end();
            --cur_elem;
        }

        return true;
    }

};

};

#endif
