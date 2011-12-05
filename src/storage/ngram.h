/* 
 *  libpinyin
 *  Library to deal with pinyin.
 *  
 *  Copyright (C) 2006-2007 Peng Wu
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

#ifndef NGRAM_H
#define NGRAM_H

#include <db.h>

namespace pinyin{

class Bigram;

/* Note:
 * When transfer from system ngram to user ngram, 
 *   if user ngram doesn't exist,
 *     copy total freq from system ngram to user ngram,
 *     so the total freq exists.
 *   if item freq don't exist, copy item freq from system to user ngram,
 *     so the item freq exists.
 *     if user ngram already exists(always true), increases the total freq,
 *     if item ngram already exists(always true), increases the freq.
 */

class SingleGram{
    friend class Bigram;
    friend bool merge_single_gram(SingleGram * merged,
                                  const SingleGram * system,
                                  const SingleGram * user);

private:
    MemoryChunk m_chunk;
    SingleGram(void * buffer, size_t length);
public:
    /* Null Constructor */
    SingleGram();
    /* retrieve all items */
    bool retrieve_all(/* out */ BigramPhraseWithCountArray array) const;

    /* search method */
    /* the array result contains many items */
    bool search(/* in */ PhraseIndexRange * range,
	       /* out */ BigramPhraseArray array) const;

    /* insert_freq method
     */
    bool insert_freq(/* in */ phrase_token_t token,
                     /* in */ guint32 freq);

    /* remove_freq method
     */
    bool remove_freq(/* in */ phrase_token_t token,
                     /* out */ guint32 & freq);

    /* get_freq method
     */
    bool get_freq(/* in */ phrase_token_t token,
	       /* out */ guint32 & freq) const;
    
    /* set_freq method
     */
    bool set_freq(/* in */ phrase_token_t token,
		  /* in */ guint32 freq);
    
    /* get_total_freq method
     * used in user bigram table
     */
    bool get_total_freq(guint32 & total) const;

    /* set_total_freq method
     * used in user bigram table
     */
    bool set_total_freq(guint32 total);
    
    /* prune one method
     * only used in training
     */
    bool prune();
};

class Bigram{
private:
    DB * m_db;

    void reset(){
	if ( m_db ){
        m_db->sync(m_db, 0);
	    m_db->close(m_db, 0);
	    m_db = NULL;
	}
    }

public:
    Bigram(){
	m_db = NULL;
    }

    ~Bigram(){
	reset();
    }

    /* load/save berkeley db in memory. */
    bool load_db(const char * dbfile);
    bool save_db(const char * dbfile);

    /* attach bi-gram */
    bool attach(const char * dbfile, guint32 flags);

    /* load/store one single gram */
    bool load(/* in */ phrase_token_t index,
              /* out */ SingleGram * & single_gram);

    bool store(/* in */ phrase_token_t index,
               /* in */ SingleGram * single_gram);

    /* array of phrase_token_t items, for parameter estimation. */
    bool get_all_items(/* out */ GArray * items);
};

bool merge_single_gram(SingleGram * merged, const SingleGram * system,
                       const SingleGram * user);

};

#endif
