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

template<int phrase_length>
int ChewingLargeTable2::search_internal(/* in */ const ChewingKey index[],
                                        /* in */ const ChewingKey keys[],
                                        /* out */ PhraseIndexRanges ranges) const {
    int result = SEARCH_NONE;

    ChewingTableEntry<phrase_length> * entry =
        (ChewingTableEntry<phrase_length> *)
        g_ptr_array_index(m_entries, phrase_length);
    assert(NULL != entry);

    const char * kbuf = (char *) index;
    const int32_t vsiz = m_db->check(kbuf, phrase_length * sizeof(ChewingKey));
    /* -1 on failure. */
    if (-1 == vsiz)
        return result;

    /* continue searching. */
    result |= SEARCH_CONTINUED;
    if (0 == vsiz)
        return result;

    entry->m_chunk.set_size(vsiz);
    /* m_chunk may re-allocate here. */
    char * vbuf = (char *) entry->m_chunk.begin();
    assert(vsiz == m_db->get(kbuf, phrase_length * sizeof(ChewingKey),
                             vbuf, vsiz));

    result = entry->search(keys, ranges) | result;

    return result;
}

int ChewingLargeTable2::search_internal(int phrase_length,
                                        /* in */ const ChewingKey index[],
                                        /* in */ const ChewingKey keys[],
                                        /* out */ PhraseIndexRanges ranges) const {
#define CASE(len) case len:                                 \
    {                                                       \
        return search_internal<len>(index, keys, ranges);   \
    }

    switch(phrase_length) {
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

#undef CASE

    return SEARCH_NONE;
}

template<int phrase_length>
int ChewingLargeTable2::add_index_internal(/* in */ const ChewingKey index[],
                                           /* in */ const ChewingKey keys[],
                                           /* in */ phrase_token_t token) {
    ChewingTableEntry<phrase_length> * entry =
        (ChewingTableEntry<phrase_length> *)
        g_ptr_array_index(m_entries, phrase_length);
    assert(NULL != entry);

    bool retval = false;

    /* load chewing table entry. */
    const char * kbuf = (char *) index;
    size_t ksiz = phrase_length * sizeof(ChewingKey);
    char * vbuf = NULL;
    int32_t vsiz = m_db->check(kbuf, ksiz);
    if (-1 == vsiz) {
        /* new entry. */
        ChewingTableEntry<phrase_length> new_entry;
        new_entry.add_index(keys, token);

        vbuf = (char *) new_entry.m_chunk.begin();
        vsiz = new_entry.m_chunk.size();
        retval = m_db->set(kbuf, ksiz, vbuf, vsiz);
        if (!retval)
            return ERROR_FILE_CORRUPTION;

        /* recursively add keys for continued information. */
        for (size_t len = phrase_length - 1; len > 0; --len) {
            ksiz = len * sizeof(ChewingKey);

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
    entry->m_chunk.set_size(vsiz);
    /* m_chunk may re-allocate here. */
    vbuf = (char *) entry->m_chunk.begin();
    assert(vsiz = m_db->get(kbuf, ksiz, vbuf, vsiz));

    int result = entry->add_index(keys, token);

    /* store the entry. */
    vbuf = (char *) entry->m_chunk.begin();
    vsiz = entry->m_chunk.size();
    retval = m_db->set(kbuf, ksiz, vbuf, vsiz);
    if (!retval)
        return ERROR_FILE_CORRUPTION;

    return result;
}

int ChewingLargeTable2::add_index_internal(int phrase_length,
                                           /* in */ const ChewingKey index[],
                                           /* in */ const ChewingKey keys[],
                                           /* in */ phrase_token_t token) {
#define CASE(len) case len:                                     \
    {                                                           \
        return add_index_internal<len>(index, keys, token);     \
    }

    switch(phrase_length) {
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

#undef CASE

    return ERROR_FILE_CORRUPTION;
}

template<int phrase_length>
int ChewingLargeTable2::remove_index_internal(/* in */ const ChewingKey index[],
                                              /* in */ const ChewingKey keys[],
                                              /* in */ phrase_token_t token) {
    ChewingTableEntry<phrase_length> * entry =
        (ChewingTableEntry<phrase_length> *)
        g_ptr_array_index(m_entries, phrase_length);
    assert(NULL != entry);

    const char * kbuf = (char *) index;
    const size_t ksiz = phrase_length * sizeof(ChewingKey);
    char * vbuf = NULL;
    int32_t vsiz = m_db->check(kbuf, ksiz);
    if (vsiz < (signed) sizeof(phrase_token_t))
        return ERROR_REMOVE_ITEM_DONOT_EXISTS;

    /* contains at least one index item. */
    entry->m_chunk.set_size(vsiz);
    /* m_chunk may re-allocate here. */
    vbuf = (char *) entry->m_chunk.begin();
    assert(vsiz == m_db->get(kbuf, ksiz, vbuf, vsiz));

    int result = entry->remove_index(keys, token);
    if (ERROR_OK != result)
        return result;

    /* for safety. */
    vbuf = (char *) entry->m_chunk.begin();
    vsiz = entry->m_chunk.size();

    if (!m_db->set(kbuf, ksiz, vbuf, vsiz))
        return ERROR_FILE_CORRUPTION;

    return ERROR_OK;
}

int ChewingLargeTable2::remove_index_internal(int phrase_length,
                                              /* in */ const ChewingKey index[],
                                              /* in */ const ChewingKey keys[],
                                              /* in */ phrase_token_t token) {
#define CASE(len) case len:                                     \
    {                                                           \
        return remove_index_internal<len>(index, keys, token);  \
    }

    switch(phrase_length) {
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

#undef CASE

    return ERROR_FILE_CORRUPTION;
}

};
