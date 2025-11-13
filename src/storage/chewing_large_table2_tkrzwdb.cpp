/* 
 *  libpinyin
 *  Library to deal with pinyin.
 *  
 *  Copyright (C) 2025 Peng Wu <alexepico@gmail.com>
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

#include "chewing_large_table2.h"
#include <tkrzw_dbm_baby.h>
#include <tkrzw_dbm_tree.h>
#include <tkrzw_str_util.h>
#include "pinyin_utils.h"
#include "tkrzwdb_utils.h"

using namespace tkrzw;

namespace pinyin {

bool tkrzw_chewing_continue_search(const std::string_view& query_key,
                                   const std::string_view& db_key) {

    int lhs_chewing_length = query_key.size() / sizeof(ChewingKey);
    int rhs_chewing_length = db_key.size() / sizeof(ChewingKey);

    ChewingKey* lhs_chewing = (ChewingKey*) query_key.data();
    ChewingKey* rhs_chewing = (ChewingKey*) db_key.data();

    /* The key in dbm is longer than the key in application. */
    if (lhs_chewing_length >= rhs_chewing_length)
        return false;

    int min_chewing_length = lhs_chewing_length;

    int result = pinyin_exact_compare2
        (lhs_chewing, rhs_chewing, min_chewing_length);
    if (0 != result)
        return false;

    /* continue the longer chewing search. */
    return true;
}

ChewingLargeTable2::ChewingLargeTable2() {
    m_db = new BabyDBM;

    m_entries = NULL;
    init_entries();
}

void ChewingLargeTable2::reset() {
    if (m_db) {
        m_db->Synchronize(false);
        m_db->Close();
        delete m_db;
        m_db = NULL;
    }

    fini_entries();
}

/* attach method */
bool ChewingLargeTable2::attach(const char * dbfile, guint32 flags) {
    bool writable = false;

    reset();

    init_entries();

    int32_t options = attach_options(flags, writable);

    if (!dbfile)
        return false;

    m_db = new TreeDBM;
    return m_db->Open(dbfile, writable, options).IsOK();
}

/* load_db/store_db method */
bool ChewingLargeTable2::load_db(const char * filename) {
    reset();

    init_entries();

    /* create in-memory db. */
    m_db = new BabyDBM;

    TreeDBM tmp_db;
    if (tmp_db.Open(filename, false, File::OPEN_NO_CREATE) != Status::SUCCESS)
        return false;

    copy_tkrzwdb(&tmp_db, m_db);

    tmp_db.Close();

    return true;
}

bool ChewingLargeTable2::store_db(const char * new_filename) {
    if (!m_db)
        return false;

    Status status = RemoveFile(new_filename);

    if (status != Status::SUCCESS && status != Status::NOT_FOUND_ERROR)
        return false;

    TreeDBM tmp_db;
    if (tmp_db.Open(new_filename, true, File::OPEN_DEFAULT) != Status::SUCCESS)
        return false;

    copy_tkrzwdb(m_db, &tmp_db);

    tmp_db.Synchronize(false);
    tmp_db.Close();

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

    std::string_view key(reinterpret_cast<const char*>(index), phrase_length * sizeof(ChewingKey));
    std::string value;

    Status status = m_db->Get(key, &value);

    if (!status.IsOK())
        return result;

    /* continue searching. */
    result |= SEARCH_CONTINUED;
    if (value.empty())
        return result;

    entry->m_chunk.set_size(value.size());
    memcpy(entry->m_chunk.begin(), value.data(), value.size());

    result = entry->search(keys, ranges) | result;

    return result;
}

int ChewingLargeTable2::search_internal(int phrase_length,
                                        const ChewingKey index[],
                                        const ChewingKey keys[],
                                        PhraseIndexRanges ranges) const {
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
        abort();
    }

#undef CASE

    return SEARCH_NONE;
}

template<int phrase_length>
int ChewingLargeTable2::search_suggestion_internal
(/* in */ const MemoryChunk & chunk,
 int prefix_len,
 /* in */ const ChewingKey prefix_keys[],
 /* out */ PhraseTokens tokens) const {
    int result = SEARCH_NONE;

    ChewingTableEntry<phrase_length> * entry =
        (ChewingTableEntry<phrase_length> *)
        g_ptr_array_index(m_entries, phrase_length);
    assert(NULL != entry);

    entry->m_chunk.set_chunk(chunk.begin(), chunk.size(), NULL);

    result = entry->search_suggestion(prefix_len, prefix_keys, tokens) | result;

    entry->m_chunk.set_size(0);

    return result;
}

int ChewingLargeTable2::search_suggestion_internal
(int phrase_length,
 /* in */ const MemoryChunk & chunk,
 int prefix_len,
 /* in */ const ChewingKey prefix_keys[],
 /* out */ PhraseTokens tokens) const {

#define CASE(len) case len:                             \
    {                                                   \
        return search_suggestion_internal<len>          \
            (chunk, prefix_len, prefix_keys, tokens);   \
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
        abort();
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

    std::string_view key(reinterpret_cast<const char*>(index), phrase_length * sizeof(ChewingKey));
    std::string value;

    Status status = m_db->Get(key, &value);

    if (!status.IsOK()) {
        /* new entry. */
        ChewingTableEntry<phrase_length> entry;
        entry.add_index(keys, token);

        std::string_view value(reinterpret_cast<const char*>(entry.m_chunk.begin()), entry.m_chunk.size());

        if (!m_db->Set(key, value).IsOK())
            return ERROR_FILE_CORRUPTION;

        /* recursively add keys for continued information. */
        for (size_t len = phrase_length - 1; len > 0; --len) {
            std::string_view key(reinterpret_cast<const char*>(index), len * sizeof(ChewingKey));

            if (m_db->Get(key, nullptr).IsOK()) {
                /* found entry. */
                return ERROR_OK;
            }

            /* new entry with empty content. */
            if (!m_db->Set(key, std::string_view(nullptr, 0)).IsOK())
                return ERROR_FILE_CORRUPTION;
        }

        return ERROR_OK;
    }

    /* already have keys. */
    entry->m_chunk.set_size(value.size());
    memcpy(entry->m_chunk.begin(), value.data(), value.size());

    int result = entry->add_index(keys, token);

    /* store the entry. */
    std::string_view new_value(reinterpret_cast<const char*>(entry->m_chunk.begin()), entry->m_chunk.size());
    if (!m_db->Set(key, new_value).IsOK())
        return ERROR_FILE_CORRUPTION;

    return result;
}

int ChewingLargeTable2::add_index_internal(int phrase_length,
                                           const ChewingKey index[],
                                           const ChewingKey keys[],
                                           phrase_token_t token) {
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
        abort();
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

    std::string_view key(reinterpret_cast<const char*>(index), phrase_length * sizeof(ChewingKey));
    std::string value;

    Status status = m_db->Get(key, &value);

    if (!status.IsOK() || value.size() < sizeof(phrase_token_t))
        return ERROR_REMOVE_ITEM_DONOT_EXISTS;

    /* contains at least one index item. */
    entry->m_chunk.set_size(value.size());
    memcpy(entry->m_chunk.begin(), value.data(), value.size());

    int result = entry->remove_index(keys, token);
    if (ERROR_OK != result)
        return result;

    /* store. */
    std::string_view new_value(reinterpret_cast<const char*>(entry->m_chunk.begin()), entry->m_chunk.size());
    if (!m_db->Set(key, new_value).IsOK())
        return ERROR_FILE_CORRUPTION;

    return ERROR_OK;
}

int ChewingLargeTable2::remove_index_internal(int phrase_length,
                                              const ChewingKey index[],
                                              const ChewingKey keys[],
                                              phrase_token_t token) {
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
        abort();
    }

#undef CASE

    return ERROR_FILE_CORRUPTION;
}

/* use MaskOutProcessor2 to avoid linking problem. */
class MaskOutProcessor2 : public DBM::RecordProcessor {
    GPtrArray * m_entries;
    phrase_token_t m_mask;
    phrase_token_t m_value;

public:
    MaskOutProcessor2(GPtrArray * entries,
                      phrase_token_t mask, phrase_token_t value)
        : m_entries(entries), m_mask(mask), m_value(value) {}

    std::string_view ProcessFull(std::string_view key, std::string_view value) override {
        int phrase_length = key.size() / sizeof(ChewingKey);

#define CASE(len) case len:                                             \
        {                                                               \
            ChewingTableEntry<len> * entry =                            \
                (ChewingTableEntry<len> *)                              \
                g_ptr_array_index(m_entries, phrase_length);            \
            assert(NULL != entry);                                      \
                                                                        \
            entry->m_chunk.set_content                                  \
                (0, const_cast<char*>(value.data()), value.size());     \
            entry->mask_out(m_mask, m_value);                           \
                                                                        \
            return std::string_view                                     \
                (reinterpret_cast<const char*>(entry->m_chunk.begin()), \
                 entry->m_chunk.size());                                \
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
            abort();
        }

#undef CASE

        abort();
        return NOOP;
    }

    std::string_view ProcessEmpty(std::string_view key) override {
        return NOOP;
    }
};

/* mask out method */
bool ChewingLargeTable2::mask_out(phrase_token_t mask,
                                  phrase_token_t value) {
    if (!m_db) return false;

    MaskOutProcessor2 processor(m_entries, mask, value);

    m_db->ProcessEach(&processor, true);

    m_db->Synchronize(false);
    return true;
}

/* search_suggestion method */
int ChewingLargeTable2::search_suggestion
(int prefix_len,
 /* in */ const ChewingKey prefix_keys[],
 /* out */ PhraseTokens tokens) const {
    ChewingKey index[MAX_PHRASE_LENGTH];
    int result = SEARCH_NONE;

    if (NULL == m_db)
        return result;

    if (contains_incomplete_pinyin(prefix_keys, prefix_len))
        compute_incomplete_chewing_index(prefix_keys, index, prefix_len);
    else
        compute_chewing_index(prefix_keys, index, prefix_len);

    std::string_view query_key(reinterpret_cast<const char*>(index), prefix_len * sizeof(ChewingKey));

    if (!m_db->Get(query_key, nullptr).IsOK())
        return result;

    std::unique_ptr<DBM::Iterator> iter = m_db->MakeIterator();

    if (!iter->Jump(query_key).IsOK())
        return result;
    /* Get the next entry */
    if (!iter->Next().IsOK())
        return result;

    std::string key, value;
    MemoryChunk chunk;
    while (iter->Get(&key, &value).IsOK()) {
        std::string_view db_key(key);

        if (!tkrzw_chewing_continue_search(query_key, db_key))
            break;

        int phrase_length = key.size() / sizeof(ChewingKey);
        chunk.set_chunk(const_cast<char*>(value.data()), value.size(), NULL);

        result = search_suggestion_internal
            (phrase_length, chunk, prefix_len, prefix_keys, tokens) | result;

        chunk.set_size(0);

        iter->Next();
    }

    return result;
}

};
