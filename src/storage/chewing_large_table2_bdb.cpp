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

#include "chewing_large_table2.h"
#include "bdb_utils.h"

namespace pinyin{

ChewingLargeTable2::ChewingLargeTable2() {
    /* create in-memory db. */
    m_db = NULL;
    int ret = db_create(&m_db, NULL, 0);
    assert(0 == ret);

    ret = m_db->open(m_db, NULL, NULL, NULL,
                     DB_BTREE, DB_CREATE, 0600);
    assert(0 == ret);

    m_entries = NULL;
    init_entries();
}

void ChewingLargeTable2::init_entries() {
    assert(NULL == m_entries);

    m_entries = g_ptr_array_new();
    /* NULL for the first pointer. */
    g_ptr_array_set_size(m_entries, MAX_PHRASE_LENGTH + 1);

#define CASE(len) case len:                         \
    {                                               \
        ChewingTableEntry<len> * entry =            \
            new ChewingTableEntry<len>;             \
        g_ptr_array_index(m_entries, len) = entry;  \
    }

    for (size_t i = 1; i < m_entries->len; i++) {
        switch(i) {
            CASE(1);
            CASE(2);
            CASE(3);
            CASE(4);
            CASE(5);
            CASE(6);
            CASE(7);
            CASE(8);
            CASE(9);
            CASE(10);
            CASE(11);
            CASE(12);
            CASE(13);
            CASE(14);
            CASE(15);
            CASE(16);
        default:
            assert(false);
        }
    }

#undef CASE
}

void ChewingLargeTable2::reset() {
    if (m_db) {
        m_db->sync(m_db, 0);
        m_db->close(m_db, 0);
        m_db = NULL;
    }

#define CASE(len) case len:                     \
    {                                           \
        ChewingTableEntry<len> * entry =        \
            (ChewingTableEntry<len> *)          \
            g_ptr_array_index(m_entries, len);  \
        delete entry;                           \
    }

    if (m_entries) {
        assert(MAX_PHRASE_LENGTH + 1 == m_entries->len);

        for (size_t i = 1; i < m_entries->len; i++) {
            switch(i) {
                CASE(1);
                CASE(2);
                CASE(3);
                CASE(4);
                CASE(5);
                CASE(6);
                CASE(7);
                CASE(8);
                CASE(9);
                CASE(10);
                CASE(11);
                CASE(12);
                CASE(13);
                CASE(14);
                CASE(15);
                CASE(16);
            default:
                assert(false);
            }
        }

        g_ptr_array_free(m_entries, TRUE);
        m_entries = NULL;
    }

#undef CASE
}

};
