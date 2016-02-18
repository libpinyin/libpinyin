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

/* search method */
int PhraseLargeTable3::search(int phrase_length,
                              /* in */ const ucs4_t phrase[],
                              /* out */ PhraseTokens tokens) const {
    int result = SEARCH_NONE;

    if (NULL == m_db)
        return result;
    assert(NULL != m_entry);

    const char * kbuf = (char *) phrase;
    const int32_t vsiz = m_db->check(kbuf, phrase_length * sizeof(ucs4_t));
    /* -1 on failure. */
    if (-1 == vsiz)
        return result;

    /* continue searching. */
    result |= SEARCH_CONTINUED;
    if (0 == vsiz)
        return result;

    m_entry->m_chunk.set_size(vsiz);
    char * vbuf = (char *) m_entry->m_chunk.begin();
    assert (vsiz == m_db->get(kbuf, phrase_length * sizeof(ucs4_t),
                              vbuf, vsiz));

    result = m_entry->search(tokens) | result;

    return result;
}

/* add_index/remove_index method */
int PhraseLargeTable3::add_index(int phrase_length,
                                 /* in */ const ucs4_t phrase[],
                                 /* in */ phrase_token_t token) {
    assert(NULL != m_db);
    assert(NULL != m_entry);

    bool retval = false;

    /* load phrase table entry. */
    const char * kbuf = (char *) phrase;
    size_t ksiz = phrase_length * sizeof(ucs4_t);
    int32_t vsiz = m_db->check(kbuf, ksiz);
    char * vbuf = NULL;
    if (-1 == vsiz) {
        /* new entry. */
        PhraseTableEntry entry;
        entry.add_index(token);

        vbuf = (char *) entry.m_chunk.begin();
        vsiz = entry.m_chunk.size();
        retval = m_db->set(kbuf, ksiz, vbuf, vsiz);
        if (!retval)
            return ERROR_FILE_CORRUPTION;

        /* recursively add keys for continued information. */
        for (size_t len = phrase_length - 1; len > 0; --len) {
            ksiz = len * sizeof(ucs4_t);

            vsiz = m_db->check(kbuf, ksiz);
            /* found entry. */
            if (-1 != vsiz)
                return ERROR_OK;

            /* new entry with empty content. */
            retval = m_db->set(kbuf, ksiz, empty_vbuf, 0);
            if (!retval)
                return ERROR_FILE_CORRUPTION;
        }

        return ERROR_OK;
    }

    /* already have keys. */
    m_entry->m_chunk.set_size(vsiz);
    /* m_chunk may re-allocate here. */
    vbuf = (char *) m_entry->m_chunk.begin();
    assert(vsiz == m_db->get(kbuf, ksiz, vbuf, vsiz));

    int result = m_entry->add_index(token);

    /* store the entry. */
    vbuf = (char *) m_entry->m_chunk.begin();
    vsiz = m_entry->m_chunk.size();
    retval = m_db->set(kbuf, ksiz, vbuf, vsiz);
    if (!retval)
        return ERROR_FILE_CORRUPTION;

    return result;
}

};
