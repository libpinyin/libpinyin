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
#include <kchashdb.h>
#include <kcprotodb.h>
#include "kyotodb_utils.h"

using namespace kyotocabinet;

namespace pinyin{

ChewingLargeTable2::ChewingLargeTable2() {
    /* create in-memory db. */
    m_db = new ProtoTreeDB;
    assert(m_db->open("-", BasicDB::OREADER|BasicDB::OWRITER|BasicDB::OCREATE));

    init_entries();
}

void ChewingLargeTable2::reset() {
    if (m_db) {
        m_db->synchronize();
        m_db->close();
        delete m_db;
        m_db = NULL;
    }

    fini_entries();
}

/* attach method */
bool ChewingLargeTable2::attach(const char * dbfile, guint32 flags) {
    reset();

    uint32_t mode = attach_options(flags);

    if (!dbfile)
        return false;

    m_db = new TreeDB;
    init_entries();

    return m_db->open(dbfile, mode);
}

/* load/store method */
/* use in-memory DBM here, for better performance. */
bool ChewingLargeTable2::load_db(const char * filename) {
    reset();

    /* create in-memory db. */
    m_db = new ProtoTreeDB;

    if (!m_db->open("-", BasicDB::OREADER|BasicDB::OWRITER|BasicDB::OCREATE))
        return false;

    /* load db into memory. */
    BasicDB * tmp_db = new TreeDB;
    if (!tmp_db->open(filename, BasicDB::OREADER))
        return false;

    CopyVisitor visitor(m_db);
    tmp_db->iterate(&visitor, false);

    tmp_db->close();
    delete tmp_db;

    init_entries();

    return true;
}

bool ChewingLargeTable2::store_db(const char * new_filename) {
    int ret = unlink(new_filename);
    if ( ret != 0 && errno != ENOENT)
        return false;

    BasicDB * tmp_db = new TreeDB;
    if (!tmp_db->open(new_filename, BasicDB::OWRITER|BasicDB::OCREATE))
        return false;

    CopyVisitor visitor(tmp_db);
    m_db->iterate(&visitor, false);

    tmp_db->synchronize();
    tmp_db->close();
    delete tmp_db;

    return true;
}

};
