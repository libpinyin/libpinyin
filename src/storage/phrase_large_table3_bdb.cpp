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

#include "phrase_large_table3.h"
#include <errno.h>
#include "bdb_utils.h"

namespace pinyin{

PhraseLargeTable3::PhraseLargeTable3() {
    /* create in-memory db. */
    m_db = NULL;
    int ret = db_create(&m_db, NULL, 0);
    assert(0 == ret);

    ret = m_db->open(m_db, NULL, NULL, NULL,
                     DB_BTREE, DB_CREATE, 0600);
    assert(0 == ret);

    m_entry = new PhraseTableEntry;
}

void PhraseLargeTable3::reset() {
    if (m_db) {
        m_db->sync(m_db, 0);
        m_db->close(m_db, 0);
        m_db = NULL;
    }

    if (m_entry) {
        delete m_entry;
        m_entry = NULL;
    }
}

/* attach method */
bool PhraseLargeTable3::attach(const char * dbfile, guint32 flags) {
    reset();

    m_entry = new PhraseTableEntry;

    u_int32_t db_flags = attach_options(flags);

    if (!dbfile)
        return false;

    int ret = db_create(&m_db, NULL, 0);
    assert(0 == ret);

    ret = m_db->open(m_db, NULL, dbfile, NULL,
                     DB_BTREE, db_flags, 0644);
    if (ret != 0)
        return false;

    return true;
}

/* load/store method */
bool PhraseLargeTable3::load_db(const char * filename) {
    reset();

    m_entry = new PhraseTableEntry;

    /* create in-memory db. */
    int ret = db_create(&m_db, NULL, 0);
    assert(0 == ret);

    ret = m_db->open(m_db, NULL, NULL, NULL,
                     DB_BTREE, DB_CREATE, 0600);
    if (ret != 0)
        return false;

    /* load db into memory. */
    DB * tmp_db = NULL;
    ret = db_create(&tmp_db, NULL, 0);
    assert(0 == ret);

    if (NULL == tmp_db)
        return false;

    ret = tmp_db->open(tmp_db, NULL, filename, NULL,
                       DB_BTREE, DB_RDONLY, 0600);
    if (ret != 0)
        return false;

    if (!copy_bdb(tmp_db, m_db))
        return false;

    if (tmp_db != NULL)
        tmp_db->close(tmp_db, 0);

    return true;
}

bool PhraseLargeTable3::store_db(const char * new_filename) {
    DB * tmp_db = NULL;

    int ret = unlink(new_filename);
    if (ret != 0 && errno != ENOENT)
        return false;

    ret = db_create(&tmp_db, NULL, 0);
    assert(0 == ret);

    if (NULL == tmp_db)
        return false;

    ret = tmp_db->open(tmp_db, NULL, new_filename, NULL,
                       DB_BTREE, DB_CREATE, 0600);
    if (ret != 0)
        return false;

    if (!copy_bdb(m_db, tmp_db))
        return false;

    if (tmp_db != NULL) {
        tmp_db->sync(m_db, 0);
        tmp_db->close(tmp_db, 0);
    }

    return true;
}

/* search method */
int PhraseLargeTable3::search(int phrase_length,
                              /* in */ const ucs4_t phrase[],
                              /* out */ PhraseTokens tokens) const {
    int result = SEARCH_NONE;

    if (NULL == m_db)
        return result;
    assert(NULL != m_entry);

    DBT db_key;
    memset(&db_key, 0, sizeof(DBT));
    db_key.data = (void *) phrase;
    db_key.size = phrase_length * sizeof(ucs4_t);

    DBT db_data;
    memset(&db_data, 0, sizeof(DBT));
    int ret = m_db->get(m_db, NULL, &db_key, &db_data, 0);
    if (ret != 0)
        return result;

    /* continue searching. */
    result |= SEARCH_CONTINUED;

    m_entry->m_chunk.set_chunk(db_data.data, db_data.size, NULL);

    result = m_entry->search(tokens) | result;

    return result;
}

/* add_index/remove_index method */
int PhraseLargeTable3::add_index(int phrase_length,
                                 /* in */ const ucs4_t phrase[],
                                 /* in */ phrase_token_t token) {
    assert(NULL != m_db);
    assert(NULL != m_entry);

    /* load phrase table entry. */
    DBT db_key;
    memset(&db_key, 0, sizeof(DBT));
    db_key.data = (void *) phrase;
    db_key.size = phrase_length * sizeof(ucs4_t);

    DBT db_data;
    memset(&db_data, 0, sizeof(DBT));
    int ret = m_db->get(m_db, NULL, &db_key, &db_data, 0);

    if (ret != 0) {
        /* new entry. */
        PhraseTableEntry entry;
        entry.add_index(token);

        memset(&db_data, 0, sizeof(DBT));
        db_data.data = entry.m_chunk.begin();
        db_data.size = entry.m_chunk.size();
        ret = m_db->put(m_db, NULL, &db_key, &db_data, 0);
        if (ret != 0)
            return ERROR_FILE_CORRUPTION;

        /* recursively add keys for continued information. */
        for (size_t len = phrase_length - 1; len > 0; --len) {
            memset(&db_key, 0, sizeof(DBT));
            db_key.data = (void *) phrase;
            db_key.size = len * sizeof(ucs4_t);

            memset(&db_data, 0, sizeof(DBT));

            ret = m_db->get(m_db, NULL, &db_key, &db_data, 0);
            /* found entry. */
            if (0 == ret)
                return ERROR_OK;

            /* new entry with empty content. */
            memset(&db_data, 0, sizeof(DBT));

            ret = m_db->put(m_db, NULL, &db_key, &db_data, 0);
            if (ret != 0)
                return ERROR_FILE_CORRUPTION;
        }

        return ERROR_OK;
    }

    /* already have keys. */
    m_entry->m_chunk.set_chunk(db_data.data, db_data.size, NULL);
    int result = m_entry->add_index(token);

    /* store the entry. */
    memset(&db_data, 0, sizeof(DBT));
    db_data.data = m_entry->m_chunk.begin();
    db_data.size = m_entry->m_chunk.size();
    ret = m_db->put(m_db, NULL, &db_key, &db_data, 0);
    if (ret != 0)
        return ERROR_FILE_CORRUPTION;

    return result;
}

int PhraseLargeTable3::remove_index(int phrase_length,
                                    /* in */ const ucs4_t phrase[],
                                    /* in */ phrase_token_t token) {
    assert(NULL != m_db);
    assert(NULL != m_entry);

    DBT db_key;
    memset(&db_key, 0, sizeof(DBT));
    db_key.data = (void *) phrase;
    db_key.size = phrase_length * sizeof(ucs4_t);

    DBT db_data;
    memset(&db_data, 0, sizeof(DBT));
    int ret = m_db->get(m_db, NULL, &db_key, &db_data, 0);
    if (ret != 0)
        return ERROR_REMOVE_ITEM_DONOT_EXISTS;

    m_entry->m_chunk.set_chunk(db_data.data, db_data.size, NULL);

    int result = m_entry->remove_index(token);
    if (ERROR_OK != result)
        return result;

    /* removed the token. */
    memset(&db_data, 0, sizeof(DBT));
    db_data.data = m_entry->m_chunk.begin();
    db_data.size = m_entry->m_chunk.size();

    ret = m_db->put(m_db, NULL, &db_key, &db_data, 0);
    if (ret != 0)
        return ERROR_FILE_CORRUPTION;

    return ERROR_OK;
}

/* mask out method */
bool PhraseLargeTable3::mask_out(phrase_token_t mask,
                                 phrase_token_t value) {
    PhraseTableEntry entry;

    DBC * cursorp = NULL;
    DBT db_key, db_data;

    /* Get a cursor */
    m_db->cursor(m_db, NULL, &cursorp, 0);

    if (NULL == cursorp)
        return false;

    /* Initialize our DBTs. */
    memset(&db_key, 0, sizeof(DBT));
    memset(&db_data, 0, sizeof(DBT));

    /* Iterate over the database, retrieving each record in turn. */
    int ret = 0;
    while((ret = cursorp->c_get(cursorp, &db_key, &db_data, DB_NEXT)) == 0) {
        entry.m_chunk.set_chunk(db_data.data, db_data.size, NULL);

        entry.mask_out(mask, value);

        memset(&db_data, 0, sizeof(DBT));
        db_data.data = entry.m_chunk.begin();
        db_data.size = entry.m_chunk.size();
        int ret = cursorp->put(cursorp, &db_key, &db_data,  DB_CURRENT);
        assert(ret == 0);

        /* Initialize our DBTs. */
        memset(&db_key, 0, sizeof(DBT));
        memset(&db_data, 0, sizeof(DBT));
    }
    assert(ret == DB_NOTFOUND);

    /* Cursors must be closed */
    if (cursorp != NULL)
        cursorp->c_close(cursorp);

    m_db->sync(m_db, 0);

    return true;
}

};
