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

#ifndef NGRAM_H
#define NGRAM_H

#include "config.h"
#include <glib.h>
#include "novel_types.h"

#ifdef HAVE_BERKELEY_DB
#include "ngram_bdb.h"
#endif

#ifdef HAVE_KYOTO_CABINET
#include "ngram_kyotodb.h"
#endif

namespace pinyin{

class Bigram;

/** Note:
 *  The system single gram contains the trained freqs.
 *  The user single gram contains the delta freqs.
 *  During the Viterbi beam search, use merge_single_gram to merge the system
 *    single gram and the user single gram.
 */


/**
 * SingleGram:
 *
 * The single gram in the bi-gram.
 *
 */
class SingleGram{
    friend class Bigram;
    friend bool merge_single_gram(SingleGram * merged,
                                  const SingleGram * system,
                                  const SingleGram * user);

private:
    MemoryChunk m_chunk;
    SingleGram(void * buffer, size_t length, bool copy);
public:
    /**
     * SingleGram::SingleGram:
     *
     * The constructor of the SingleGram.
     *
     */
    SingleGram();
    /**
     * SingleGram::retrieve_all:
     * @array: the GArray to store the retrieved bi-gram phrase item.
     * @returns: whether the retrieve operation is successful.
     *
     * Retrieve all bi-gram phrase items in this single gram.
     *
     */
    bool retrieve_all(/* out */ BigramPhraseWithCountArray array) const;

    /**
     * SingleGram::search:
     * @range: the token range.
     * @array: the GArray to store the matched bi-gram phrase item.
     * @returns: whether the search operation is successful.
     *
     * Search the bi-gram phrase items according to the token range.
     *
     * Note: the array result may contain many items.
     *
     */
    bool search(/* in */ PhraseIndexRange * range,
	       /* out */ BigramPhraseArray array) const;

    /**
     * SingleGram::insert_freq:
     * @token: the phrase token.
     * @freq: the freq of this token.
     * @returns: whether the insert operation is successful.
     *
     * Insert the token with the freq.
     *
     */
    bool insert_freq(/* in */ phrase_token_t token,
                     /* in */ guint32 freq);

    /**
     * SingleGram::remove_freq:
     * @token: the phrase token.
     * @freq: the freq of the removed token.
     * @returns: whether the remove operation is successful.
     *
     * Remove the token.
     *
     */
    bool remove_freq(/* in */ phrase_token_t token,
                     /* out */ guint32 & freq);

    /**
     * SingleGram::get_freq:
     * @token: the phrase token.
     * @freq: the freq of the token.
     * @returns: whether the get operation is successful.
     *
     * Get the freq of the token.
     *
     */
    bool get_freq(/* in */ phrase_token_t token,
                  /* out */ guint32 & freq) const;
    
    /**
     * SingleGram::set_freq:
     * @token: the phrase token.
     * @freq: the freq of the token.
     * @returns: whether the set operation is successful.
     *
     * Set the freq of the token.
     *
     */
    bool set_freq(/* in */ phrase_token_t token,
                  /* in */ guint32 freq);
    
    /**
     * SingleGram::get_total_freq:
     * @total: the total freq of this single gram.
     * @returns: whether the get operation is successful.
     *
     * Get the total freq of this single gram.
     *
     */
    bool get_total_freq(guint32 & total) const;

    /**
     * SingleGram::set_total_freq:
     * @total: the total freq of this single gram.
     * @returns: whether the set operation is successful.
     *
     * Set the total freq of this single gram.
     *
     */
    bool set_total_freq(guint32 total);

    /**
     * SingleGram::get_length:
     * @returns: the number of items in this single gram.
     *
     * Get the number of items in this single gram.
     *
     */
    guint32 get_length();

    /**
     * SingleGram::mask_out:
     * @mask: the mask.
     * @value: the value.
     * @returns: the number of removed items.
     *
     * Mask out the matched items in this single gram.
     *
     */
    guint32 mask_out(phrase_token_t mask, phrase_token_t value);
    
    /**
     * SingleGram::prune:
     * @returns: whether the prune operation is successful.
     *
     * Obsoleted by Katz k mixture model pruning.
     *
     */
    bool prune();
};


/**
 * merge_single_gram:
 * @merged: the merged single gram of system and user single gram.
 * @system: the system single gram to be merged.
 * @user: the user single gram to be merged.
 * @returns: whether the merge operation is successful.
 *
 * Merge the system and user single gram into one merged single gram.
 *
 * Note: Please keep system and user single gram
 * when using merged single gram.
 *
 */
bool merge_single_gram(SingleGram * merged, const SingleGram * system,
                       const SingleGram * user);

};

#endif
