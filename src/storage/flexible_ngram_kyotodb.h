/* 
 *  libpinyin
 *  Library to deal with pinyin.
 *  
 *  Copyright (C) 2015 Peng Wu <alexepico@gmail.com>
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

#ifndef FLEXIBLE_NGRAM_KYOTODB_H
#define FLEXIBLE_NGRAM_KYOTODB_H

#include "config.h"
#ifdef HAVE_KYOTO_CABINET
#include <kcdb.h>
#include <kchashdb.h>
#endif

#include "memory_chunk.h"

namespace pinyin{

using kyotocabinet::DB;
using kyotocabinet::BasicDB;
using kyotocabinet::HashDB;

class FlexibleKeyCollectVisitor : public DB::Visitor {
private:
    GArray * m_items;
public:
    FlexibleKeyCollectVisitor(GArray * items) {
        m_items = items;
    }

    virtual const char* visit_full(const char* kbuf, size_t ksiz,
                                   const char* vbuf, size_t vsiz, size_t* sp) {
        /* skip magic header. */
        if (ksiz != sizeof(phrase_token_t))
            return NOP;

        const phrase_token_t * token = (phrase_token_t *) kbuf;
        g_array_append_val(m_items, *token);
        return NOP;
    }

    virtual const char* visit_empty(const char* kbuf, size_t ksiz, size_t* sp) {
        /* assume no empty record. */
        assert (FALSE);
        return NOP;
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
    BasicDB * m_db;

    MemoryChunk m_chunk;

    phrase_token_t m_magic_header_index[2];

    char m_magic_number[4];

    void reset(){
        if ( m_db ){
            m_db->synchronize();
            m_db->close();
            m_db = NULL;
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
        m_db = NULL;
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
     * @flags: the attach flags for the Berkeley DB.
     * @returns: whether the attach operation is successful.
     *
     * Attach Berkeley DB on filesystem for training purpose.
     *
     */
    bool attach(const char * dbfile, guint32 flags){
        reset();
        uint32_t mode = 0;

        if (flags & ATTACH_READONLY)
            mode |= BasicDB::OREADER;
        if (flags & ATTACH_READWRITE) {
            assert( !( flags & ATTACH_READONLY ) );
            mode |= BasicDB::OREADER | BasicDB::OWRITER;
        }

        if (!dbfile)
            return false;

        m_db = new HashDB;

        if (!m_db->open(dbfile, mode)) {
            if (!(flags & ATTACH_CREATE)) {
                delete m_db;
                m_db = NULL;
                return false;
            }

            mode |= BasicDB::OCREATE;
            /* Create database file here, and write the signature. */
            if (!m_db->open(dbfile, mode))
                return false;

            const char * kbuf = (char *) m_magic_header_index;
            const size_t ksiz = sizeof(m_magic_header_index);
            const char * vbuf = (char *) m_magic_number;
            const size_t vsiz = sizeof(m_magic_number);
            m_db->set(kbuf, ksiz, vbuf, vsiz);
            return true;
        }

        /* check the signature. */
        const char * kbuf = (char *) m_magic_header_index;
        const size_t ksiz = sizeof(m_magic_header_index);
        const int32_t vsiz = m_db->check(kbuf, ksiz);
        if (-1 == vsiz)
            return false;

        m_chunk.set_size(vsiz);
        char * vbuf = (char *) m_chunk.begin();
        assert (vsiz == m_db->get(kbuf, ksiz, vbuf, vsiz));

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
        single_gram = NULL;
        if ( !m_db )
            return false;

        /* Use DB interface, first check, second reserve the memory chunk,
           third get value into the chunk. */
        const char * kbuf = (char *) &index;
        const int32_t vsiz = m_db->check(kbuf, sizeof(phrase_token_t));
        /* -1 on failure. */
        if (-1 == vsiz)
            return false;

        m_chunk.set_size(vsiz);
        char * vbuf = (char *) m_chunk.begin();
        assert (vsiz == m_db->get(kbuf, sizeof(phrase_token_t),
                                  vbuf, vsiz));

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
        return m_db->set(kbuf, sizeof(phrase_token_t), vbuf, vsiz);
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
        return m_db->remove(kbuf, sizeof(phrase_token_t));
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

        FlexibleKeyCollectVisitor visitor(items);
        m_db->iterate(&visitor, false);

        return true;
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

        /* reserve memory chunk for magic header. */
        const char * kbuf = (char *) m_magic_header_index;
        const size_t ksiz = sizeof(m_magic_header_index);
        const size_t vsiz = sizeof(m_magic_number) + sizeof(MagicHeader);
        m_chunk.set_size(vsiz);
        char * vbuf = (char *)m_chunk.begin();

        const int32_t retsize = m_db->get(kbuf, ksiz, vbuf, vsiz);
        /* an empty file without magic header here. */
        if (retsize != vsiz) {
            assert(retsize == sizeof(m_magic_number));
            return false;
        }

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

        return m_db->set(kbuf, ksiz, vbuf, vsiz);
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
        const size_t vsiz = sizeof(ArrayHeader);
        m_chunk.set_size(vsiz);
        char * vbuf = (char *) m_chunk.begin();

        int32_t retsize = m_db->get(kbuf, ksiz, vbuf, vsiz);
        if (-1 == retsize)
            return false;

        /* the single gram contains at least the array header. */
        assert(retsize >= (int32_t)vsiz);
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

        /* As kyoto cabinet doesn't support partial load/store operation,
           load the entire item, then store it.*/
        const char * kbuf = (char *) &index;
        const size_t ksiz = sizeof(phrase_token_t);

        int32_t vsiz = m_db->check(kbuf, ksiz);
        if (-1 == vsiz) { /* not found. */
            vsiz = sizeof(ArrayHeader);
        } else { /* found */
            m_chunk.set_size(vsiz);
            char * vbuf = (char *) m_chunk.begin();
            assert(vsiz == m_db->get(kbuf, ksiz, vbuf, vsiz));
        }

        m_chunk.set_content(0, &header, sizeof(ArrayHeader));

        /* the memory chunk address may change when re-allocated. */
        char * vbuf = (char *) m_chunk.begin();
        return m_db->set(kbuf, ksiz, vbuf, vsiz);
    }
};

};

#endif
