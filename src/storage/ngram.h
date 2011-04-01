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
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
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
private:
    MemoryChunk m_chunk;
    SingleGram(void * buffer, size_t length);
public:
    /* Null Constructor */
    SingleGram();
    /* retrieve all items */
    bool retrieve_all(/* out */ BigramPhraseWithCountArray array);

    /* search method */
    /* the array result contains many items */
    bool search(/* in */ PhraseIndexRange * range, 
	       /* out */ BigramPhraseArray array);

    bool get_freq(/* in */ phrase_token_t token,
	       /* out */ guint32 & freq); 
    
    /* set_freq method
     */
    bool set_freq(/* in */ phrase_token_t token,
		  guint32 freq);
    
    /* get_total_freq method
     * used in user bigram table
     */
    bool get_total_freq(guint32 & total);

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
    DB * m_system;
    DB * m_user;

    void reset(){
	if ( m_system ){
	    m_system->close(m_system, 0);
	    m_system = NULL;
	}
	if ( m_user ){
	    m_user->close(m_user, 0);
	    m_user = NULL;
	}
    }

public:
    Bigram(){
	m_system = NULL; m_user = NULL;
    }

    ~Bigram(){
	reset();
    }

    /* attach system and user bi-gram */
    /* when with training systemdb is NULL, only user_gram */
    bool attach(const char * systemfile, const char * userfile);

    /* load/store one single gram */
    bool load(/* in */ phrase_token_t index,
              /* out */ SingleGram * & system_gram,
              /* out */ SingleGram * & user_gram);

    bool store(/* in */ phrase_token_t index,
               /* in */ SingleGram * user_gram);

    /* array of phrase_token_t items, for parameter estimation. */
    bool get_all_items(/* out */ GArray * system,
                       /* out */ GArray * user);
};

};

using namespace pinyin;


#endif
