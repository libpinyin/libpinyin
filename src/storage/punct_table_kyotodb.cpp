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
#include <kchashdb.h>
#include <kccachedb.h>
#include "kyotodb_utils.h"

using namespace kyotocabinet;
using namespace pinyin;

PunctTable::PunctTable() {
    /* create in-memory db. */
    m_db = new ProtoTreeDB;
    check_result(m_db->open("-", BasicDB::OREADER|BasicDB::OWRITER|BasicDB::OCREATE));

    m_entry = new PunctTableEntry;
}

void PunctTable::reset() {
    if (m_db) {
        m_db->synchronize();
        m_db->close();
        delete m_db;
        m_db = NULL;
    }

    if (m_entry) {
        delete m_entry;
        m_entry = NULL;
    }
}

/* attach method */
bool PunctTable::attach(const char * dbfile, guint32 flags) {
    reset();

    m_entry = new PunctTableEntry;

    uint32_t mode = attach_options(flags);

    if (!dbfile)
        return false;

    m_db = new TreeDB;

    return m_db->open(dbfile, mode);
}

/* load_db/save_db method */
/* use in-memory DBM here, for better performance. */
bool PunctTable::load_db(const char * filename) {
    reset();

    m_entry = new PunctTableEntry;

    /* create in-memory db. */
    m_db = new ProtoTreeDB;

    if (!m_db->open("-", BasicDB::OREADER|BasicDB::OWRITER|BasicDB::OCREATE))
        return false;

    if (!m_db->load_snapshot(filename, NULL))
        return false;

#if 0
    /* load db into memory. */
    BasicDB * tmp_db = new TreeDB;
    if (!tmp_db->open(filename, BasicDB::OREADER))
        return false;

    CopyVisitor visitor(m_db);
    tmp_db->iterate(&visitor, false);

    tmp_db->close();
    delete tmp_db;
#endif

    return true;
}

bool PunctTable::save_db(const char * new_filename) {
    int ret = unlink(new_filename);
    if ( ret != 0 && errno != ENOENT)
        return false;

    if (!m_db->dump_snapshot(new_filename, NULL))
        return false;

#if 0
    BasicDB * tmp_db = new TreeDB;
    if (!tmp_db->open(new_filename, BasicDB::OWRITER|BasicDB::OCREATE))
        return false;

    CopyVisitor visitor(tmp_db);
    m_db->iterate(&visitor, false);

    tmp_db->synchronize();
    tmp_db->close();
    delete tmp_db;
#endif

    return true;
}

bool PunctTable::load_entry(phrase_token_t index) {
    if (NULL == m_db)
        return false;
    assert(NULL != m_entry);

    m_entry->m_chunk.set_size(0);

    const char * kbuf = (char *) &index;
    const int32_t vsiz = m_db->check(kbuf, sizeof(phrase_token_t));
    /* -1 on failure. */
    if (-1 == vsiz || 0 == vsiz)
        return true;

    m_entry->m_chunk.set_size(vsiz);
    /* m_chunk may re-allocate here. */
    char * vbuf = (char *) m_entry->m_chunk.begin();
    check_result(vsiz == m_db->get(kbuf, sizeof(phrase_token_t),
                                   vbuf, vsiz));
    return true;
}

bool PunctTable::store_entry(phrase_token_t index) {
    if (NULL == m_db)
        return false;
    assert(NULL != m_entry);

    const char * kbuf = (char *) &index;
    char * vbuf = (char *) m_entry->m_chunk.begin();
    int32_t vsiz = m_entry->m_chunk.size();
    return m_db->set(kbuf, sizeof(phrase_token_t), vbuf, vsiz);
}

bool PunctTable::remove_all_punctuations(/* in */ phrase_token_t index) {
    if (NULL == m_db)
        return false;

    const char * kbuf = (char *) &index;
    return m_db->remove(kbuf, sizeof(phrase_token_t));
}

class KeyCollectVisitor : public DB::Visitor {
private:
    GArray * m_items;
public:
    KeyCollectVisitor(GArray * items) {
        m_items = items;
    }

    virtual const char* visit_full(const char* kbuf, size_t ksiz,
                                   const char* vbuf, size_t vsiz, size_t* sp) {
        assert(ksiz == sizeof(phrase_token_t));
        const phrase_token_t * token = (phrase_token_t *) kbuf;
        g_array_append_val(m_items, *token);
        return NOP;
    }

    virtual const char* visit_empty(const char* kbuf, size_t ksiz, size_t* sp) {
        /* assume no empty record. */
        assert (FALSE);
        return NOP;
    }
};

bool PunctTable::get_all_items(/* out */ GArray * items) {
    g_array_set_size(items, 0);

    if ( !m_db )
        return false;

    KeyCollectVisitor visitor(items);
    m_db->iterate(&visitor, false);

    return true;
}
