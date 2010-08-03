/* 
 *  novel-pinyin,
 *  A Simplified Chinese Sentence-Based Pinyin Input Method Engine
 *  Based On Markov Model.
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

#ifndef PINYIN_PHRASE_H
#define PINYIN_PHRASE_H

#include <string.h>
#include "stl_lite.h"

namespace novel{

static inline int pinyin_utility_sign(int value){
  if(value > 0)
    return 1;
  else if (value < 0)
    return -1;
  else return 0;
}

inline int pinyin_exact_compare(const PinyinKey key_lhs[], 
				const PinyinKey key_rhs[],
				int phrase_length){
  int i;
  int result;
  for ( i = 0 ; i < phrase_length ; i++){
    result = key_lhs[i].m_initial - key_rhs[i].m_initial;
    if ( result != 0 )
      return pinyin_utility_sign(result);
  }
  for( i = 0 ; i < phrase_length ; i++){
    result = key_lhs[i].m_final - key_rhs[i].m_final;
    if ( result != 0 )
      return pinyin_utility_sign(result);
  }
  for( i = 0 ; i < phrase_length ; i++){
    result = key_lhs[i].m_tone - key_rhs[i].m_tone;
    if ( result != 0 )
      return pinyin_utility_sign(result);
  }
  return 0;
}


inline int pinyin_compare_with_ambiguities(const PinyinCustomSettings &custom,
					   const PinyinKey* key_lhs,
					   const PinyinKey* key_rhs,
					   int phrase_length){
    int i;
    int result;
    for ( i = 0 ; i < phrase_length ; i++){
	result = pinyin_compare_initial
	    (custom, 
	     (PinyinInitial)key_lhs[i].m_initial, 
	     (PinyinInitial)key_rhs[i].m_initial);
	if ( result != 0 )
	    return result;
    }
    for( i = 0 ; i < phrase_length ; i++){
	result = pinyin_compare_final
	    (custom, 
	     (PinyinFinal)key_lhs[i].m_final, 
	     (PinyinFinal)key_rhs[i].m_final);
	if ( result != 0 )
	    return result;
    }
    for( i = 0 ; i < phrase_length ; i++){
	result = pinyin_compare_tone
	    (custom,
	     (PinyinTone)key_lhs[i].m_tone,
	     (PinyinTone)key_rhs[i].m_tone);
	if ( result != 0 )
	    return result;
    }
    return 0;
}

//compute pinyin lower bound
//maybe replace by table lookup
inline void compute_lower_value(const PinyinCustomSettings &custom,
				PinyinKey in_keys[], 
				PinyinKey out_keys[], 
				int phrase_length){
    PinyinKey aKey = in_keys[0];
    
    for ( int i = 0; i < phrase_length; i++){
	int k; int sel;
	aKey = in_keys[i];
	//deal with initial
	sel = aKey.m_initial;
	for( k = aKey.m_initial - 1; k >= PINYIN_ZeroInitial; k--){
	    if ( 0 != pinyin_compare_initial(custom, 
					     (PinyinInitial)k, 
					     (PinyinInitial)aKey.m_initial) )
		break;
	    else
		sel = k;
	}
	aKey.m_initial = (PinyinInitial)sel;
	//deal with final
	sel = aKey.m_final;
	for( k = aKey.m_final - 1; k >= PINYIN_ZeroFinal; k--){
	    if ( 0 != pinyin_compare_final(custom,
					   (PinyinFinal)k,
					   (PinyinFinal)aKey.m_final) )
		break;
	    else
		sel = k;
	}
	aKey.m_final = (PinyinFinal)sel;
	//deal with tone
	sel = aKey.m_tone;
	for( k = aKey.m_tone - 1; k >= PINYIN_ZeroTone; k--){
	    if ( 0 != pinyin_compare_tone(custom,
					  (PinyinTone)k, 
					  (PinyinTone)aKey.m_tone) )
		break;
	    else
	    sel = k;
	}
	aKey.m_tone = (PinyinTone)sel;
	//save the result
	out_keys[i] = aKey;
    }
}

//compute pinyin upper bound
//maybe replace by table lookup
inline void compute_upper_value(const PinyinCustomSettings &custom,
				PinyinKey in_keys[], 
				PinyinKey out_keys[],
				int phrase_length){
    PinyinKey aKey = in_keys[0];
    
    for ( int i = 0; i < phrase_length; i++){
	int k; int sel;
	aKey = in_keys[i];
	//deal with initial
	sel = aKey.m_initial;
	for( k = aKey.m_initial + 1; k <= PINYIN_LastInitial; k++){
	    if ( 0 != pinyin_compare_initial(custom, (PinyinInitial)k, (PinyinInitial)aKey.m_initial) )
		break;
	    else
		sel = k;
	}
	aKey.m_initial = (PinyinInitial)sel;
	//deal with final
	sel = aKey.m_final;
	for( k = aKey.m_final + 1; k <= PINYIN_LastFinal; k++){
	    if ( 0 != pinyin_compare_final(custom, (PinyinFinal)k, (PinyinFinal)aKey.m_final) )
		break;
	    else
		sel = k;
	}
	aKey.m_final = (PinyinFinal)sel;
	//deal with tone
	sel = aKey.m_tone;
	for( k = aKey.m_tone + 1; k <= PINYIN_LastTone; k++){
	    if ( 0 != pinyin_compare_tone(custom, (PinyinTone)k, (PinyinTone)aKey.m_tone) )
		break;
	    else
		sel = k;
	}
	aKey.m_tone = (PinyinTone)sel;
	//save the result
	out_keys[i] = aKey;
    }
}

template<int phrase_length>
struct PinyinIndexItem{
    phrase_token_t m_token;
    PinyinKey m_keys[phrase_length];
public:
    PinyinIndexItem<phrase_length>(PinyinKey * keys, phrase_token_t token){
	memmove(m_keys, keys, sizeof(PinyinKey) * phrase_length);
	m_token = token;
    }
};

/*
//just need less than  mode
//this method mainly used in pinyin lookup
template<int phrase_length>
class PhraseCompareWithAmbiguities
    : public std_lite::binary_function <const PinyinIndexItem <phrase_length>,
				   const PinyinIndexItem <phrase_length>, int>
{
    const PinyinCustomSettings & m_custom;
public:
    PhraseCompareWithAmbiguities<phrase_length>
	(const PinyinCustomSettings & custom):m_custom(custom){}

    int operator () (const PinyinIndexItem<phrase_length> &lhs,
                      const PinyinIndexItem<phrase_length> &rhs) const{
      PinyinKey * key_lhs = (PinyinKey *) lhs.m_keys;
      PinyinKey * key_rhs = (PinyinKey *) rhs.m_keys;
      return pinyin_compare_with_ambiguities(m_custom, 
					     key_lhs, key_rhs, phrase_length);
    }
};
*/

//for find the element in the phrase array
template<int phrase_length>
class PhraseExactCompare
  : public std_lite::binary_function <const PinyinIndexItem<phrase_length>
				 ,const PinyinIndexItem<phrase_length>, int>
{
public:
  int operator () (const PinyinIndexItem<phrase_length> &lhs,
		   const PinyinIndexItem<phrase_length> &rhs) const{
    PinyinKey * key_lhs = (PinyinKey *) lhs.m_keys;
    PinyinKey * key_rhs = (PinyinKey *) rhs.m_keys;
    
    return pinyin_exact_compare(key_lhs, key_rhs, phrase_length);
  }
};

/*
//for find the element in the phrase array
template<int phrase_length>
class PhraseExactCompareWithToken
  : public std_lite::binary_function <const PinyinIndexItem<phrase_length>
				 ,const PinyinIndexItem<phrase_length>, int>
{
public:
  int operator () (const PinyinIndexItem<phrase_length> &lhs,
		   const PinyinIndexItem<phrase_length> &rhs) const{
    PinyinKey * key_lhs = (PinyinKey *) lhs.m_keys;
    PinyinKey * key_rhs = (PinyinKey *) rhs.m_keys;
    
    phrase_token_t token_lhs = lhs.m_token;
    phrase_token_t token_rhs = rhs.m_token;
    
    int result = pinyin_exact_compare(key_lhs, key_rhs, phrase_length);
    if ( !result )
	return result;
    return pinyin_utility_sign(token_lhs - token_rhs);
  }
};
*/

template<int phrase_length>
class PhraseExactLessThan
    : public std_lite::binary_function <const PinyinIndexItem<phrase_length>
				   ,const PinyinIndexItem<phrase_length>,
    bool>
{
 private:
  PhraseExactCompare<phrase_length> m_compare;
 public:
  bool operator () (const PinyinIndexItem<phrase_length> &lhs,
		   const PinyinIndexItem<phrase_length> &rhs) const{
    return -1 == m_compare(lhs, rhs);
  }
};

/*
template<int phrase_length>
class PhraseExactLessThanWithToken
    : public std_lite::binary_function <const PinyinIndexItem<phrase_length>
				   ,const PinyinIndexItem<phrase_length>,
    bool>
{
 private:
  PhraseExactCompareWithToken<phrase_length> m_compare;
 public:
  bool operator () (const PinyinIndexItem<phrase_length> &lhs,
		   const PinyinIndexItem<phrase_length> &rhs) const{
    return -1 == m_compare(lhs, rhs);
  }
};
*/

};

using namespace novel;

#endif
