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

#ifndef KYOTODB_UTILS_H
#define KYOTODB_UTILS_H

#include <assert.h>
#include <kchashdb.h>
#include <kcprotodb.h>

using namespace kyotocabinet;

namespace pinyin{

inline uint32_t attach_options(guint32 flags) {
    uint32_t mode = 0;

    if (flags & ATTACH_READONLY)
        mode |= BasicDB::OREADER;
    if (flags & ATTACH_READWRITE) {
        assert( !( flags & ATTACH_READONLY ) );
        mode |= BasicDB::OREADER | BasicDB::OWRITER;
    }
    if (flags & ATTACH_CREATE)
        mode |= BasicDB::OCREATE;

    return mode;
}

/* Use DB::visitor. */

/* Kyoto Cabinet requires non-NULL pointer for zero length value. */
static const char * empty_vbuf = (char *)UINTPTR_MAX;

#if 0
class CopyVisitor : public DB::Visitor {
private:
    BasicDB * m_db;
public:
    CopyVisitor(BasicDB * db) {
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
#endif

};

#endif
