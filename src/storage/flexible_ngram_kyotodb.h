/* 
 *  libpinyin
 *  Library to deal with pinyin.
 *  
 *  Copyright (C) 2015 Peng Wu <alexepico@gmail.com>
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

#ifndef FLEXIBLE_NGRAM_KYOTODB_H
#define FLEXIBLE_NGRAM_KYOTODB_H

#ifdef HAVE_KYOTO_CABINET
#include <kcdb.h>
#endif

#include "memory_chunk.h"

namespace pinyin{

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
    using kyotocabinet::BasicDB;

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

        m_chunk.set_size(vsiz);
        char * vbuf = (char *) m_chunk.begin();
        assert (vsiz == m_db->get(kbuf, sizeof(phrase_token_t),
                                  vbuf, vsiz));

        if ( memcmp(vbuf, m_magic_number,
                    sizeof(m_magic_number)) == 0 )
            return true;
        return false;
    }
};

};

#endif
