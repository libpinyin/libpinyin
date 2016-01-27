/* 
 *  libpinyin
 *  Library to deal with pinyin.
 *  
 *  Copyright (C) 2016 Peng Wu <alexepico@gmail.com>
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

#include "phrase_large_table3.h"
#include <errno.h>


namespace pinyin{

PhraseLargeTable3::PhraseLargeTable3() {
    m_db = NULL;
    m_entry = NULL;
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

    u_int32_t db_flags = 0;

    if (flags & ATTACH_READONLY)
        db_flags |= DB_RDONLY;
    if (flags & ATTACH_READWRITE)
        assert(!(flags & ATTACH_READONLY));
    if (flags & ATTACH_CREATE)
        db_flags |= DB_CREATE;

    if (!dbfile)
        return false;

    int ret = db_create(&m_db, NULL, 0);
    assert(0 == ret);

    ret = m_db->open(m_db, NULL, dbfile, NULL,
                     DB_BTREE, db_flags, 0644);
    if (ret != 0)
        return false;

    m_entry = new PhraseTableEntry;

    return true;
}

/* load/store method */
bool PhraseLargeTable3::load_db(const char * filename) {
    reset();

    /* create in memory db. */
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

    DBC * cursorp = NULL;
    DBT key, data;

    /* Get a cursor */
    tmp_db->cursor(tmp_db, NULL, &cursorp, 0);

    if (NULL == cursorp)
        return false;

    /* Initialize our DBTs. */
    memset(&key, 0, sizeof(DBT));
    memset(&data, 0, sizeof(DBT));

    /* Iterate over the database, retrieving each record in turn. */
    while ((ret = cursorp->c_get(cursorp, &key, &data, DB_NEXT)) == 0) {
        int ret = m_db->put(m_db, NULL, &key, &data, 0);
        assert(ret == 0);
    }
    assert (ret == DB_NOTFOUND);

    /* Cursors must be closed */
    if ( cursorp != NULL )
        cursorp->c_close(cursorp);

    if ( tmp_db != NULL )
        tmp_db->close(tmp_db, 0);

    m_entry = new PhraseTableEntry;

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

    DBC * cursorp = NULL;
    DBT key, data;
    /* Get a cursor */
    m_db->cursor(m_db, NULL, &cursorp, 0);

    if (NULL == cursorp)
        return false;

    /* Initialize our DBTs. */
    memset(&key, 0, sizeof(DBT));
    memset(&data, 0, sizeof(DBT));

    /* Iterate over the database, retrieving each record in turn. */
    while ((ret = cursorp->c_get(cursorp, &key, &data, DB_NEXT)) == 0) {
        int ret = tmp_db->put(tmp_db, NULL, &key, &data, 0);
        assert(ret == 0);
    }
    assert (ret == DB_NOTFOUND);

    /* Cursors must be closed */
    if ( cursorp != NULL )
        cursorp->c_close(cursorp);

    if ( tmp_db != NULL ) {
        tmp_db->sync(m_db, 0);
        tmp_db->close(tmp_db, 0);
    }

    return true;
}


};
