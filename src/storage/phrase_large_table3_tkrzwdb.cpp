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

#include "phrase_large_table3.h"
#include <tkrzw_dbm_baby.h>
#include <tkrzw_dbm_tree.h>
#include <tkrzw_str_util.h>
#include "tkrzwdb_utils.h"

using namespace tkrzw;

namespace pinyin {

bool tkrzw_phrase_continue_search(const std::string_view& query_key,
                                  const std::string_view& db_key) {

    /* The key in dbm must be longer than the search key for suggestion. */
    if (db_key.size() <= query_key.size())
        return false;

    /* Check if db_key starts with query_key */
    if (db_key.substr(0, query_key.size()) != query_key)
        return false;

    return true;
}

PhraseLargeTable3::PhraseLargeTable3() {
    m_db = new BabyDBM;
    m_entry = new PhraseTableEntry;
}

void PhraseLargeTable3::reset() {
    if (m_db) {
        m_db->Synchronize(false);
        m_db->Close();
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
    bool writable = false;

    reset();

    m_entry = new PhraseTableEntry;

    int32_t options = attach_options(flags, writable);

    if (!dbfile)
        return false;

    m_db = new TreeDBM;

    return m_db->Open(dbfile, writable, options).IsOK();
}

/* load_db/store_db method */
bool PhraseLargeTable3::load_db(const char * filename) {
    reset();

    m_entry = new PhraseTableEntry;

    /* create in-memory db. */
    m_db = new BabyDBM;

    TreeDBM tmp_db;
    if (tmp_db.Open(filename, false, File::OPEN_NO_CREATE) != Status::SUCCESS)
        return false;

    copy_tkrzwdb(&tmp_db, m_db);

    tmp_db.Close();

    return true;
}

bool PhraseLargeTable3::store_db(const char * new_filename){
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

/* search method */
int PhraseLargeTable3::search(int phrase_length,
                              /* in */ const ucs4_t phrase[],
                              /* out */ PhraseTokens tokens) const {
    int result = SEARCH_NONE;

    if (NULL == m_db)
        return result;
    assert(NULL != m_entry);

    std::string_view key(reinterpret_cast<const char*>(phrase), phrase_length * sizeof(ucs4_t));
    std::string value;

    Status status = m_db->Get(key, &value);

    if (!status.IsOK())
        return result;

    /* continue searching. */
    result |= SEARCH_CONTINUED;
    if (value.empty())
        return result;

    /* Update entry chunk */
    m_entry->m_chunk.set_size(value.size());
    memcpy(m_entry->m_chunk.begin(), value.data(), value.size());

    result = m_entry->search(tokens) | result;

    return result;
}

int PhraseLargeTable3::search_suggestion(int phrase_length,
                                         /* in */ const ucs4_t phrase[],
                                         /* out */ PhraseTokens tokens) const {
    int result = SEARCH_NONE;

    if (NULL == m_db)
        return result;
    assert(NULL != m_entry);

    std::string_view query_key(reinterpret_cast<const char*>(phrase), phrase_length * sizeof(ucs4_t));

    if (!m_db->Get(query_key, nullptr).IsOK())
        return result;

    std::unique_ptr<DBM::Iterator> iter = m_db->MakeIterator();

    if (!iter->Jump(query_key).IsOK())
        return result;
    /* Get the next entry */
    if (!iter->Next().IsOK())
        return result;

    std::string key, value;
    while (iter->Get(&key, &value).IsOK()) {
        if (!tkrzw_phrase_continue_search(query_key, key))
            break;

        m_entry->m_chunk.set_chunk(value.data(), value.size(), NULL);
        result = m_entry->search(tokens) | result;
        m_entry->m_chunk.set_size(0);

        iter->Next();
    }

    return result;
}

/* add_index/remove_index method */
int PhraseLargeTable3::add_index(int phrase_length,
                                 /* in */ const ucs4_t phrase[],
                                 /* in */ phrase_token_t token) {
    assert(NULL != m_db);
    assert(NULL != m_entry);

    std::string_view key(reinterpret_cast<const char*>(phrase), phrase_length * sizeof(ucs4_t));
    std::string value;

    Status status = m_db->Get(key, &value);

    if (!status.IsOK()) {
        /* new entry. */
        PhraseTableEntry entry;
        entry.add_index(token);

        std::string_view value(reinterpret_cast<const char*>(entry.m_chunk.begin()), entry.m_chunk.size());

        if (!m_db->Set(key, value).IsOK())
            return ERROR_FILE_CORRUPTION;

        /* recursively add keys for continued information (prefix entries). */
        for (size_t len = phrase_length - 1; len > 0; --len) {
            std::string_view key(reinterpret_cast<const char*>(phrase), len * sizeof(ucs4_t));

            if (m_db->Get(key, nullptr).IsOK())
                 return ERROR_OK;

            /* new entry with empty content. */
            if (!m_db->Set(key, std::string_view(nullptr, 0)).IsOK())
                return ERROR_FILE_CORRUPTION;
        }

        return ERROR_OK;
    }

    /* already have keys. */
    m_entry->m_chunk.set_size(value.size());
    memcpy(m_entry->m_chunk.begin(), value.data(), value.size());

    int result = m_entry->add_index(token);

    /* store the entry. */
    std::string_view new_value(reinterpret_cast<const char*>(m_entry->m_chunk.begin()), m_entry->m_chunk.size());

    if (!m_db->Set(key, new_value).IsOK())
        return ERROR_FILE_CORRUPTION;

    return result;
}

int PhraseLargeTable3::remove_index(int phrase_length,
                                    /* in */ const ucs4_t phrase[],
                                    /* in */ phrase_token_t token) {
    assert(NULL != m_db);
    assert(NULL != m_entry);

    std::string_view key(reinterpret_cast<const char*>(phrase), phrase_length * sizeof(ucs4_t));
    std::string value;

    Status status = m_db->Get(key, &value);

    if (!status.IsOK())
        return ERROR_REMOVE_ITEM_DONOT_EXISTS;

    if (value.size() < sizeof(phrase_token_t))
         return ERROR_REMOVE_ITEM_DONOT_EXISTS;

    /* contains at least one token. */
    m_entry->m_chunk.set_size(value.size());
    memcpy(m_entry->m_chunk.begin(), value.data(), value.size());

    int result = m_entry->remove_index(token);
    if (ERROR_OK != result)
        return result;

    std::string_view new_value(reinterpret_cast<const char*>(m_entry->m_chunk.begin()), m_entry->m_chunk.size());

    if (!m_db->Set(key, new_value).IsOK())
        return ERROR_FILE_CORRUPTION;

    return ERROR_OK;
}

class MaskOutProcessor : public DBM::RecordProcessor {
private:
    phrase_token_t m_mask;
    phrase_token_t m_value;

    PhraseTableEntry m_entry;

public:
    MaskOutProcessor(phrase_token_t mask, phrase_token_t value)
        : m_mask(mask), m_value(value) {}

    std::string_view ProcessFull(std::string_view key, std::string_view value) override {
        m_entry.m_chunk.set_content(0, const_cast<char*>(value.data()), value.size());
        m_entry.mask_out(m_mask, m_value);

        return std::string_view(
            reinterpret_cast<const char*>(m_entry.m_chunk.begin()),
            m_entry.m_chunk.size()
        );
    }

    std::string_view ProcessEmpty(std::string_view key) override {
        return NOOP;
    }
};

/* mask out method */
bool PhraseLargeTable3::mask_out(phrase_token_t mask,
                                 phrase_token_t value) {
    if (!m_db) return false;

    MaskOutProcessor processor(mask, value);

    m_db->ProcessEach(&processor, true);

    m_db->Synchronize(false);

    return true;
}

};
