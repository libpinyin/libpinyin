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

#ifndef FLEXIBLE_NGRAM_TKRZWDB_H
#define FLEXIBLE_NGRAM_TKRZWDB_H

#include "config.h"
#ifdef HAVE_TKRZW
#include <tkrzw_dbm.h>
#include <tkrzw_dbm_tree.h>
#include <tkrzw_file_util.h>
#endif

#include "memory_chunk.h"

namespace pinyin{

using tkrzw::DBM;
using tkrzw::HashDBM;
using tkrzw::Status;

class FlexibleKeyCollectProcessor final : public DBM::RecordProcessor {
private:
    GArray * m_items;
public:
    FlexibleKeyCollectProcessor(GArray * items) {
        m_items = items;
    }

    std::string_view ProcessFull(std::string_view key, std::string_view value) override {
        /* skip magic header. */
        if (key.size() != sizeof(phrase_token_t))
            return NOOP;

        const phrase_token_t * token = (const phrase_token_t *) key.data();
        g_array_append_val(m_items, *token);
        return NOOP;
    }

    std::string_view ProcessEmpty(std::string_view key) override {
        return NOOP;
    }
};

/**
 * FlexibleBigram:
 * @MagicHeader: the struct type of the magic header.
 * @ArrayHeader: the struct type of the array header.
 * @ArrayItem: the struct type of the array item.
 *
 * The flexible bi-gram is mainly used for training purpose.
 *
 */
template<typename MagicHeader, typename ArrayHeader,
         typename ArrayItem>
class FlexibleBigram{
    /* Note: some flexible bi-gram file format check should be here. */
private:
    DBM * m_db;

    MemoryChunk m_chunk;

    phrase_token_t m_magic_header_index[2];

    char m_magic_number[4];

    void reset(){
        if ( m_db ){
            m_db->Synchronize(false);
            m_db->Close();
            delete m_db;
            m_db = nullptr;
        }
    }

public:
    /**
     * FlexibleBigram::FlexibleBigram:
     * @magic_number: the 4 bytes magic number of the flexible bi-gram.
     *
     * The constructor of the FlexibleBigram.
     *
     */
    FlexibleBigram(const char * magic_number){
        m_db = nullptr;
        m_magic_header_index[0] = null_token;
        m_magic_header_index[1] = null_token;

        memcpy(m_magic_number, magic_number, sizeof(m_magic_number));
    }

    /**
     * FlexibleBigram::~FlexibleBigram:
     *
     * The destructor of the FlexibleBigram.
     *
     */
    ~FlexibleBigram(){
        reset();
    }

    /**
     * FlexibleBigram::attach:
     * @dbfile: the path name of the flexible bi-gram.
     * @flags: the attach flags for the Tkrzw DB.
     * @returns: whether the attach operation is successful.
     *
     * Attach Tkrzw DB on filesystem for training purpose.
     *
     */
    bool attach(const char * dbfile, guint32 flags){
        reset();

        bool writable = false;
        int32_t options = 0;

        if (flags & ATTACH_READONLY)
            writable = false;
        if (flags & ATTACH_READWRITE) {
            assert( !( flags & ATTACH_READONLY ) );
            writable = true;
            options = tkrzw::File::OPEN_DEFAULT;
        }

        if (!dbfile)
            return false;

        m_db = new HashDBM;

        if (!m_db->Open(dbfile, writable, options | tkrzw::File::OPEN_NO_CREATE).IsOK()) {
            if (!(flags & ATTACH_CREATE)) {
                delete m_db;
                m_db = nullptr;
                return false;
            }

            /* Create database file here, and write the signature. */
            if (!m_db->Open(dbfile, writable, options).IsOK()) {
                delete m_db;
                m_db = nullptr;
                return false;
            }

            const char * kbuf = (char *) m_magic_header_index;
            const size_t ksiz = sizeof(m_magic_header_index);
            const char * vbuf = (char *) m_magic_number;
            const size_t vsiz = sizeof(m_magic_number);

            m_db->Set(std::string_view(kbuf, ksiz), std::string_view(vbuf, vsiz));
            return true;
        }

        /* check the signature. */
        const char * kbuf = (char *) m_magic_header_index;
        const size_t ksiz = sizeof(m_magic_header_index);

        std::string value_str;
        Status status = m_db->Get(std::string_view(kbuf, ksiz), &value_str);
        if (!status.IsOK())
            return false;

        size_t vsiz = value_str.size();
        m_chunk.set_size(vsiz);
        char * vbuf = (char *) m_chunk.begin();
        memcpy(vbuf, value_str.data(), vsiz);

        if ( memcmp(vbuf, m_magic_number,
                    sizeof(m_magic_number)) == 0 )
            return true;
        return false;
    }

    /**
     * FlexibleBigram::load:
     * @index: the previous token in the flexible bi-gram.
     * @single_gram: the single gram of the previous token.
     * @copy: whether copy content to the single gram.
     * @returns: whether the load operation is successful.
     *
     * Load the single gram of the previous token.
     *
     */
    bool load(phrase_token_t index,
              FlexibleSingleGram<ArrayHeader, ArrayItem> * & single_gram,
              bool copy=false){
        single_gram = nullptr;
        if ( !m_db )
            return false;

        /* Use DB interface to get value into the chunk. */
        const char * kbuf = (char *) &index;
        const size_t ksiz = sizeof(phrase_token_t);

        std::string value_str;
        Status status = m_db->Get(std::string_view(kbuf, ksiz), &value_str);
        if (!status.IsOK())
            return false;

        size_t vsiz = value_str.size();
        m_chunk.set_size(vsiz);
        char * vbuf = (char *) m_chunk.begin();
        memcpy(vbuf, value_str.data(), vsiz);

        single_gram = new FlexibleSingleGram<ArrayHeader, ArrayItem>
            (m_chunk.begin(), vsiz, copy);

        return true;
    }

    /**
     * FlexibleBigram::store:
     * @index: the previous token in the flexible bi-gram.
     * @single_gram: the single gram of the previous token.
     * @returns: whether the store operation is successful.
     *
     * Store the single gram of the previous token.
     *
     */
    bool store(phrase_token_t index,
               FlexibleSingleGram<ArrayHeader, ArrayItem> * single_gram){
        if ( !m_db )
            return false;

        const char * kbuf = (char *) &index;
        char * vbuf = (char *) single_gram->m_chunk.begin();
        size_t vsiz = single_gram->m_chunk.size();

        return m_db->Set(std::string_view(kbuf, sizeof(phrase_token_t)),
                         std::string_view(vbuf, vsiz)).IsOK();
    };

    /**
     * FlexibleBigram::remove:
     * @index: the previous token in the flexible bi-gram.
     * @returns: whether the remove operation is successful.
     *
     * Remove the single gram of the previous token.
     *
     */
    bool remove(phrase_token_t index){
        if ( !m_db )
            return false;

        const char * kbuf = (char *) &index;

        return m_db->Remove(std::string_view(kbuf, sizeof(phrase_token_t))).IsOK();
    }

    /**
     * FlexibleBigram::get_all_items:
     * @items: the GArray to store all previous tokens.
     * @returns: whether the get operation is successful.
     *
     * Get the array of all previous tokens for parameter estimation.
     *
     */
    bool get_all_items(GArray * items){
        g_array_set_size(items, 0);

        if ( !m_db )
            return false;

        FlexibleKeyCollectProcessor processor(items);
        return m_db->ProcessEach(&processor, false).IsOK();
    }

    /**
     * FlexibleBigram::get_magic_header:
     * @header: the magic header.
     * @returns: whether the get operation is successful.
     *
     * Get the magic header of the flexible bi-gram.
     *
     */
    bool get_magic_header(MagicHeader & header){
        /* clear retval */
        memset(&header, 0, sizeof(MagicHeader));

        if ( !m_db )
            return false;

        const char * kbuf = (char *) m_magic_header_index;
        const size_t ksiz = sizeof(m_magic_header_index);
        const std::string_view key_view(kbuf, ksiz);

        const size_t expected_vsiz = sizeof(m_magic_number) + sizeof(MagicHeader);

        std::string value_str;
        Status status = m_db->Get(key_view, &value_str);
        const int32_t retsize = value_str.size();

        /* an empty file without magic header here. */
        if (!status.IsOK() || retsize != expected_vsiz) {
            if (status.IsOK())
                assert(retsize == sizeof(m_magic_number));
            return false;
        }

        const char* vbuf = value_str.data();
        /* double check the magic number. */
        assert(0 == memcmp(m_magic_number, vbuf, sizeof(m_magic_number)));

        /* copy the result. */
        memcpy(&header, vbuf + sizeof(m_magic_number), sizeof(MagicHeader));
        return true;
    }

    /**
     * FlexibleBigram::set_magic_header:
     * @header: the magic header.
     * @returns: whether the set operation is successful.
     *
     * Set the magic header of the flexible bi-gram.
     *
     */
    bool set_magic_header(const MagicHeader & header){
        if ( !m_db )
            return false;

        /* As when create file, we will store the signature;
           when open file, we will check the signature;
           skip the signature check here, store both
           signature and header here. */

        /* reserve memory chunk for magic header. */
        const char * kbuf = (char *) m_magic_header_index;
        const size_t ksiz = sizeof(m_magic_header_index);

        /* copy to the memory chunk. */
        m_chunk.set_content(0, m_magic_number, sizeof(m_magic_number));
        m_chunk.set_content
            (sizeof(m_magic_number), &header, sizeof(MagicHeader));

        const size_t vsiz = sizeof(m_magic_number) + sizeof(MagicHeader);
        m_chunk.set_size(vsiz);
        char * vbuf = (char *)m_chunk.begin();

        return m_db->Set(std::string_view(kbuf, ksiz),
                         std::string_view(vbuf, vsiz)).IsOK();
    }

    /**
     * FlexibleBigram::get_array_header:
     * @index: the previous token in the flexible bi-gram.
     * @header: the array header in the single gram of the previous token.
     * @returns: whether the get operation is successful.
     *
     * Get the array header in the single gram of the previous token.
     *
     */
    bool get_array_header(phrase_token_t index, ArrayHeader & header){
        /* clear retval */
        memset(&header, 0, sizeof(ArrayHeader));

        if ( !m_db )
            return false;

        const char * kbuf = (char *) &index;
        const size_t ksiz = sizeof(phrase_token_t);
        const size_t header_vsiz = sizeof(ArrayHeader);

        std::string value_str;
        Status status = m_db->Get(std::string_view(kbuf, ksiz), &value_str);

        if (!status.IsOK())
            return false;

        const int32_t retsize = value_str.size();
        const char* vbuf = value_str.data();

        assert(retsize >= (int32_t) header_vsiz);
        memcpy(&header, vbuf, sizeof(ArrayHeader));
        return true;
    }

    /**
     * FlexibleBigram::set_array_header:
     * @index: the previous token of the flexible bi-gram.
     * @header: the array header in the single gram of the previous token.
     * @returns: whether the set operation is successful.
     *
     * Set the array header in the single gram of the previous token.
     *
     */
    bool set_array_header(phrase_token_t index, const ArrayHeader & header){
        if ( !m_db )
            return false;

        /* As tkrzw doesn't support partial load/store operation,
           load the entire item, then store it.*/
        const char * kbuf = (char *) &index;
        const size_t ksiz = sizeof(phrase_token_t);

        std::string value_str;
        Status status = m_db->Get(std::string_view(kbuf, ksiz), &value_str);
        if (!status.IsOK())
            return false;

        size_t vsiz = value_str.size();
        m_chunk.set_size(vsiz);
        char * vbuf = (char *) m_chunk.begin();
        memcpy(vbuf, value_str.data(), vsiz);

        m_chunk.set_content(0, &header, sizeof(ArrayHeader));

        /* the memory chunk address may change when re-allocated. */
        vbuf = (char *) m_chunk.begin();

        return m_db->Set(std::string_view(kbuf, ksiz),
                         std::string_view(vbuf, vsiz)).IsOK();
    }
};

};

#endif
