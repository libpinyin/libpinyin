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

#include "ngram.h"
#include <assert.h>
#include <errno.h>
#include <tkrzw_dbm_hash.h>
#include <tkrzw_dbm_tiny.h>
#include <tkrzw_str_util.h>
#include "tkrzwdb_utils.h"

using namespace pinyin;
using namespace tkrzw;

Bigram::Bigram(){
    m_db = NULL;
}

Bigram::~Bigram(){
    reset();
}

void Bigram::reset(){
    if ( m_db ){
        m_db->Close();
        delete m_db;
        m_db = NULL;
    }
}

bool Bigram::load_db(const char * dbfile){
    reset();

    /* create in-memory db. */
    m_db = new TinyDBM;

    HashDBM tmp_db;
    if (tmp_db.Open(dbfile, false, File::OPEN_NO_CREATE) != Status::SUCCESS)
        return false;

    copy_tkrzwdb(&tmp_db, m_db);

    tmp_db.Close();

    return true;
}

bool Bigram::save_db(const char * dbfile){
    if (!m_db)
        return false;

    Status status = RemoveFile(dbfile);

    if (status != Status::SUCCESS && status != Status::NOT_FOUND_ERROR)
        return false;

    HashDBM tmp_db;
    if (tmp_db.Open(dbfile, true, File::OPEN_DEFAULT) != Status::SUCCESS)
        return false;

    copy_tkrzwdb(m_db, &tmp_db);

    tmp_db.Synchronize(false);
    tmp_db.Close();

    return true;
}

bool Bigram::attach(const char * dbfile, guint32 flags){
    bool writable = false;

    reset();

    int32_t options = attach_options(flags, writable);

    if (!dbfile)
        return false;

    m_db = new HashDBM;

    return m_db->Open(dbfile, writable, options).IsOK();
}

/* Use DB interface. */
bool Bigram::load(phrase_token_t index, SingleGram * & single_gram,
                  bool copy){
    single_gram = NULL;
    if ( !m_db )
        return false;

    std::string_view key(reinterpret_cast<const char*>(&index), sizeof(phrase_token_t));
    std::string value;

    Status status = m_db->Get(key, &value);

    if (!status.IsOK())
        return false;

    size_t vsiz = value.size();
    m_chunk.set_size(vsiz);
    memcpy(m_chunk.begin(), value.data(), vsiz);

    single_gram = new SingleGram(m_chunk.begin(), vsiz, copy);
    return true;
}

bool Bigram::store(phrase_token_t index, SingleGram * single_gram){
    if ( !m_db )
        return false;

    std::string_view key(reinterpret_cast<const char*>(&index), sizeof(phrase_token_t));
    std::string_view value(reinterpret_cast<const char*>(single_gram->m_chunk.begin()),
                           single_gram->m_chunk.size());

    return m_db->Set(key, value).IsOK();
}

bool Bigram::remove(/* in */ phrase_token_t index){
    if ( !m_db )
        return false;

    std::string_view key(reinterpret_cast<const char*>(&index), sizeof(phrase_token_t));
    return m_db->Remove(key).IsOK();
}

class KeyCollectProcessor : public DBM::RecordProcessor {
private:
    GArray * m_items;
public:
    KeyCollectProcessor(GArray * items) : m_items(items) {}

    std::string_view ProcessFull(std::string_view key, std::string_view value) override {
        assert(key.size() == sizeof(phrase_token_t));
        const phrase_token_t * token = reinterpret_cast<const phrase_token_t *>(key.data());
        g_array_append_val(m_items, *token);
        return NOOP;
    }

    std::string_view ProcessEmpty(std::string_view key) override {
        /* assume no empty record. */
        assert (FALSE);
        return NOOP;
    }
};

bool Bigram::get_all_items(GArray * items){
    g_array_set_size(items, 0);

    if ( !m_db )
        return false;

    KeyCollectProcessor processor(items);

    Status status = m_db->ProcessEach(&processor, false);

    return status.IsOK();
}

/* Note: sync mask_out code with ngram_bdb.cpp. */
bool Bigram::mask_out(phrase_token_t mask, phrase_token_t value){
    GArray * items = g_array_new(FALSE, FALSE, sizeof(phrase_token_t));

    if (!get_all_items(items)) {
        g_array_free(items, TRUE);
        return false;
    }

    for (size_t i = 0; i < items->len; ++i) {
        phrase_token_t index = g_array_index(items, phrase_token_t, i);

        if ((index & mask) == value) {
            check_result(remove(index));
            continue;
        }

        SingleGram * gram = NULL;
        check_result(load(index, gram));

        int num = gram->mask_out(mask, value);
        if (0 == num) {
            delete gram;
            continue;
        }

        if (0 == gram->get_length()) {
            check_result(remove(index));
        } else {
            check_result(store(index, gram));
        }

        delete gram;
    }

    g_array_free(items, TRUE);
    return true;
}
