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
#include "kyotodb_utils.h"


using namespace kyotocabinet;

namespace pinyin{

PhraseLargeTable3::PhraseLargeTable3() {
    /* create in-memory db. */
    m_db = new ProtoTreeDB;
    assert(m_db->open("-", BasicDB::OREADER|BasicDB::OWRITER|BasicDB::OCREATE));

    m_entry = new PhraseTableEntry;
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
    uint32_t mode = attach_options(flags);

    if (!dbfile)
        return false;

    m_db = new TreeDB;
    m_entry = new PhraseTableEntry;

    return m_db->open(dbfile, mode);
}

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

    CopyVisitor visitor(m_db);
    tmp_db->iterate(&visitor, false);

    tmp_db->close();
    delete tmp_db;

    m_entry = new PhraseTableEntry;

    return true;
}

bool PhraseLargeTable3::store_db(const char * new_filename){
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
    /* m_chunk may re-allocate here. */
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
    char * vbuf = NULL;
    int32_t vsiz = m_db->check(kbuf, ksiz);
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

int PhraseLargeTable3::remove_index(int phrase_length,
                                    /* in */ const ucs4_t phrase[],
                                    /* in */ phrase_token_t token) {
    assert(NULL != m_db);
    assert(NULL != m_entry);

    const char * kbuf = (char *) phrase;
    const size_t ksiz = phrase_length * sizeof(ucs4_t);
    char * vbuf = NULL;
    int32_t vsiz = m_db->check(kbuf, ksiz);
    if (vsiz < (signed) sizeof(ucs4_t))
        return ERROR_REMOVE_ITEM_DONOT_EXISTS;

    /* contains at least one token. */
    m_entry->m_chunk.set_size(vsiz);
    /* m_chunk may re-allocate here. */
    vbuf = (char *) m_entry->m_chunk.begin();
    assert(vsiz == m_db->get(kbuf, ksiz, vbuf, vsiz));

    int result = m_entry->remove_index(token);
    if (ERROR_OK != result)
        return result;

    /* for safety. */
    vbuf = (char *) m_entry->m_chunk.begin();
    vsiz = m_entry->m_chunk.size();
    if (!m_db->set(kbuf, ksiz, vbuf, vsiz))
        return ERROR_FILE_CORRUPTION;

    return ERROR_OK;
}


class MaskOutVisitor : public DB::Visitor {
private:
    BasicDB * m_db;
    phrase_token_t m_mask;
    phrase_token_t m_value;

    PhraseTableEntry m_entry;
public:
    MaskOutVisitor(BasicDB * db, phrase_token_t mask, phrase_token_t value) {
        m_db = db;
        m_mask = mask;
        m_value = value;
    }

    virtual const char* visit_full(const char* kbuf, size_t ksiz,
                                   const char* vbuf, size_t vsiz, size_t* sp) {
        m_entry.m_chunk.set_content(0, vbuf, vsiz);
        m_entry.mask_out(m_mask, m_value);

        vbuf = (char *) m_entry.m_chunk.begin();
        vsiz = m_entry.m_chunk.size();
        assert(m_db->set(kbuf, ksiz, vbuf, vsiz));
        return NOP;
    }

    virtual const char* visit_empty(const char* kbuf, size_t ksiz, size_t* sp) {
        m_db->set(kbuf, ksiz, empty_vbuf, 0);
        return NOP;
    }
};

/* mask out method */
/* assume it is in-memory dbm. */
bool PhraseLargeTable3::mask_out(phrase_token_t mask,
                                 phrase_token_t value) {
    /* use copy and sweep algorithm here. */
    BasicDB * tmp_db = new ProtoTreeDB;
    if (!tmp_db->open("-", BasicDB::OREADER|BasicDB::OWRITER|BasicDB::OCREATE))
        return false;

    MaskOutVisitor visitor(tmp_db, mask, value);
    m_db->iterate(&visitor, false);

    reset();

    m_db = tmp_db;
    m_entry = new PhraseTableEntry;

    return true;
}

};
