/* 
 *  libpinyin
 *  Library to deal with pinyin.
 *  
 *  Copyright (C) 2024 Peng Wu <alexepico@gmail.com>
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


#include "punct_table.h"
#include <errno.h>
#include "bdb_utils.h"

using namespace pinyin;

PunctTable::PunctTable() {
    /* create in-memory db. */
    m_db = NULL;
    int ret = db_create(&m_db, NULL, 0);
    assert(0 == ret);

    ret = m_db->open(m_db, NULL, NULL, NULL,
                     DB_BTREE, DB_CREATE, 0600);
    assert(0 == ret);

    m_entry = new PunctTableEntry();
}

void PunctTable::reset() {
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

bool PunctTable::attach(const char * dbfile, guint32 flags) {
    reset();

    m_entry = new PunctTableEntry();

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

bool PunctTable::load_db(const char * dbfile) {
    reset();

    m_entry = new PunctTableEntry;

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

    ret = tmp_db->open(tmp_db, NULL, dbfile, NULL,
                       DB_BTREE, DB_RDONLY, 0600);
    if (ret != 0)
        return false;

    if (!copy_bdb(tmp_db, m_db))
        return false;

    if (tmp_db != NULL)
        tmp_db->close(tmp_db, 0);

    return true;
}

bool PunctTable::save_db(const char * dbfile) {
    DB * tmp_db = NULL;

    int ret = unlink(dbfile);
    if (ret != 0 && errno != ENOENT)
        return false;

    ret = db_create(&tmp_db, NULL, 0);
    assert(0 == ret);

    if (NULL == tmp_db)
        return false;

    ret = tmp_db->open(tmp_db, NULL, dbfile, NULL,
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

bool PunctTable::load_entry(phrase_token_t index) {
    if (NULL == m_db)
        return false;
    assert(NULL != m_entry);

    m_entry->m_chunk.set_size(0);

    DBT db_key;
    memset(&db_key, 0, sizeof(DBT));
    db_key.data = (void *) &index;
    db_key.size = sizeof(phrase_token_t);

    DBT db_data;
    memset(&db_data, 0, sizeof(DBT));
    int ret = m_db->get(m_db, NULL, &db_key, &db_data, 0);
    if (ret != 0)
        return true;

    m_entry->m_chunk.set_content(0, db_data.data, db_data.size);
    return true;
}

bool PunctTable::store_entry(phrase_token_t index) {
    if (NULL == m_db)
        return false;
    assert(NULL != m_entry);

    DBT db_key;
    memset(&db_key, 0, sizeof(DBT));
    db_key.data = (void *) &index;
    db_key.size = sizeof(phrase_token_t);

    DBT db_data;
    memset(&db_data, 0, sizeof(DBT));
    db_data.data = m_entry->m_chunk.begin();
    db_data.size = m_entry->m_chunk.size();
    int ret = m_db->put(m_db, NULL, &db_key, &db_data, 0);
    if (ret != 0)
        return false;
    return true;
}

bool PunctTable::remove_all_punctuations(/* in */ phrase_token_t index) {
    if (NULL == m_db)
        return false;

    DBT db_key;
    memset(&db_key, 0, sizeof(DBT));
    db_key.data = (void *) &index;
    db_key.size = sizeof(phrase_token_t);

    int ret = m_db->del(m_db, NULL, &db_key, 0);
    return 0 == ret;
}

bool PunctTable::get_all_items(/* out */ GArray * items) {
    g_array_set_size(items, 0);

    if ( !m_db )
        return false;

    DBC * cursorp = NULL;
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
    while ((ret = cursorp->c_get(cursorp, &key, &data, DB_NEXT)) == 0) {
        assert(key.size == sizeof(phrase_token_t));
        phrase_token_t * token = (phrase_token_t *)key.data;
        g_array_append_val(items, *token);

        /* Initialize our DBTs. */
        memset(&key, 0, sizeof(DBT));
        memset(&data, 0, sizeof(DBT));
    }

    assert (ret == DB_NOTFOUND);

    /* Cursors must be closed */
    if (cursorp != NULL)
        cursorp->c_close(cursorp);

    return true;
}
