/* 
 *  libpinyin
 *  Library to deal with pinyin.
 *  
 *  Copyright (C) 2011 Peng Wu <alexepico@gmail.com>
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
    LOG_INVALID_RECORD = 0,
    LOG_ADD_RECORD = 1,
    LOG_REMOVE_RECORD,
    LOG_MODIFY_RECORD,
    LOG_MODIFY_HEADER
};


/**
 * PhraseIndexLogger:
 *
 * The logger of phrase index changes.
 *
 */
class PhraseIndexLogger{
protected:
    MemoryChunk * m_chunk;
    size_t m_offset;
    bool m_error;

    void reset(){
        if ( m_chunk ){
            delete m_chunk;
            m_chunk = NULL;
        }
        m_offset = 0;
        m_error = false;
    }
public:
    /**
     * PhraseIndexLogger::PhraseIndexLogger:
     *
     * The constructor of the PhraseIndexLogger.
     *
     */
    PhraseIndexLogger():m_offset(0), m_error(false){
        m_chunk = new MemoryChunk;
    }

    /**
     * PhraseIndexLogger::~PhraseIndexLogger:
     *
     * The destructor of the PhraseIndexLogger.
     *
     */
    ~PhraseIndexLogger(){
        reset();
    }

    /**
     * PhraseIndexLogger::load:
     * @chunk: the memory chunk of the logs.
     * @returns: whether the load operation is successful.
     *
     * Load the logs from the memory chunk.
     *
     */
    bool load(MemoryChunk * chunk) {
        reset();
        m_chunk = chunk;
        return true;
    }

    /**
     * PhraseIndexLogger::store:
     * @new_chunk: the new memory chunk to store the logs.
     * @returns: whether the store operation is successful.
     *
     * Store the logs to the new memory chunk.
     *
     */
    bool store(MemoryChunk * new_chunk){
        new_chunk->set_content(0, m_chunk->begin(), m_chunk->size());
        return true;
    }

    /**
     * PhraseIndexLogger::has_next_record:
     * @returns: whether this logger has next record.
     *
     * Whether this logger has next record.
     *
     */
    bool has_next_record(){
        if (m_error)
            return false;

        return m_offset < m_chunk->size();
    }

    /**
     * PhraseIndexLogger::rewind:
     * @returns: whether the rewind operation is successful.
     *
     * Rewind this logger to the begin of logs.
     *
     */
    bool rewind(){
        m_offset = 0;
        return true;
    }

    /**
     * PhraseIndexLogger::next_record:
     * @log_type: the type of this log record.
     * @token: the token of this log record.
     * @oldone: the original content of the phrase item.
     * @newone: the new content of the phrase item.
     *
     * Read the next log record.
     *
     * Prolog: has_next_record() returned true.
     *
     */
    bool next_record(LOG_TYPE & log_type, phrase_token_t & token,
                     MemoryChunk * oldone, MemoryChunk * newone){
        log_type = LOG_INVALID_RECORD;
        token = null_token;

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
            m_error = true;
            return false;
        }

        m_offset = offset;
        return true;
    }

    /**
     * PhraseIndexLogger::append_record:
     * @log_type: the type of this log record.
     * @token: the token of this log record.
     * @oldone: the original content of the phrase item.
     * @newone: the new content of the phrase item.
     *
     * Append one log record to the logger.
     *
     */
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
