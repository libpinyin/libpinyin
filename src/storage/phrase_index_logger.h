/* 
 *  libpinyin
 *  Library to deal with pinyin.
 *  
 *  Copyright (C) 2011 Peng Wu <alexepico@gmail.com>
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


#ifndef PHRASE_LOGGER_H
#define PHRASE_LOGGER_H

#include <assert.h>
#include "novel_types.h"
#include "memory_chunk.h"

/**
 *  File Format
 *  Logger Record type: add/remove/modify
 *
 *  Modify Header: header/null token/len/old data chunk/new data chunk
 *
 *  Add Record:    add/token/len/data chunk
 *  Remove Record: remove/token/len/data chunk
 *  Modify Record: modify/token/old len/new len/old data chunk/new data chunk
 *
 */

namespace pinyin{

enum LOG_TYPE{
    LOG_ADD_RECORD = 1,
    LOG_REMOVE_RECORD,
    LOG_MODIFY_RECORD,
    LOG_MODIFY_HEADER
};

class PhraseIndexLogger{
protected:
    MemoryChunk * m_chunk;
    size_t m_offset;

    void reset(){
        if ( m_chunk ){
            delete m_chunk;
            m_chunk = NULL;
        }
        m_offset = 0;
    }
public:
    PhraseIndexLogger():m_offset(0){
        m_chunk = new MemoryChunk;
    }

    ~PhraseIndexLogger(){
        reset();
    }

    bool load(MemoryChunk * chunk) {
        reset();
        m_chunk = chunk;
        return true;
    }

    bool store(MemoryChunk * new_chunk){
        new_chunk->set_content(0, m_chunk->begin(), m_chunk->size());
        return true;
    }

    bool has_next_record(){
        return m_offset < m_chunk->size();
    }

    bool rewind(){
        m_offset = 0;
        return true;
    }

    /* prolog: has_next_record() returned true. */
    bool next_record(LOG_TYPE & log_type, phrase_token_t & token,
                     MemoryChunk * oldone, MemoryChunk * newone){
        size_t offset = m_offset;
        m_chunk->get_content(offset, &log_type, sizeof(LOG_TYPE));
        offset += sizeof(LOG_TYPE);
        m_chunk->get_content(offset, &token, sizeof(phrase_token_t));
        offset += sizeof(phrase_token_t);

        oldone->set_size(0); newone->set_size(0);

        switch(log_type){
        case LOG_ADD_RECORD:{
            guint16 len = 0;
            m_chunk->get_content(offset, &len, sizeof(guint16));
            offset += sizeof(guint16);
            newone->set_content(0, ((char *)m_chunk->begin()) + offset, len);
            offset += len;
            break;
        }
        case LOG_REMOVE_RECORD:{
            guint16 len = 0;
            m_chunk->get_content(offset, &len, sizeof(guint16));
            offset += sizeof(guint16);
            oldone->set_content(0, ((char *)m_chunk->begin()) + offset, len);
            offset += len;
            break;
        }
        case LOG_MODIFY_RECORD:{
            guint16 oldlen = 0, newlen = 0;
            m_chunk->get_content(offset, &oldlen, sizeof(guint16));
            offset += sizeof(guint16);
            m_chunk->get_content(offset, &newlen, sizeof(guint16));
            offset += sizeof(guint16);
            oldone->set_content(0, ((char *)m_chunk->begin()) + offset,
                                oldlen);
            offset += oldlen;
            newone->set_content(0, ((char *)m_chunk->begin()) + offset, newlen);
            offset += newlen;
            break;
        }
        case LOG_MODIFY_HEADER:{
            assert(token == null_token);
            guint16 len = 0;
            m_chunk->get_content(offset, &len, sizeof(guint16));
            offset += sizeof(guint16);
            oldone->set_content(0, ((char *)m_chunk->begin()) + offset,
                                len);
            offset += len;
            newone->set_content(0, ((char *)m_chunk->begin()) + offset,
                                len);
            offset += len;
            break;
        }
        default:
            assert(false);
        }

        m_offset = offset;
        return true;
    }

    bool append_record(LOG_TYPE log_type, phrase_token_t token,
                       MemoryChunk * oldone, MemoryChunk * newone){

        MemoryChunk chunk;
        size_t offset = 0;
        chunk.set_content(offset, &log_type, sizeof(LOG_TYPE));
        offset += sizeof(LOG_TYPE);
        chunk.set_content(offset, &token, sizeof(phrase_token_t));
        offset += sizeof(phrase_token_t);

        switch(log_type){
        case LOG_ADD_RECORD:{
            assert( NULL == oldone );
            assert( NULL != newone );
            /* use newone chunk */
            guint16 len = newone->size();
            chunk.set_content(offset, &len, sizeof(guint16));
            offset += sizeof(guint16);
            chunk.set_content(offset, newone->begin(), newone->size());
            offset += newone->size();
            break;
        }
        case LOG_REMOVE_RECORD:{
            assert(NULL != oldone);
            assert(NULL == newone);
            /* use oldone chunk */
            guint16 len = oldone->size();
            chunk.set_content(offset, &len, sizeof(guint16));
            offset += sizeof(guint16);
            chunk.set_content(offset, oldone->begin(), oldone->size());
            offset += oldone->size();
            break;
        }
        case LOG_MODIFY_RECORD:{
            assert(NULL != oldone);
            assert(NULL != newone);
            guint16 oldlen = oldone->size();
            guint16 newlen = newone->size();
            chunk.set_content(offset, &oldlen, sizeof(guint16));
            offset += sizeof(guint16);
            chunk.set_content(offset, &newlen, sizeof(guint16));
            offset += sizeof(guint16);
            chunk.set_content(offset, oldone->begin(), oldone->size());
            offset += oldlen;
            chunk.set_content(offset, newone->begin(), newone->size());
            offset += newlen;
            break;
        }
        case LOG_MODIFY_HEADER:{
            assert(NULL != oldone);
            assert(NULL != newone);
            assert(null_token == token);
            guint16 oldlen = oldone->size();
            guint16 newlen = newone->size();
            assert(oldlen == newlen);
            chunk.set_content(offset, &oldlen, sizeof(guint16));
            offset += sizeof(guint16);
            chunk.set_content(offset, oldone->begin(), oldone->size());
            offset += oldlen;
            chunk.set_content(offset, newone->begin(), newone->size());
            offset += newlen;
            break;
        }
        default:
            assert(false);
        }

        /* store log record. */
        m_chunk->set_content(m_chunk->size(), chunk.begin(), chunk.size());
        return true;
    }
};

};

#endif
