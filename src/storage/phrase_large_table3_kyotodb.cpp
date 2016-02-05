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

#include "phrase_large_table3_kyotodb.h"
#include "phrase_large_table3.h"
#include <kchashdb.h>
#include <kcprotodb.h>


using namespace kyotocabinet;

namespace pinyin{

PhraseLargeTable3::PhraseLargeTable3() {
    m_db = NULL;
    m_entry = NULL;
}

void PhraseLargeTable3::reset() {
    if (m_db) {
        m_db->synchronize();
        m_db->close();
        delete m_db;
        m_db = NULL;
    }

    m_chunk.set_size(0);

    if (m_entry) {
        delete m_entry;
        m_entry = NULL;
    }
}


/* attach method */
bool PhraseLargeTable3::attach(const char * dbfile, guint32 flags) {
    reset();
    uint32_t mode = 0;

    if (flags & ATTACH_READONLY)
        mode |= BasicDB::OREADER;
    if (flags & ATTACH_READWRITE) {
        assert( !( flags & ATTACH_READONLY ) );
        mode |= BasicDB::OREADER | BasicDB::OWRITER;
    }
    if (flags & ATTACH_CREATE)
        mode |= BasicDB::OCREATE;

    if (!dbfile)
        return false;

    m_db = new TreeDB;

    return m_db->open(dbfile, mode);
}


/* Use DB::visitor. */

/* Kyoto Cabinet requires non-NULL pointer for zero length value. */
static const char * empty_vbuf = (char *)UINTPTR_MAX;

/* Use CopyVisitor2 to avoid linking problems. */
class CopyVisitor2 : public DB::Visitor {
private:
    BasicDB * m_db;
public:
    CopyVisitor2(BasicDB * db) {
        m_db = db;
    }

    virtual const char* visit_full(const char* kbuf, size_t ksiz,
                                   const char* vbuf, size_t vsiz, size_t* sp) {
        m_db->set(kbuf, ksiz, vbuf, vsiz);
        return NOP;
    }

    virtual const char* visit_empty(const char* kbuf, size_t ksiz, size_t* sp) {
        m_db->set(kbuf, ksiz, empty_vbuf, 0);
        return NOP;
    }
};

/* load_db/store_db method */
/* use in-memory DBM here, for better performance. */
bool PhraseLargeTable3::load_db(const char * filename) {
    reset();

    /* create in-memory db. */
    m_db = new ProtoTreeDB;

    if (!m_db->open("-", BasicDB::OREADER|BasicDB::OWRITER|BasicDB::OCREATE))
        return false;

    /* load db into memory. */
    BasicDB * tmp_db = new TreeDB;
    if (!tmp_db->open(filename, BasicDB::OREADER))
        return false;

    CopyVisitor2 visitor(m_db);
    tmp_db->iterate(&visitor, false);

    tmp_db->close();
    delete tmp_db;

    return true;
}

bool PhraseLargeTable3::store_db(const char * new_filename){
    int ret = unlink(new_filename);
    if ( ret != 0 && errno != ENOENT)
        return false;

    BasicDB * tmp_db = new TreeDB;
    if (!tmp_db->open(new_filename, BasicDB::OWRITER|BasicDB::OCREATE))
        return false;

    CopyVisitor2 visitor(tmp_db);
    m_db->iterate(&visitor, false);

    tmp_db->synchronize();
    tmp_db->close();
    delete tmp_db;

    return true;
}


};
