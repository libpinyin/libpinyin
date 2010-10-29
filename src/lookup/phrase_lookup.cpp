/* 
 *  libpinyin
 *  Library to deal with pinyin.
 *  
 *  Copyright (C) 2010 Peng Wu
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

#include "stl_lite.h"
#include "novel_types.h"
#include "phrase_index.h"
#include "phrase_large_table.h"
#include "ngram.h"
#include "phrase_lookup.h"

const gfloat PhraseLookup::bigram_lambda;
const gfloat PhraseLookup::unigram_lambda;

PhraseLookup::PhraseLookup(PhraseLargeTable * phrase_table,
                           FacadePhraseIndex * phrase_index,
                           Bigram * bigram){
    m_phrase_table = phrase_table;
    m_phrase_index = phrase_index;
    m_bigram = bigram;

    m_steps_index = g_ptr_array_new();
    m_steps_content = g_ptr_array_new();
}





bool PhraseLookup::convert_to_utf8(phrase_token_t token, /* out */ char * & phrase){
    m_phrase_index->get_phrase_item(token, m_cache_phrase_item);
    utf16_t buffer[MAX_PHRASE_LENGTH];
    m_cache_phrase_item.get_phrase_string(buffer);
    guint8 length = m_cache_phrase_item.get_phrase_length();
    phrase = g_utf16_to_utf8(buffer, length, NULL, NULL, NULL);
    return true;
}
