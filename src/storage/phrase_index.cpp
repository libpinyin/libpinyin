/* 
 *  libpinyin
 *  Library to deal with pinyin.
 *  
 *  Copyright (C) 2006-2007 Peng Wu
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

#include "phrase_index.h"
#include "pinyin_custom2.h"

namespace pinyin{

bool PhraseItem::set_n_pronunciation(guint8 n_prouns){
    m_chunk.set_content(sizeof(guint8), &n_prouns, sizeof(guint8));
    return true;
}

bool PhraseItem::get_nth_pronunciation(size_t index, ChewingKey * keys,
                                       guint32 & freq){
    guint8 phrase_length = get_phrase_length();
    table_offset_t offset = phrase_item_header + phrase_length * sizeof( ucs4_t) + index * ( phrase_length * sizeof (ChewingKey) + sizeof(guint32));

    bool retval = m_chunk.get_content
        (offset, keys, phrase_length * sizeof(ChewingKey));
    if ( !retval )
        return retval;
    return m_chunk.get_content
        (offset + phrase_length * sizeof(ChewingKey), &freq , sizeof(guint32));
}

#if 0
void PhraseItem::append_pronunciation(ChewingKey * keys, guint32 freq){
    guint8 phrase_length = get_phrase_length();
    set_n_pronunciation(get_n_pronunciation() + 1);
    m_chunk.set_content(m_chunk.size(), keys,
                        phrase_length * sizeof(ChewingKey));
    m_chunk.set_content(m_chunk.size(), &freq, sizeof(guint32));
}
#endif

bool PhraseItem::add_pronunciation(ChewingKey * keys, guint32 delta){
    guint8 phrase_length = get_phrase_length();
    guint8 npron = get_n_pronunciation();
    size_t offset = phrase_item_header + phrase_length * sizeof(ucs4_t);
    char * buf_begin = (char *) m_chunk.begin();
    guint32 total_freq = 0;

    for (int i = 0; i < npron; ++i) {
        char * chewing_begin = buf_begin + offset +
            i * (phrase_length * sizeof(ChewingKey) + sizeof(guint32));
        guint32 * freq = (guint32 *)(chewing_begin +
                                     phrase_length * sizeof(ChewingKey));

        total_freq += *freq;

        if (0 == pinyin_exact_compare2
            (keys, (ChewingKey *)chewing_begin, phrase_length)) {
            /* found the exact match pinyin keys. */

            /* protect against total_freq overflow. */
            if (delta > 0 && total_freq > total_freq + delta)
                return false;

            *freq += delta;
            total_freq += delta;
            return true;
        }
    }

    set_n_pronunciation(npron + 1);
    m_chunk.set_content(m_chunk.size(), keys,
                        phrase_length * sizeof(ChewingKey));
    m_chunk.set_content(m_chunk.size(), &delta, sizeof(guint32));
    return true;
}

void PhraseItem::remove_nth_pronunciation(size_t index){
    guint8 phrase_length = get_phrase_length();
    set_n_pronunciation(get_n_pronunciation() - 1);
    size_t offset = phrase_item_header + phrase_length * sizeof(ucs4_t) +
        index * (phrase_length * sizeof(ChewingKey) + sizeof(guint32));
    m_chunk.remove_content(offset, phrase_length * sizeof(ChewingKey) + sizeof(guint32));
}

bool PhraseItem::get_phrase_string(ucs4_t * phrase){
    guint8 phrase_length = get_phrase_length();
    return m_chunk.get_content(phrase_item_header, phrase, phrase_length * sizeof(ucs4_t));
}

bool PhraseItem::set_phrase_string(guint8 phrase_length, ucs4_t * phrase){
    m_chunk.set_content(0, &phrase_length, sizeof(guint8));
    m_chunk.set_content(phrase_item_header, phrase, phrase_length * sizeof(ucs4_t));
    return true;
}

void PhraseItem::increase_pronunciation_possibility(ChewingKey * keys,
                                                    gint32 delta){
    guint8 phrase_length = get_phrase_length();
    guint8 npron = get_n_pronunciation();
    size_t offset = phrase_item_header + phrase_length * sizeof(ucs4_t);
    char * buf_begin = (char *) m_chunk.begin();
    guint32 total_freq = 0;

    for (int i = 0; i < npron; ++i) {
        char * chewing_begin = buf_begin + offset +
            i * (phrase_length * sizeof(ChewingKey) + sizeof(guint32));
        guint32 * freq = (guint32 *)(chewing_begin +
                                     phrase_length * sizeof(ChewingKey));
        total_freq += *freq;

        if (0 == pinyin_compare_with_tones(keys, (ChewingKey *)chewing_begin,
                                           phrase_length)) {

            /* protect against total_freq overflow. */
            if (delta > 0 && total_freq > total_freq + delta)
                return;

            *freq += delta;
            total_freq += delta;
        }
    }
}


guint32 SubPhraseIndex::get_phrase_index_total_freq(){
    return m_total_freq;
}

int SubPhraseIndex::add_unigram_frequency(phrase_token_t token, guint32 delta){
    table_offset_t offset;
    guint32 freq;
    bool result = m_phrase_index.get_content
        ((token & PHRASE_MASK) 
         * sizeof(table_offset_t), &offset, sizeof(table_offset_t));

    if ( !result )
        return ERROR_OUT_OF_RANGE;

    if ( 0 == offset )
        return ERROR_NO_ITEM;

    result = m_phrase_content.get_content
        (offset + sizeof(guint8) + sizeof(guint8), &freq, sizeof(guint32));

    if ( !result )
        return ERROR_FILE_CORRUPTION;

    //protect total_freq overflow
    if ( delta > 0 && m_total_freq > m_total_freq + delta )
        return ERROR_INTEGER_OVERFLOW;

    freq += delta;
    m_total_freq += delta;
    m_phrase_content.set_content(offset + sizeof(guint8) + sizeof(guint8), &freq, sizeof(guint32));

    return ERROR_OK;
}

int SubPhraseIndex::get_phrase_item(phrase_token_t token, PhraseItem & item){
    table_offset_t offset;
    guint8 phrase_length;
    guint8 n_prons;
    
    bool result = m_phrase_index.get_content
        ((token & PHRASE_MASK) 
         * sizeof(table_offset_t), &offset, sizeof(table_offset_t));

    if ( !result )
        return ERROR_OUT_OF_RANGE;

    if ( 0 == offset )
        return ERROR_NO_ITEM;

    result = m_phrase_content.get_content(offset, &phrase_length, sizeof(guint8));
    if ( !result ) 
        return ERROR_FILE_CORRUPTION;
    
    result = m_phrase_content.get_content(offset+sizeof(guint8), &n_prons, sizeof(guint8));
    if ( !result ) 
        return ERROR_FILE_CORRUPTION;

    size_t length = phrase_item_header + phrase_length * sizeof ( ucs4_t ) + n_prons * ( phrase_length * sizeof (ChewingKey) + sizeof(guint32) );
    item.m_chunk.set_chunk((char *)m_phrase_content.begin() + offset, length, NULL);
    return ERROR_OK;
}

int SubPhraseIndex::add_phrase_item(phrase_token_t token, PhraseItem * item){
    table_offset_t offset = m_phrase_content.size();
    if ( 0 == offset )
        offset = 8;
    m_phrase_content.set_content(offset, item->m_chunk.begin(), item->m_chunk.size());
    m_phrase_index.set_content((token & PHRASE_MASK) 
                               * sizeof(table_offset_t), &offset, sizeof(table_offset_t));
    m_total_freq += item->get_unigram_frequency();
    return ERROR_OK;
}

int SubPhraseIndex::remove_phrase_item(phrase_token_t token, PhraseItem * & item){
    PhraseItem old_item;

    int result = get_phrase_item(token, old_item);
    if (result != ERROR_OK)
        return result;

    item = new PhraseItem;
    //implictly copy data from m_chunk_content.
    item->m_chunk.set_content(0, (char *) old_item.m_chunk.begin() , old_item.m_chunk.size());

    const table_offset_t zero_const = 0;
    m_phrase_index.set_content((token & PHRASE_MASK)
                               * sizeof(table_offset_t), &zero_const, sizeof(table_offset_t));
    m_total_freq -= item->get_unigram_frequency();
    return ERROR_OK;
}

bool FacadePhraseIndex::load(guint8 phrase_index, MemoryChunk * chunk){
    SubPhraseIndex * & sub_phrases = m_sub_phrase_indices[phrase_index];
    if ( !sub_phrases ){
        sub_phrases = new SubPhraseIndex;
    }

    m_total_freq -= sub_phrases->get_phrase_index_total_freq();
    bool retval = sub_phrases->load(chunk, 0, chunk->size());
    if ( !retval )
        return retval;
    m_total_freq += sub_phrases->get_phrase_index_total_freq();
    return retval;
}

bool FacadePhraseIndex::store(guint8 phrase_index, MemoryChunk * new_chunk){
    table_offset_t end;
    SubPhraseIndex * & sub_phrases = m_sub_phrase_indices[phrase_index];
    if ( !sub_phrases )
        return false;
    
    sub_phrases->store(new_chunk, 0, end);
    return true;
}

bool FacadePhraseIndex::unload(guint8 phrase_index){
    SubPhraseIndex * & sub_phrases = m_sub_phrase_indices[phrase_index];
    if ( !sub_phrases )
        return false;
    m_total_freq -= sub_phrases->get_phrase_index_total_freq();
    delete sub_phrases;
    sub_phrases = NULL;
    return true;
}

bool FacadePhraseIndex::diff(guint8 phrase_index, MemoryChunk * oldchunk,
                             MemoryChunk * newlog){
    SubPhraseIndex * & sub_phrases = m_sub_phrase_indices[phrase_index];
    if ( !sub_phrases )
        return false;

    SubPhraseIndex old_sub_phrases;
    old_sub_phrases.load(oldchunk, 0, oldchunk->size());
    PhraseIndexLogger logger;

    bool retval = sub_phrases->diff(&old_sub_phrases, &logger);
    logger.store(newlog);
    return retval;
}

bool FacadePhraseIndex::merge(guint8 phrase_index, MemoryChunk * log){
    SubPhraseIndex * & sub_phrases = m_sub_phrase_indices[phrase_index];
    if ( !sub_phrases )
        return false;

    m_total_freq -= sub_phrases->get_phrase_index_total_freq();
    PhraseIndexLogger logger;
    logger.load(log);

    bool retval = sub_phrases->merge(&logger);
    m_total_freq += sub_phrases->get_phrase_index_total_freq();

    return retval;
}

bool FacadePhraseIndex::merge_with_mask(guint8 phrase_index,
                                        MemoryChunk * log,
                                        phrase_token_t mask,
                                        phrase_token_t value){
    SubPhraseIndex * & sub_phrases = m_sub_phrase_indices[phrase_index];
    if ( !sub_phrases )
        return false;

    /* check mask and value. */
    phrase_token_t index_mask = PHRASE_INDEX_LIBRARY_INDEX(mask);
    phrase_token_t index_value = PHRASE_INDEX_LIBRARY_INDEX(value);
    if ((phrase_index & index_mask) != index_value)
        return false;

    /* unload old sub phrase index */
    m_total_freq -= sub_phrases->get_phrase_index_total_freq();

    /* calculate the sub phrase index mask and value. */
    mask &= PHRASE_MASK; value &= PHRASE_MASK;

    /* prepare the new logger. */
    PhraseIndexLogger oldlogger;
    oldlogger.load(log);
    PhraseIndexLogger * newlogger = mask_out_phrase_index_logger
        (&oldlogger, mask, value);

    bool retval = sub_phrases->merge(newlogger);
    m_total_freq += sub_phrases->get_phrase_index_total_freq();
    delete newlogger;

    return retval;
}


bool SubPhraseIndex::load(MemoryChunk * chunk, 
                          table_offset_t offset, table_offset_t end){
    //save the memory chunk
    if ( m_chunk ){
        delete m_chunk;
        m_chunk = NULL;
    }
    m_chunk = chunk;
    
    char * buf_begin = (char *)chunk->begin();
    chunk->get_content(offset, &m_total_freq, sizeof(guint32));
    offset += sizeof(guint32);
    table_offset_t index_one = 0, index_two = 0, index_three = 0;
    chunk->get_content(offset, &index_one, sizeof(table_offset_t));
    offset += sizeof(table_offset_t);
    chunk->get_content(offset, &index_two, sizeof(table_offset_t));
    offset += sizeof(table_offset_t);
    chunk->get_content(offset, &index_three, sizeof(table_offset_t));
    offset += sizeof(table_offset_t);
    g_return_val_if_fail(*(buf_begin + offset) == c_separate, FALSE);
    g_return_val_if_fail(*(buf_begin + index_two - 1) == c_separate, FALSE);
    g_return_val_if_fail(*(buf_begin + index_three - 1) == c_separate, FALSE);
    m_phrase_index.set_chunk(buf_begin + index_one, 
                             index_two - 1 - index_one, NULL);
    m_phrase_content.set_chunk(buf_begin + index_two, 
                               index_three - 1 - index_two, NULL);
    g_return_val_if_fail( index_three <= end, FALSE);
    return true;
}

bool SubPhraseIndex::store(MemoryChunk * new_chunk, 
                           table_offset_t offset, table_offset_t& end){
    new_chunk->set_content(offset, &m_total_freq, sizeof(guint32));
    table_offset_t index = offset + sizeof(guint32);
        
    offset = index + sizeof(table_offset_t) * 3 ;
    new_chunk->set_content(offset, &c_separate, sizeof(char));
    offset += sizeof(char);
    
    new_chunk->set_content(index, &offset, sizeof(table_offset_t));
    index += sizeof(table_offset_t);
    new_chunk->set_content(offset, m_phrase_index.begin(), m_phrase_index.size());
    offset += m_phrase_index.size();
    new_chunk->set_content(offset, &c_separate, sizeof(char));
    offset += sizeof(char);

    new_chunk->set_content(index, &offset, sizeof(table_offset_t));
    index += sizeof(table_offset_t);
    
    new_chunk->set_content(offset, m_phrase_content.begin(), m_phrase_content.size());
    offset += m_phrase_content.size();
    new_chunk->set_content(offset, &c_separate, sizeof(char));
    offset += sizeof(char);
    new_chunk->set_content(index, &offset, sizeof(table_offset_t));
    return true;
}

bool SubPhraseIndex::diff(SubPhraseIndex * oldone, PhraseIndexLogger * logger){
    /* diff the header */
    MemoryChunk oldheader, newheader;
    guint32 total_freq = oldone->get_phrase_index_total_freq();
    oldheader.set_content(0, &total_freq, sizeof(guint32));
    total_freq = get_phrase_index_total_freq();
    newheader.set_content(0, &total_freq, sizeof(guint32));
    logger->append_record(LOG_MODIFY_HEADER, null_token,
                          &oldheader, &newheader);

    /* diff phrase items */
    PhraseIndexRange oldrange, currange, range;
    oldone->get_range(oldrange); get_range(currange);
    range.m_range_begin = std_lite::min(oldrange.m_range_begin,
                                        currange.m_range_begin);
    range.m_range_end = std_lite::max(oldrange.m_range_end,
                                      currange.m_range_end);
    PhraseItem olditem, newitem;

    for (phrase_token_t token = range.m_range_begin;
         token < range.m_range_end; ++token ){
        bool oldretval = ERROR_OK == oldone->get_phrase_item(token, olditem);
        bool newretval = ERROR_OK == get_phrase_item(token, newitem);

        if ( oldretval ){
            if ( newretval ) { /* compare phrase item. */
                if ( olditem == newitem )
                    continue;
                logger->append_record(LOG_MODIFY_RECORD, token,
                                      &(olditem.m_chunk), &(newitem.m_chunk));
            } else { /* remove phrase item. */
                logger->append_record(LOG_REMOVE_RECORD, token,
                                      &(olditem.m_chunk), NULL);
            }
        } else {
            if ( newretval ){ /* add phrase item. */
                logger->append_record(LOG_ADD_RECORD, token,
                                      NULL, &(newitem.m_chunk));
            } else { /* both empty. */
                /* do nothing. */
            }
        }
    }

    return true;
}

bool SubPhraseIndex::merge(PhraseIndexLogger * logger){
    LOG_TYPE log_type = LOG_INVALID_RECORD;
    phrase_token_t token = null_token;
    MemoryChunk oldchunk, newchunk;
    PhraseItem olditem, newitem, item, * tmpitem;

    while(logger->has_next_record()){
        bool retval = logger->next_record
            (log_type, token, &oldchunk, &newchunk);

        if (!retval)
            break;

        switch(log_type){
        case LOG_ADD_RECORD:{
            assert( 0 == oldchunk.size() );
            newitem.m_chunk.set_chunk(newchunk.begin(), newchunk.size(),
                                      NULL);
            add_phrase_item(token, &newitem);
            break;
        }
        case LOG_REMOVE_RECORD:{
            assert( 0 == newchunk.size() );
            tmpitem = NULL;
            remove_phrase_item(token, tmpitem);

            olditem.m_chunk.set_chunk(oldchunk.begin(), oldchunk.size(),
                                      NULL);

            if (olditem != *tmpitem) {
                delete tmpitem;
                return false;
            }

            delete tmpitem;

            break;
        }
        case LOG_MODIFY_RECORD:{
            get_phrase_item(token, item);
            olditem.m_chunk.set_chunk(oldchunk.begin(), oldchunk.size(),
                                      NULL);
            newitem.m_chunk.set_chunk(newchunk.begin(), newchunk.size(),
                                      NULL);
            if (item != olditem)
                return false;

            if (newchunk.size() > item.m_chunk.size() ){ /* increase size. */
                tmpitem = NULL;
                remove_phrase_item(token, tmpitem);
                assert(olditem == *tmpitem);
                add_phrase_item(token, &newitem);
                delete tmpitem;
            } else { /* in place editing. */
                /* newchunk.size() <= item.m_chunk.size() */
                /* Hack here: we assume the behaviour of get_phrase_item
                 * point to the actual data positon, so changes to item
                 * will be saved in SubPhraseIndex immediately.
                 */
                memmove(item.m_chunk.begin(), newchunk.begin(),
                        newchunk.size());
            }
            break;
        }
        case LOG_MODIFY_HEADER:{
            guint32 total_freq = get_phrase_index_total_freq();
            guint32 tmp_freq = 0;
            assert(null_token == token);
            assert(oldchunk.size() == newchunk.size());
            oldchunk.get_content(0, &tmp_freq, sizeof(guint32));
            if (total_freq != tmp_freq)
                return false;
            newchunk.get_content(0, &tmp_freq, sizeof(guint32));
            m_total_freq = tmp_freq;
            break;
        }
        default:
            assert(false);
        }
    }
    return true;
}

bool FacadePhraseIndex::load_text(guint8 phrase_index, FILE * infile,
                                  TABLE_PHONETIC_TYPE type){
    SubPhraseIndex * & sub_phrases = m_sub_phrase_indices[phrase_index];
    if ( !sub_phrases ){
        sub_phrases = new SubPhraseIndex;
    }

    char pinyin[256];
    char phrase[256];
    phrase_token_t token;
    size_t freq;

    PhraseItem * item_ptr = new PhraseItem;
    phrase_token_t cur_token = 0;

    while (!feof(infile)){
#ifdef __APPLE__
        int num = fscanf(infile, "%255s %255[^ \t] %u %ld",
                         pinyin, phrase, &token, &freq);
#else
        int num = fscanf(infile, "%255s %255s %u %ld",
                         pinyin, phrase, &token, &freq);
#endif

        if (4 != num)
            continue;

        if (feof(infile))
            break;

        assert(PHRASE_INDEX_LIBRARY_INDEX(token) == phrase_index );

        glong written;
        ucs4_t * phrase_ucs4 = g_utf8_to_ucs4(phrase, -1, NULL, 
                                              &written, NULL);
	
        if ( 0 == cur_token ){
            cur_token = token;
            item_ptr->set_phrase_string(written, phrase_ucs4);
        }

        if ( cur_token != token ){
            add_phrase_item( cur_token, item_ptr);
            delete item_ptr;
            item_ptr = new PhraseItem;
            cur_token = token;
            item_ptr->set_phrase_string(written, phrase_ucs4);
        }

        ChewingKeyVector keys = g_array_new(FALSE, FALSE, sizeof(ChewingKey));
        ChewingKeyRestVector key_rests =
            g_array_new(FALSE, FALSE, sizeof(ChewingKeyRest));

        switch (type) {
        case PINYIN_TABLE: {
            PinyinDirectParser2 parser;
            pinyin_option_t options = USE_TONE;
            parser.parse(options, keys, key_rests, pinyin, strlen(pinyin));
            break;
        }

        case ZHUYIN_TABLE: {
            ZhuyinDirectParser2 parser;
            pinyin_option_t options = USE_TONE | FORCE_TONE;
            parser.parse(options, keys, key_rests, pinyin, strlen(pinyin));
            break;
        }
        };

        if (item_ptr->get_phrase_length() == keys->len) {
            item_ptr->add_pronunciation((ChewingKey *)keys->data, freq);
        } else {
            fprintf(stderr, "FacadePhraseIndex::load_text:%s\t%s\n",
                    pinyin, phrase);
        }

        g_array_free(keys, TRUE);
        g_array_free(key_rests, TRUE);
        g_free(phrase_ucs4);
    }

    add_phrase_item( cur_token, item_ptr);
    delete item_ptr;
#if 0
    m_total_freq += m_sub_phrase_indices[phrase_index]->get_phrase_index_total_freq();
#endif
    return true;
}

int FacadePhraseIndex::get_sub_phrase_range(guint8 & min_index,
                                            guint8 & max_index){
    min_index = PHRASE_INDEX_LIBRARY_COUNT; max_index = 0;
    for ( guint8 i = 0; i < PHRASE_INDEX_LIBRARY_COUNT; ++i ){
        if ( m_sub_phrase_indices[i] ) {
            min_index = std_lite::min(min_index, i);
            max_index = std_lite::max(max_index, i);
        }
    }
    return ERROR_OK;
}

int FacadePhraseIndex::get_range(guint8 phrase_index, /* out */ PhraseIndexRange & range){
    SubPhraseIndex * sub_phrase = m_sub_phrase_indices[phrase_index];
    if ( !sub_phrase )
        return ERROR_NO_SUB_PHRASE_INDEX;

    int result = sub_phrase->get_range(range);
    if ( result )
        return result;

    range.m_range_begin = PHRASE_INDEX_MAKE_TOKEN(phrase_index, range.m_range_begin);
    range.m_range_end = PHRASE_INDEX_MAKE_TOKEN(phrase_index, range.m_range_end);
    return ERROR_OK;
}

int SubPhraseIndex::get_range(/* out */ PhraseIndexRange & range){
    const table_offset_t * begin = (const table_offset_t *)m_phrase_index.begin();
    const table_offset_t * end = (const table_offset_t *)m_phrase_index.end();

    if (begin == end) {
        /* skip empty sub phrase index. */
        range.m_range_begin = 1;
        range.m_range_end = 1;
        return ERROR_OK;
    }

    /* remove trailing zeros. */
    const table_offset_t * poffset = NULL;
    for (poffset = end; poffset > begin + 1; --poffset) {
        if (0 !=  *(poffset - 1))
            break;
    }

    range.m_range_begin = 1; /* token starts with 1 in gen_pinyin_table. */
    range.m_range_end = poffset - begin; /* removed zeros. */

    return ERROR_OK;
}

bool FacadePhraseIndex::compact(){
    for ( size_t index = 0; index < PHRASE_INDEX_LIBRARY_COUNT; ++index) {
        SubPhraseIndex * sub_phrase = m_sub_phrase_indices[index];
        if ( !sub_phrase )
            continue;

        PhraseIndexRange range;
        int result = sub_phrase->get_range(range);
        if ( result != ERROR_OK )
            continue;

        SubPhraseIndex * new_sub_phrase =  new SubPhraseIndex;

        PhraseItem item;
        for ( phrase_token_t token = range.m_range_begin;
              token < range.m_range_end;
              ++token ) {
            result = sub_phrase->get_phrase_item(token, item);
            if ( result != ERROR_OK )
                continue;
            new_sub_phrase->add_phrase_item(token, &item);
        }

        delete sub_phrase;
        m_sub_phrase_indices[index] = new_sub_phrase;
    }
    return true;
}

bool SubPhraseIndex::mask_out(phrase_token_t mask, phrase_token_t value){
    PhraseIndexRange range;
    if (ERROR_OK != get_range(range))
        return false;

    /* calculate mask and value for sub phrase index. */
    mask &= PHRASE_MASK; value &= PHRASE_MASK;

    for (phrase_token_t token = range.m_range_begin;
         token < range.m_range_end; ++token) {
        if ((token & mask) != value)
            continue;

        PhraseItem * item = NULL;
        remove_phrase_item(token, item);
        if (item)
            delete item;
    }

    return true;
}

bool FacadePhraseIndex::mask_out(guint8 phrase_index,
                                 phrase_token_t mask,
                                 phrase_token_t value){
    SubPhraseIndex * & sub_phrases = m_sub_phrase_indices[phrase_index];
    if (!sub_phrases)
        return false;

    /* check mask and value. */
    phrase_token_t index_mask = PHRASE_INDEX_LIBRARY_INDEX(mask);
    phrase_token_t index_value = PHRASE_INDEX_LIBRARY_INDEX(value);

    if ((phrase_index & index_mask ) != index_value)
        return false;

    m_total_freq -= sub_phrases->get_phrase_index_total_freq();
    bool retval = sub_phrases->mask_out(mask, value);
    m_total_freq += sub_phrases->get_phrase_index_total_freq();

    return retval;
}


static bool _peek_header(PhraseIndexLogger * logger,
                         guint32 & old_total_freq){
    old_total_freq = 0;

    size_t header_count = 0;
    LOG_TYPE log_type; phrase_token_t token;
    MemoryChunk oldchunk, newchunk;

    while (logger->has_next_record()) {
        bool retval = logger->next_record
            (log_type, token, &oldchunk, &newchunk);

        if (!retval)
            break;

        if (LOG_MODIFY_HEADER != log_type)
            continue;

        ++header_count;

        oldchunk.get_content(0, &old_total_freq, sizeof(guint32));
    }

    /* 1 for normal case, 0 for corrupted file. */
    assert(1 >= header_count);

    return  1 == header_count? true : false;
}

bool _compute_new_header(PhraseIndexLogger * logger,
                         phrase_token_t mask,
                         phrase_token_t value,
                         guint32 & new_total_freq) {

    LOG_TYPE log_type = LOG_INVALID_RECORD;
    phrase_token_t token = null_token;
    MemoryChunk oldchunk, newchunk;
    PhraseItem olditem, newitem;

    while(logger->has_next_record()) {
        bool retval = logger->next_record
            (log_type, token, &oldchunk, &newchunk);

        if (!retval)
            break;

        if (LOG_MODIFY_HEADER == log_type)
            continue;

        if ((token & mask) == value)
            continue;

        switch(log_type) {
        case LOG_ADD_RECORD:{
            assert( 0 == oldchunk.size() );
            newitem.m_chunk.set_chunk(newchunk.begin(), newchunk.size(),
                                      NULL);
            new_total_freq += newitem.get_unigram_frequency();
            break;
        }
        case LOG_REMOVE_RECORD:{
            assert( 0 == newchunk.size() );
            olditem.m_chunk.set_chunk(oldchunk.begin(), oldchunk.size(),
                                      NULL);
            new_total_freq -= olditem.get_unigram_frequency();
            break;
        }
        case LOG_MODIFY_RECORD:{
            olditem.m_chunk.set_chunk(oldchunk.begin(), oldchunk.size(),
                                      NULL);
            new_total_freq -= olditem.get_unigram_frequency();

            newitem.m_chunk.set_chunk(newchunk.begin(), newchunk.size(),
                                      NULL);
            new_total_freq += newitem.get_unigram_frequency();
            break;
        }
        default:
            assert(false);
        }
    }

    return true;
}

static bool _write_header(PhraseIndexLogger * logger,
                          guint32 & old_total_freq,
                          guint32 & new_total_freq) {
    MemoryChunk oldheader, newheader;
    oldheader.set_content(0, &old_total_freq, sizeof(guint32));
    newheader.set_content(0, &new_total_freq, sizeof(guint32));
    logger->append_record(LOG_MODIFY_HEADER, null_token,
                          &oldheader, &newheader);
    return true;
}

static bool _mask_out_records(PhraseIndexLogger * oldlogger,
                              phrase_token_t mask,
                              phrase_token_t value,
                              PhraseIndexLogger * newlogger) {
    LOG_TYPE log_type = LOG_INVALID_RECORD;
    phrase_token_t token = null_token;
    MemoryChunk oldchunk, newchunk;

    while(oldlogger->has_next_record()) {
        bool retval = oldlogger->next_record
            (log_type, token, &oldchunk, &newchunk);

        if (!retval)
            break;

        if (LOG_MODIFY_HEADER == log_type)
            continue;

        if ((token & mask) == value)
            continue;

        newlogger->append_record(log_type, token, &oldchunk, &newchunk);
    }

    return true;
}

PhraseIndexLogger * mask_out_phrase_index_logger
(PhraseIndexLogger * oldlogger, phrase_token_t mask,
 phrase_token_t value) {
    PhraseIndexLogger * newlogger = new PhraseIndexLogger;
    guint32 old_total_freq = 0, new_total_freq = 0;

    /* peek the header value. */
    if (!_peek_header(oldlogger, old_total_freq))
        return newlogger;

    new_total_freq = old_total_freq;

    /* compute the new header based on add/modify/remove records. */
    oldlogger->rewind();
    if (!_compute_new_header(oldlogger, mask, value, new_total_freq))
        return newlogger;

    /* write out the modify header record. */
    _write_header(newlogger, old_total_freq, new_total_freq);

    /* mask out the matched records. */
    oldlogger->rewind();
    _mask_out_records(oldlogger, mask, value, newlogger);

    return newlogger;
}

};
