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
#include <tkrzw/dbm.h>
#include <tkrzw/tree_dbm.h>
#include <tkrzw/file_util.h>
#include <string>
#include <string_view>
#include <memory>
#include <assert.h>
#include <string.h>
#include <errno.h>
#include "tkrzwdb_utils.h"

using namespace tkrzw;
using namespace pinyin;

PunctTable::PunctTable() {
    /* create in-memory db. */
    m_db = new BabyDBM;

    m_entry = new PunctTableEntry;
}

void PunctTable::reset() {
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
bool PunctTable::attach(const char * dbfile, guint32 flags) {
    bool writable =  false;

    reset();

    m_entry = new PunctTableEntry;

    int32_t options = attach_options(flags, writable);

    if (!dbfile)
        return false;

    m_db = new TreeDBM;

    return m_db->Open(dbfile, writable, options) == Status::SUCCESS;
}

/* load_db/save_db method */
/* use in-memory DBM here, for better performance. */
bool PunctTable::load_db(const char * filename) {
    reset();

    m_entry = new PunctTableEntry;

    /* create in-memory db. */
    m_db = new BabyDBM;

    TreeDBM tmp_db;
    if (tmp_db.Open(filename, false, File::OPEN_DEFAULT) != Status::SUCCESS)
        return false;

    copy_tkrzwdb(&tmp_db, m_db);

    tmp_db.Close();

    return true;
}

bool PunctTable::save_db(const char * new_filename) {
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

bool PunctTable::load_entry(phrase_token_t index) {
    if (NULL == m_db)
        return false;
    assert(NULL != m_entry);

    m_entry->m_chunk.set_size(0);

    const std::string_view kbuf(reinterpret_cast<const char*>(&index), sizeof(index));

    std::string value;
    const Status status = m_db->Get(kbuf, &value);

    if (status == Status::NOT_FOUND_ERROR || 0 == value.size())
        return true;

    if (status != Status::SUCCESS)
        return false;

    m_entry->m_chunk.set_size(value.size());
    char * vbuf = (char *) m_entry->m_chunk.begin();
    memcpy(vbuf, value.data(), value.size());

    return true;
}

bool PunctTable::store_entry(phrase_token_t index) {
    if (NULL == m_db)
        return false;
    assert(NULL != m_entry);

    const std::string_view kbuf(reinterpret_cast<const char*>(&index), sizeof(index));
    const std::string_view vbuf(reinterpret_cast<const char*>(m_entry->m_chunk.begin()),
                                m_entry->m_chunk.size());

    return m_db->Set(kbuf, vbuf) == Status::SUCCESS;
}

bool PunctTable::remove_all_punctuations(/* in */ phrase_token_t index) {
    if (NULL == m_db)
        return false;

    const std::string_view kbuf(reinterpret_cast<const char*>(&index), sizeof(index));

    const Status status = m_db->Remove(kbuf);

    return status == Status::SUCCESS || status == Status::NOT_FOUND_ERROR;
}

bool PunctTable::get_all_items(/* out */ GArray * items) {
    g_array_set_size(items, 0);

    if ( !m_db )
        return false;

    std::unique_ptr<DBM::Iterator> iter(m_db->MakeIterator());
    if (!iter)
        return false;

    iter->First();

    std::string key;

    while (iter->Get(&key).IsOK()) {
        assert(key.size() == sizeof(phrase_token_t));
        const phrase_token_t * token = reinterpret_cast<const phrase_token_t *>(key.data());
        g_array_append_val(items, *token);

        iter->Next();
    }

    return true;
}
