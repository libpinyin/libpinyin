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
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */



#ifndef FLEXIBLE_NGRAM_H
#define FLEXIBLE_NGRAM_H

#include <db.h>
#include <errno.h>

/* Note: the signature of the template parameters.
 * struct MagicHeader, ArrayHeader, ArrayItem.
 */

namespace pinyin{

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

    /* insert array item */
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

    /* get array header */
    bool get_array_header(/* out */ ArrayHeader & header){
        /* clear retval */
        memset(&header, 0, sizeof(ArrayHeader));
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
    /* Note: some flexible bi-gram file format check should be here. */
private:
    DB * m_db;

    phrase_token_t m_magic_header_index[2];

    char m_magic_number[4];

    void reset(){
        if ( m_db ){
            m_db->sync(m_db, 0);
            m_db->close(m_db, 0);
            m_db = NULL;
        }
    }

public:
    FlexibleBigram(const char * magic_number){
        m_db = NULL;
        m_magic_header_index[0] = null_token;
        m_magic_header_index[1] = null_token;

        memcpy(m_magic_number, magic_number, sizeof(m_magic_number));
    }

    ~FlexibleBigram(){
        reset();
    }

    /* attach berkeley db on filesystem for training purpose. */
    bool attach(const char * dbfile, guint32 flags){
        reset();
        u_int32_t db_flags = 0;

        if ( flags & ATTACH_READONLY )
            db_flags |= DB_RDONLY;
        if ( flags & ATTACH_READWRITE )
            assert( !(flags & ATTACH_READONLY ) );

        if ( !dbfile )
            return false;
        int ret = db_create(&m_db, NULL, 0);
        if ( ret != 0 )
            assert(false);

        ret = m_db->open(m_db, NULL, dbfile, NULL, DB_HASH, db_flags, 0644);
        if ( ret != 0 && (flags & ATTACH_CREATE) ) {
            db_flags |= DB_CREATE;
            /* Create database file here, and write the signature. */
            ret = m_db->open(m_db, NULL, dbfile, NULL, DB_HASH, db_flags, 0644);
            if ( ret != 0 )
                return false;

            DBT db_key;
            memset(&db_key, 0, sizeof(DBT));
            db_key.data = m_magic_header_index;
            db_key.size = sizeof(m_magic_header_index);
            DBT db_data;
            memset(&db_data, 0, sizeof(DBT));
            db_data.data = m_magic_number;
            db_data.size = sizeof(m_magic_number);
            db_data.flags = DB_DBT_PARTIAL;
            db_data.doff = 0;
            db_data.dlen = sizeof(m_magic_number);

            ret = m_db->put(m_db, NULL, &db_key, &db_data, 0);
            return ret == 0;
        }

        /* check the signature. */
        DBT db_key;
        memset(&db_key, 0, sizeof(DBT));
        db_key.data = m_magic_header_index;
        db_key.size = sizeof(m_magic_header_index);
        DBT db_data;
        memset(&db_data, 0, sizeof(DBT));
        db_data.flags = DB_DBT_PARTIAL;
        db_data.doff = 0;
        db_data.dlen = sizeof(m_magic_number);
        ret = m_db->get(m_db, NULL, &db_key, &db_data, 0);
        if ( ret != 0 )
            return false;
        if ( sizeof(m_magic_number) != db_data.size )
            return false;
        if ( memcmp(db_data.data, m_magic_number,
                    sizeof(m_magic_number)) == 0 )
            return true;
        return false;
    }

    /* load/store one array. */
    bool load(phrase_token_t index,
              FlexibleSingleGram<ArrayHeader, ArrayItem> * & single_gram){
        if ( !m_db )
            return false;

        DBT db_key;
        memset(&db_key, 0, sizeof(DBT));
        db_key.data = &index;
        db_key.size = sizeof(phrase_token_t);

        single_gram = NULL;

        DBT db_data;
        memset(&db_data, 0, sizeof(DBT));
        int ret = m_db->get(m_db, NULL, &db_key, &db_data, 0);
        if ( ret != 0)
            return false;

        single_gram = new FlexibleSingleGram<ArrayHeader, ArrayItem>
            (db_data.data, db_data.size);

        return true;
    }

    bool store(phrase_token_t index,
               FlexibleSingleGram<ArrayHeader, ArrayItem> * single_gram){
        if ( !m_db )
            return false;

        DBT db_key;
        memset(&db_key, 0, sizeof(DBT));
        db_key.data = &index;
        db_key.size = sizeof(phrase_token_t);
        DBT db_data;
        memset(&db_data, 0, sizeof(DBT));
        db_data.data = single_gram->m_chunk.begin();
        db_data.size = single_gram->m_chunk.size();

        int ret = m_db->put(m_db, NULL, &db_key, &db_data, 0);
        return ret == 0;
    }

    bool remove(phrase_token_t index){
        if ( !m_db )
            return false;

        DBT db_key;
        memset(&db_key, 0, sizeof(DBT));
        db_key.data = &index;
        db_key.size = sizeof(phrase_token_t);

        int ret = m_db->del(m_db, NULL, &db_key, 0);
        return ret == 0;
    }

    /* array of phrase_token_t items, for parameter estimation. */
    bool get_all_items(GArray * items){
        g_array_set_size(items, 0);
        if ( !m_db )
            return false;

        DBC * cursorp;
        DBT key, data;
        int ret;

        /* Get a cursor */
        m_db->cursor(m_db, NULL, &cursorp, 0);

        /* Initialize our DBTs. */
        memset(&key, 0, sizeof(DBT));
        memset(&data, 0, sizeof(DBT));

        /* Iterate over the database, retrieving each record in turn. */
        while ((ret =  cursorp->c_get(cursorp, &key, &data, DB_NEXT)) == 0 ){
            if (key.size != sizeof(phrase_token_t)){
                /* skip magic header. */
                continue;
            }
            phrase_token_t * token = (phrase_token_t *) key.data;
            g_array_append_val(items, *token);
        }

        if ( ret != DB_NOTFOUND ){
            fprintf(stderr, "training db error, exit!");
            exit(EIO);
        }

        /* Cursors must be closed */
        if (cursorp != NULL)
            cursorp->c_close(cursorp);
        return true;
    }

    /* get/set magic header. */
    bool get_magic_header(MagicHeader & header){
        /* clear retval */
        memset(&header, 0, sizeof(MagicHeader));

        if ( !m_db )
            return false;

        DBT db_key;
        memset(&db_key, 0, sizeof(DBT));
        db_key.data = m_magic_header_index;
        db_key.size = sizeof(m_magic_header_index);
        DBT db_data;
        memset(&db_data, 0, sizeof(DBT));
        db_data.flags = DB_DBT_PARTIAL;
        db_data.doff = sizeof(m_magic_number);
        db_data.dlen = sizeof(MagicHeader);
        
        int ret = m_db->get(m_db, NULL, &db_key, &db_data, 0);
        if ( ret != 0 )
            return false;

        if ( sizeof(MagicHeader) != db_data.size )
            return false;

        memcpy(&header, db_data.data, sizeof(MagicHeader));
        return true;
    }

    bool set_magic_header(const MagicHeader & header){
        if ( !m_db )
            return false;

        DBT db_key;
        memset(&db_key, 0, sizeof(DBT));
        db_key.data = m_magic_header_index;
        db_key.size = sizeof(m_magic_header_index);
        DBT db_data;
        memset(&db_data, 0, sizeof(DBT));
        db_data.data = (void *) &header;
        db_data.size = sizeof(MagicHeader);
        db_data.flags = DB_DBT_PARTIAL;
        db_data.doff = sizeof(m_magic_number);
        db_data.dlen = sizeof(MagicHeader);

        int ret = m_db->put(m_db, NULL, &db_key, &db_data, 0);
        return ret == 0;
    }

    bool get_array_header(phrase_token_t index, ArrayHeader & header){
        /* clear retval */
        memset(&header, 0, sizeof(ArrayHeader));

        if ( !m_db )
            return false;

        DBT db_key;
        memset(&db_key, 0, sizeof(DBT));
        db_key.data = &index;
        db_key.size = sizeof(phrase_token_t);

        DBT db_data;
        memset(&db_data, 0, sizeof(DBT));
        db_data.flags = DB_DBT_PARTIAL;
        db_data.doff = 0;
        db_data.dlen = sizeof(ArrayHeader);
        int ret = m_db->get(m_db, NULL, &db_key, &db_data, 0);
        if ( ret != 0 )
            return false;

        assert(db_data.size == sizeof(ArrayHeader));
        memcpy(&header, db_data.data, sizeof(ArrayHeader));
        return true;
    }

    bool set_array_header(phrase_token_t index, const ArrayHeader & header){
        if ( !m_db )
            return false;

        DBT db_key;
        memset(&db_key, 0, sizeof(DBT));
        db_key.data = &index;
        db_key.size = sizeof(phrase_token_t);
        DBT db_data;
        memset(&db_data, 0, sizeof(DBT));
        db_data.data = (void *)&header;
        db_data.size = sizeof(ArrayHeader);
        db_data.flags = DB_DBT_PARTIAL;
        db_data.doff = 0;
        db_data.dlen = sizeof(ArrayHeader);

        int ret = m_db->put(m_db, NULL, &db_key, &db_data, 0);
        return ret == 0;
    }

};

};

#endif
