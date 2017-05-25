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

#ifndef PHRASE_INDEX_H
#define PHRASE_INDEX_H

#include <stdio.h>
#include <glib.h>
#include "novel_types.h"
#include "chewing_key.h"
#include "pinyin_parser2.h"
#include "zhuyin_parser2.h"
#include "pinyin_phrase3.h"
#include "memory_chunk.h"
#include "phrase_index_logger.h"
#include "table_info.h"

/**
 * Phrase Index File Format
 *
 * Indirect Index: Index by Token
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * + Phrase Offset + Phrase Offset + Phrase Offset + ......  +
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * Phrase Content:
 * ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * + Phrase Length + number of  Pronunciations  + Uni-gram Frequency+
 * ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * + Phrase String(UCS4) + n Pronunciations with Frequency +
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 */

namespace pinyin{

/* Store delta info by phrase index logger in user home directory.
 */

const size_t phrase_item_header = sizeof(guint8) + sizeof(guint8) + sizeof(guint32);

/**
 * PhraseItem:
 *
 * The PhraseItem to access the items in phrase index.
 *
 */
class PhraseItem{
    friend class SubPhraseIndex;
    friend bool _compute_new_header(PhraseIndexLogger * logger,
                                    phrase_token_t mask,
                                    phrase_token_t value,
                                    guint32 & new_total_freq);

private:
    MemoryChunk m_chunk;
    bool set_n_pronunciation(guint8 n_prouns);
public:
    /**
     * PhraseItem::PhraseItem:
     *
     * The constructor of the PhraseItem.
     *
     */
    PhraseItem(){
        m_chunk.set_size(phrase_item_header);
        memset(m_chunk.begin(), 0, m_chunk.size());
    }

#if 0
    PhraseItem(MemoryChunk & chunk){
        m_chunk.set_content(0, chunk->begin(), chunk->size());
        assert ( m_chunk.size() >= phrase_item_header);
    }
#endif

    /**
     * PhraseItem::get_phrase_length:
     * @returns: the length of this phrase item.
     *
     * Get the length of this phrase item.
     *
     */
    guint8 get_phrase_length(){
        char * buf_begin = (char *)m_chunk.begin();
        return (*(guint8 *)buf_begin);
    }

    /**
     * PhraseItem::get_n_pronunciation:
     * @returns: the number of the pronunciations.
     *
     * Get the number of the pronunciations.
     *
     */
    guint8 get_n_pronunciation(){
        char * buf_begin = ( char *) m_chunk.begin();
        return (*(guint8 *)(buf_begin + sizeof(guint8)));
    }

    /**
     * PhraseItem::get_unigram_frequency:
     * @returns: the uni-gram frequency of this phrase item.
     *
     * Get the uni-gram frequency of this phrase item.
     *
     */
    guint32 get_unigram_frequency(){
        char * buf_begin = (char *)m_chunk.begin();
        return (*(guint32 *)(buf_begin + sizeof(guint8) + sizeof(guint8)));
    }

    /**
     * PhraseItem::get_pronunciation_possibility:
     * @keys: the pronunciation keys.
     * @returns: the possibility of this phrase item pronounces the pinyin.
     *
     * Get the possibility of this phrase item pronounces the pinyin.
     *
     */
    gfloat get_pronunciation_possibility(ChewingKey * keys){
        guint8 phrase_length = get_phrase_length();
        guint8 npron = get_n_pronunciation();
        size_t offset = phrase_item_header + phrase_length * sizeof (ucs4_t);
        char * buf_begin = (char *)m_chunk.begin();
        guint32 matched = 0, total_freq =0;
        for ( int i = 0 ; i < npron ; ++i){
            char * chewing_begin = buf_begin + offset +
                i * (phrase_length * sizeof(ChewingKey) + sizeof(guint32));
            guint32 * freq = (guint32 *)(chewing_begin +
                                         phrase_length * sizeof(ChewingKey));
            total_freq += *freq;
            if ( 0 == pinyin_compare_with_tones(keys, (ChewingKey *)chewing_begin,
                                                phrase_length) ){
                matched += *freq;
            }
        }

#if 1
        /* an additional safe guard for chewing. */
        if ( 0 == total_freq )
            return 0;
#endif

        /* used preprocessor to avoid zero freq, in gen_pinyin_table. */
        gfloat retval = matched / (gfloat) total_freq;
        return retval;
    }

    /**
     * PhraseItem::increase_pronunciation_possibility:
     * @keys: the pronunciation keys.
     * @delta: the delta to be added to the pronunciation keys.
     *
     * Add the delta to the pronunciation of the pronunciation keys.
     *
     */
    void increase_pronunciation_possibility(ChewingKey * keys,
                                            gint32 delta);

    /**
     * PhraseItem::get_phrase_string:
     * @phrase: the ucs4 character buffer.
     * @returns: whether the get operation is successful.
     *
     * Get the ucs4 characters of this phrase item.
     *
     */
    bool get_phrase_string(ucs4_t * phrase);

    /**
     * PhraseItem::set_phrase_string:
     * @phrase_length: the ucs4 character length of this phrase item.
     * @phrase: the ucs4 character buffer.
     * @returns: whether the set operation is successful.
     *
     * Set the length and ucs4 characters of this phrase item.
     *
     */
    bool set_phrase_string(guint8 phrase_length, ucs4_t * phrase);

    /**
     * PhraseItem::get_nth_pronunciation:
     * @index: the pronunciation index.
     * @keys: the pronunciation keys.
     * @freq: the frequency of the pronunciation.
     * @returns: whether the get operation is successful.
     *
     * Get the nth pronunciation of this phrase item.
     *
     */
    bool get_nth_pronunciation(size_t index, 
                               /* out */ ChewingKey * keys,
                               /* out */ guint32 & freq);

    /**
     * PhraseItem::add_pronunciation:
     * @keys: the pronunciation keys.
     * @delta: the delta of the frequency of the pronunciation.
     * @returns: whether the add operation is successful.
     *
     * Add one pronunciation.
     *
     */
    bool add_pronunciation(ChewingKey * keys, guint32 delta);

    /**
     * PhraseItem::remove_nth_pronunciation:
     * @index: the pronunciation index.
     *
     * Remove the nth pronunciation.
     *
     * Note: Normally don't change the first pronunciation,
     * which decides the token number.
     *
     */
    void remove_nth_pronunciation(size_t index);

    bool operator == (const PhraseItem & rhs) const{
        if (m_chunk.size() != rhs.m_chunk.size())
            return false;
        return memcmp(m_chunk.begin(), rhs.m_chunk.begin(),
                      m_chunk.size()) == 0;
    }

    bool operator != (const PhraseItem & rhs) const{
        return ! (*this == rhs);
    }
};

/*
 *  In Sub Phrase Index, token == (token & PHRASE_MASK).
 */

/**
 * SubPhraseIndex:
 *
 * The SubPhraseIndex class for internal usage.
 *
 */
class SubPhraseIndex{
private:
    guint32 m_total_freq;
    MemoryChunk m_phrase_index;
    MemoryChunk m_phrase_content;
    MemoryChunk * m_chunk;

    void reset(){
        m_total_freq = 0;
        m_phrase_index.set_size(0);
        m_phrase_content.set_size(0);
        if ( m_chunk ){
            delete m_chunk;
            m_chunk = NULL;
        }
    }

public:
    /**
     * SubPhraseIndex::SubPhraseIndex:
     *
     * The constructor of the SubPhraseIndex.
     *
     */
    SubPhraseIndex():m_total_freq(0){
        m_chunk = NULL;
    }

    /**
     * SubPhraseIndex::~SubPhraseIndex:
     *
     * The destructor of the SubPhraseIndex.
     *
     */
    ~SubPhraseIndex(){
        reset();
    }
    
    /**
     * SubPhraseIndex::load:
     * @chunk: the memory chunk of the binary sub phrase index.
     * @offset: the begin of binary data in the memory chunk.
     * @end: the end of binary data in the memory chunk.
     * @returns: whether the load operation is successful.
     *
     * Load the sub phrase index from the memory chunk.
     *
     */
    bool load(MemoryChunk * chunk, 
              table_offset_t offset, table_offset_t end);

    /**
     * SubPhraseIndex::store:
     * @new_chunk: the new memory chunk to store this sub phrase index.
     * @offset: the begin of binary data in the memory chunk.
     * @end: the end of stored binary data in the memory chunk.
     * @returns: whether the store operation is successful.
     *
     * Store the sub phrase index to the new memory chunk.
     *
     */
    bool store(MemoryChunk * new_chunk, 
               table_offset_t offset, table_offset_t & end);

    /**
     * SubPhraseIndex::diff:
     * @oldone: the original content of sub phrase index.
     * @logger: the delta information of user self-learning data.
     * @returns: whether the diff operation is successful.
     *
     * Compare this sub phrase index with the original content of the system
     * sub phrase index to generate the logger of difference.
     *
     * Note: Switch to logger format to reduce user space storage.
     *
     */
    bool diff(SubPhraseIndex * oldone, PhraseIndexLogger * logger);

    /**
     * SubPhraseIndex::merge:
     * @logger: the logger of difference in user home directory.
     * @returns: whether the merge operation is successful.
     *
     * Merge the user logger of difference with this sub phrase index.
     *
     */
    bool merge(PhraseIndexLogger * logger);

    /**
     * SubPhraseIndex::get_range:
     * @range: the token range.
     * @returns: whether the get operation is successful.
     *
     * Get the token range in this sub phrase index.
     *
     */
    int get_range(/* out */ PhraseIndexRange & range);

    /**
     * SubPhraseIndex::get_phrase_index_total_freq:
     * @returns: the total frequency of this sub phrase index.
     *
     * Get the total frequency of this sub phrase index.
     *
     * Note: maybe call it "Zero-gram".
     *
     */
    guint32 get_phrase_index_total_freq();

    /**
     * SubPhraseIndex::add_unigram_frequency:
     * @token: the phrase token.
     * @delta: the delta value of the phrase token.
     * @returns: the status of the add operation.
     *
     * Add delta value to the phrase of the token.
     *
     * Note: this method is a fast path to add delta value.
     * Maybe use the get_phrase_item method instead in future.
     *
     */
    int add_unigram_frequency(phrase_token_t token, guint32 delta);

    /**
     * SubPhraseIndex::get_phrase_item:
     * @token: the phrase token.
     * @item: the phrase item of the token.
     * @returns: the status of the get operation.
     *
     * Get the phrase item from this sub phrase index.
     *
     * Note:get_phrase_item function can't modify the phrase item size,
     * but can increment the freq of the special pronunciation,
     * or change the content without size increasing.
     *
     */
    int get_phrase_item(phrase_token_t token, PhraseItem & item);

    /**
     * SubPhraseIndex::add_phrase_item:
     * @token: the phrase token.
     * @item: the phrase item of the token.
     * @returns: the status of the add operation.
     *
     * Add the phrase item to this sub phrase index.
     *
     */
    int add_phrase_item(phrase_token_t token, PhraseItem * item);

    /**
     * SubPhraseIndex::remove_phrase_item:
     * @token: the phrase token.
     * @item: the removed phrase item of the token.
     * @returns: the status of the remove operation.
     *
     * Remove the phrase item of the token.
     *
     * Note: this remove_phrase_item method will substract the unigram
     * frequency of the removed item from m_total_freq.
     *
     */
    int remove_phrase_item(phrase_token_t token, /* out */ PhraseItem * & item);

    /**
     * SubPhraseIndex::mask_out:
     * @mask: the mask.
     * @value: the value.
     * @returns: whether the mask out operation is successful.
     *
     * Mask out the matched phrase items.
     *
     */
    bool mask_out(phrase_token_t mask, phrase_token_t value);
};

/**
 * FacadePhraseIndex:
 *
 * The facade class of phrase index.
 *
 */
class FacadePhraseIndex{
private:
    guint32 m_total_freq;
    SubPhraseIndex * m_sub_phrase_indices[PHRASE_INDEX_LIBRARY_COUNT];
public:
    /**
     * FacadePhraseIndex::FacadePhraseIndex:
     *
     * The constructor of the FacadePhraseIndex.
     *
     */
    FacadePhraseIndex(){
        m_total_freq = 0;
        memset(m_sub_phrase_indices, 0, sizeof(m_sub_phrase_indices));
    }

    /**
     * FacadePhraseIndex::~FacadePhraseIndex:
     *
     * The destructor of the FacadePhraseIndex.
     *
     */
    ~FacadePhraseIndex(){
        for ( size_t i = 0; i < PHRASE_INDEX_LIBRARY_COUNT; ++i){
            if ( m_sub_phrase_indices[i] ){
                delete m_sub_phrase_indices[i];
                m_sub_phrase_indices[i] = NULL;
            }
        }
    }

    /**
     * FacadePhraseIndex::load_text:
     * @phrase_index: the index of sub phrase index to be loaded.
     * @infile: the textual format file of the phrase table.
     * @type: the type of phonetic table.
     * @returns: whether the load operation is successful.
     *
     * Load one sub phrase index from the textual format file.
     * Note: load sub phrase index according to the config in future.
     *
     */
    bool load_text(guint8 phrase_index, FILE * infile,
                   TABLE_PHONETIC_TYPE type);

    /**
     * FacadePhraseIndex::load:
     * @phrase_index: the index of sub phrase index to be loaded.
     * @chunk: the memory chunk of sub phrase index to be loaded.
     * @returns: whether the load operation is successful.
     *
     * Load one sub phrase index from the memory chunk.
     *
     */
    bool load(guint8 phrase_index, MemoryChunk * chunk);

    /**
     * FacadePhraseIndex::store:
     * @phrase_index: the index of sub phrase index to be stored.
     * @new_chunk: the memory chunk of sub phrase index to be stored.
     * @returns: whether the store operation is successful.
     *
     * Store one sub phrase index to the memory chunk.
     *
     */
    bool store(guint8 phrase_index, MemoryChunk * new_chunk);

    /**
     * FacadePhraseIndex::unload:
     * @phrase_index: the index of sub phrase index to be unloaded.
     * @returns: whether the unload operation is successful.
     *
     * Unload one sub phrase index.
     *
     */
    bool unload(guint8 phrase_index);


    /**
     * FacadePhraseIndex::diff:
     * @phrase_index: the index of sub phrase index to be differed.
     * @oldchunk: the original content of sub phrase index.
     * @newlog: the delta information of user self-learning data.
     * @returns: whether the diff operation is successful.
     *
     * Store user delta information in the logger format.
     *
     * Note: the ownership of oldchunk is transfered here.
     *
     */
    bool diff(guint8 phrase_index, MemoryChunk * oldchunk,
              MemoryChunk * newlog);

    /**
     * FacadePhraseIndex::merge:
     * @phrase_index: the index of sub phrase index to be merged.
     * @log: the logger of difference in user home directory.
     * @returns: whether the merge operation is successful.
     *
     * Merge the user logger of difference with the sub phrase index.
     *
     * Note: the ownership of log is transfered here.
     *
     */
    bool merge(guint8 phrase_index, MemoryChunk * log);

    /**
     * FacadePhraseIndex::merge_with_mask:
     * @phrase_index: the index of sub phrase index to be merged.
     * @log: the logger of difference in user home directory.
     * @mask: the mask.
     * @value: the value.
     * @returns: whether the merge operation is successful.
     *
     * Merge the user logger of difference with mask operation.
     *
     * Note: the ownership of log is transfered here.
     *
     */
    bool merge_with_mask(guint8 phrase_index, MemoryChunk * log,
                         phrase_token_t mask, phrase_token_t value);

    /**
     * FacadePhraseIndex::compact:
     * @returns: whether the compact operation is successful.
     *
     * Compat all sub phrase index memory usage.
     *
     */
    bool compact();

    /**
     * FacadePhraseIndex::mask_out:
     * @phrase_index: the index of sub phrase index.
     * @mask: the mask.
     * @value: the value.
     * @returns: whether the mask out operation is successful.
     *
     * Mask out the matched phrase items.
     *
     * Note: should call compact() after the mask out operation.
     *
     */
    bool mask_out(guint8 phrase_index,
                  phrase_token_t mask, phrase_token_t value);

    /**
     * FacadePhraseIndex::get_sub_phrase_range:
     * @min_index: the minimal sub phrase index.
     * @max_index: the maximal sub phrase index.
     * @returns: the status of the get operation.
     *
     * Get the minimum and maximum of the sub phrase index.
     *
     */
    int get_sub_phrase_range(guint8 & min_index, guint8 & max_index);

    /**
     * FacadePhraseIndex::get_range:
     * @phrase_index: the index of sub phrase index.
     * @range: the token range of the sub phrase index.
     * @returns: the status of the get operation.
     *
     * Get the token range of the sub phrase index.
     *
     */
    int get_range(guint8 phrase_index, /* out */ PhraseIndexRange & range);

    /**
     * FacadePhraseIndex::get_phrase_index_total_freq:
     * @returns: the total freq of the facade phrase index.
     *
     * Get the total freq of the facade phrase index.
     *
     * Note: maybe call it "Zero-gram".
     *
     */
    guint32 get_phrase_index_total_freq(){
        return m_total_freq;
    }

    /**
     * FacadePhraseIndex::add_unigram_frequency:
     * @token: the phrase token.
     * @delta: the delta value of the phrase token.
     * @returns: the status of the add operation.
     *
     * Add delta value to the phrase of the token.
     *
     */
    int add_unigram_frequency(phrase_token_t token, guint32 delta){
        guint8 index = PHRASE_INDEX_LIBRARY_INDEX(token);
        SubPhraseIndex * sub_phrase = m_sub_phrase_indices[index];
        if ( !sub_phrase )
            return ERROR_NO_SUB_PHRASE_INDEX;
        m_total_freq += delta;
        return sub_phrase->add_unigram_frequency(token, delta);
    }

    /**
     * FacadePhraseIndex::get_phrase_item:
     * @token: the phrase token.
     * @item: the phrase item of the token.
     * @returns: the status of the get operation.
     *
     * Get the phrase item from the facade phrase index.
     *
     */
    int get_phrase_item(phrase_token_t token, PhraseItem & item){
        guint8 index = PHRASE_INDEX_LIBRARY_INDEX(token);
        SubPhraseIndex * sub_phrase = m_sub_phrase_indices[index];
        if ( !sub_phrase )
            return ERROR_NO_SUB_PHRASE_INDEX;
        return sub_phrase->get_phrase_item(token, item);
    }

    /**
     * FacadePhraseIndex::add_phrase_item:
     * @token: the phrase token.
     * @item: the phrase item of the token.
     * @returns: the status of the add operation.
     *
     * Add the phrase item to the facade phrase index.
     *
     */
    int add_phrase_item(phrase_token_t token, PhraseItem * item){
        guint8 index = PHRASE_INDEX_LIBRARY_INDEX(token);
        SubPhraseIndex * & sub_phrase = m_sub_phrase_indices[index];
        if ( !sub_phrase ){
            sub_phrase = new SubPhraseIndex;
        }   
        m_total_freq += item->get_unigram_frequency();
        return sub_phrase->add_phrase_item(token, item);
    }

    /**
     * FacadePhraseIndex::remove_phrase_item:
     * @token: the phrase token.
     * @item: the removed phrase item of the token.
     * @returns: the status of the remove operation.
     *
     * Remove the phrase item of the token.
     *
     */
    int remove_phrase_item(phrase_token_t token, PhraseItem * & item){
        guint8 index = PHRASE_INDEX_LIBRARY_INDEX(token);
        SubPhraseIndex * & sub_phrase = m_sub_phrase_indices[index];
        if ( !sub_phrase ){
            return ERROR_NO_SUB_PHRASE_INDEX;
        }
        int result = sub_phrase->remove_phrase_item(token, item);
        if ( result )
            return result;
        m_total_freq -= item->get_unigram_frequency();
        return result;
    }

    /**
     * FacadePhraseIndex::prepare_ranges:
     * @ranges: the ranges to be prepared.
     * @returns: whether the prepare operation is successful.
     *
     * Prepare the ranges.
     *
     */
    bool prepare_ranges(PhraseIndexRanges ranges) {
        /* assume memset(ranges, 0, sizeof(ranges)); */
        for (size_t i = 0; i < PHRASE_INDEX_LIBRARY_COUNT; ++i) {
            GArray * & range = ranges[i];
            assert(NULL == range);

            SubPhraseIndex * sub_phrase = m_sub_phrase_indices[i];
            if (sub_phrase) {
                range = g_array_new(FALSE, FALSE, sizeof(PhraseIndexRange));
            }
        }
        return true;
    }

    /**
     * FacadePhraseIndex::clear_ranges:
     * @ranges: the ranges to be cleared.
     * @returns: whether the clear operation is successful.
     *
     * Clear the ranges.
     *
     */
    bool clear_ranges(PhraseIndexRanges ranges) {
        for (size_t i = 0; i < PHRASE_INDEX_LIBRARY_COUNT; ++i) {
            GArray * range = ranges[i];
            if (range) {
                g_array_set_size(range, 0);
            }
        }
        return true;
    }

    /**
     * FacadePhraseIndex::destroy_ranges:
     * @ranges: the ranges to be destroyed.
     * @returns: whether the destroy operation is successful.
     *
     * Destroy the ranges.
     *
     */
    bool destroy_ranges(PhraseIndexRanges ranges) {
        for (size_t i = 0; i < PHRASE_INDEX_LIBRARY_COUNT; ++i) {
            GArray * & range = ranges[i];
            if (range) {
                g_array_free(range, TRUE);
                range = NULL;
            }
        }
        return true;
    }

    /**
     * FacadePhraseIndex::prepare_tokens:
     * @tokens: the tokens to be prepared.
     * @returns: whether the prepare operation is successful.
     *
     * Prepare the tokens.
     *
     */
    bool prepare_tokens(PhraseTokens tokens) {
        /* assume memset(tokens, 0, sizeof(tokens)); */
        for (size_t i = 0; i < PHRASE_INDEX_LIBRARY_COUNT; ++i) {
            GArray * & token = tokens[i];
            assert(NULL == token);

            SubPhraseIndex * sub_phrase = m_sub_phrase_indices[i];
            if (sub_phrase) {
                token = g_array_new(FALSE, FALSE, sizeof(phrase_token_t));
            }
        }
        return true;
    }

    /**
     * FacadePhraseIndex::clear_tokens:
     * @tokens: the tokens to be cleared.
     * @return: whether the clear operation is successful.
     *
     * Clear the tokens.
     *
     */
    bool clear_tokens(PhraseTokens tokens) {
        for (size_t i = 0; i < PHRASE_INDEX_LIBRARY_COUNT; ++i) {
            GArray * token = tokens[i];
            if (token) {
                g_array_set_size(token, 0);
            }
        }
        return true;
    }

    /**
     * FacadePhraseIndex::destroy_tokens:
     * @tokens: the tokens to be destroyed.
     * @returns: whether the destroy operation is successful.
     *
     * Destroy the tokens.
     *
     */
    bool destroy_tokens(PhraseTokens tokens) {
        for (size_t i = 0; i < PHRASE_INDEX_LIBRARY_COUNT; ++i) {
            GArray * & token = tokens[i];
            if (token) {
                g_array_free(token, TRUE);
                token = NULL;
            }
        }
        return true;
    }

    /**
     * FacadePhraseIndex::create_sub_phrase:
     * @index: the phrase index to be created.
     * @returns: the result of the create operation.
     *
     * Create the sub phrase index.
     *
     */
    int create_sub_phrase(guint8 index) {
        SubPhraseIndex * & sub_phrase = m_sub_phrase_indices[index];
        if (sub_phrase) {
            return ERROR_ALREADY_EXISTS;
        }

        sub_phrase = new SubPhraseIndex;

        return ERROR_OK;
    }
};

PhraseIndexLogger * mask_out_phrase_index_logger
(PhraseIndexLogger * oldlogger, phrase_token_t mask, phrase_token_t value);

};

#endif
