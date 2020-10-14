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

#ifndef FLEXIBLE_NGRAM_BDB_H
#define FLEXIBLE_NGRAM_BDB_H

#ifdef HAVE_BERKELEY_DB
#include <errno.h>
#include <db.h>
#endif

namespace pinyin{

/**
 * FlexibleBigram:
 * @MagicHeader: the struct type of the magic header.
 * @ArrayHeader: the struct type of the array header.
 * @ArrayItem: the struct type of the array item.
 *
 * The flexible bi-gram is mainly used for training purpose.
 *
 */
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
    /**
     * FlexibleBigram::FlexibleBigram:
     * @magic_number: the 4 bytes magic number of the flexible bi-gram.
     *
     * The constructor of the FlexibleBigram.
     *
     */
    FlexibleBigram(const char * magic_number){
        m_db = NULL;
        m_magic_header_index[0] = null_token;
        m_magic_header_index[1] = null_token;

        memcpy(m_magic_number, magic_number, sizeof(m_magic_number));
    }

    /**
     * FlexibleBigram::~FlexibleBigram:
     *
     * The destructor of the FlexibleBigram.
     *
     */
    ~FlexibleBigram(){
        reset();
    }

    /**
     * FlexibleBigram::attach:
     * @dbfile: the path name of the flexible bi-gram.
     * @flags: the attach flags for the Berkeley DB.
     * @returns: whether the attach operation is successful.
     *
     * Attach Berkeley DB on filesystem for training purpose.
     *
     */
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

    /**
     * FlexibleBigram::load:
     * @index: the previous token in the flexible bi-gram.
     * @single_gram: the single gram of the previous token.
     * @copy: whether copy content to the single gram.
     * @returns: whether the load operation is successful.
     *
     * Load the single gram of the previous token.
     *
     */
    bool load(phrase_token_t index,
              FlexibleSingleGram<ArrayHeader, ArrayItem> * & single_gram,
              bool copy=false){
        single_gram = NULL;
        if ( !m_db )
            return false;

        DBT db_key;
        memset(&db_key, 0, sizeof(DBT));
        db_key.data = &index;
        db_key.size = sizeof(phrase_token_t);

        DBT db_data;
        memset(&db_data, 0, sizeof(DBT));
        int ret = m_db->get(m_db, NULL, &db_key, &db_data, 0);
        if ( ret != 0)
            return false;

        single_gram = new FlexibleSingleGram<ArrayHeader, ArrayItem>
            (db_data.data, db_data.size, copy);

        return true;
    }

    /**
     * FlexibleBigram::store:
     * @index: the previous token in the flexible bi-gram.
     * @single_gram: the single gram of the previous token.
     * @returns: whether the store operation is successful.
     *
     * Store the single gram of the previous token.
     *
     */
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

    /**
     * FlexibleBigram::remove:
     * @index: the previous token in the flexible bi-gram.
     * @returns: whether the remove operation is successful.
     *
     * Remove the single gram of the previous token.
     *
     */
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

    /**
     * FlexibleBigram::get_all_items:
     * @items: the GArray to store all previous tokens.
     * @returns: whether the get operation is successful.
     *
     * Get the array of all previous tokens for parameter estimation.
     *
     */
    bool get_all_items(GArray * items){
        g_array_set_size(items, 0);

        if ( !m_db )
            return false;

        DBC * cursorp;
        DBT key, data;
        int ret;

        /* Get a cursor */
        m_db->cursor(m_db, NULL, &cursorp, 0);

        if (NULL == cursorp)
            return false;

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

            /* Initialize our DBTs. */
            memset(&key, 0, sizeof(DBT));
            memset(&data, 0, sizeof(DBT));
        }

        if ( ret != DB_NOTFOUND ){
            fprintf(stderr, "training db error, exit!");

            if (cursorp != NULL)
                cursorp->c_close(cursorp);

            exit(EIO);
        }

        /* Cursors must be closed */
        if (cursorp != NULL)
            cursorp->c_close(cursorp);
        return true;
    }

    /**
     * FlexibleBigram::get_magic_header:
     * @header: the magic header.
     * @returns: whether the get operation is successful.
     *
     * Get the magic header of the flexible bi-gram.
     *
     */
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

    /**
     * FlexibleBigram::set_magic_header:
     * @header: the magic header.
     * @returns: whether the set operation is successful.
     *
     * Set the magic header of the flexible bi-gram.
     *
     */
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

    /**
     * FlexibleBigram::get_array_header:
     * @index: the previous token in the flexible bi-gram.
     * @header: the array header in the single gram of the previous token.
     * @returns: whether the get operation is successful.
     *
     * Get the array header in the single gram of the previous token.
     *
     */
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

    /**
     * FlexibleBigram::set_array_header:
     * @index: the previous token of the flexible bi-gram.
     * @header: the array header in the single gram of the previous token.
     * @returns: whether the set operation is successful.
     *
     * Set the array header in the single gram of the previous token.
     *
     */
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
