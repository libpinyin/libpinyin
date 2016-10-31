/* 
 *  libpinyin
 *  Library to deal with pinyin.
 *  
 *  Copyright (C) 2016 Peng Wu <alexepico@gmail.com>
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

#ifndef PINYIN_PHRASE3_H
#define PINYIN_PHRASE3_H

#include <assert.h>
#include "novel_types.h"
#include "chewing_key.h"

/* All compare function should be symmetric for the lhs and rhs operands.
   URL: http://en.cppreference.com/w/cpp/algorithm/equal_range . */

namespace pinyin{

inline int pinyin_exact_compare2(const ChewingKey * key_lhs,
                                 const ChewingKey * key_rhs,
                                 int phrase_length){
    int i;
    int result;

    /* compare initial */
    for (i = 0; i < phrase_length; ++i) {
        result = key_lhs[i].m_initial - key_rhs[i].m_initial;
        if (0 != result)
            return result;
    }

    /* compare middle and final */
    for (i = 0; i < phrase_length; ++i) {
        result = key_lhs[i].m_middle - key_rhs[i].m_middle;
        if (0 != result)
            return result;
        result = key_lhs[i].m_final - key_rhs[i].m_final;
        if (0 != result)
            return result;
    }

    /* compare tone */
    for (i = 0; i < phrase_length; ++i) {
        result = key_lhs[i].m_tone - key_rhs[i].m_tone;
        if (0 != result)
            return result;
    }

    return 0;
}

/* compare pinyins with chewing internal representations,
   fuzzy pinyin is not handled here. */
inline int pinyin_compare_initial3(ChewingInitial lhs,
                                   ChewingInitial rhs) {
    return (lhs - rhs);
}

inline int pinyin_compare_middle_and_final3(ChewingMiddle middle_lhs,
                                            ChewingMiddle middle_rhs,
                                            ChewingFinal final_lhs,
                                            ChewingFinal final_rhs) {
    if (middle_lhs == middle_rhs && final_lhs == final_rhs)
        return 0;

    /* handle in-complete pinyin here. */
    if (middle_lhs == CHEWING_ZERO_MIDDLE &&
        final_lhs == CHEWING_ZERO_FINAL)
        return 0;
    if (middle_rhs == CHEWING_ZERO_MIDDLE &&
        final_rhs == CHEWING_ZERO_FINAL)
        return 0;

    /* compare chewing middle first. */
    int middle_diff = middle_lhs - middle_rhs;
    if (middle_diff)
        return middle_diff;

    return (final_lhs - final_rhs);
}

inline int pinyin_compare_tone3(ChewingTone lhs,
                                ChewingTone rhs) {
    if (lhs == rhs)
        return 0;
    if (lhs == CHEWING_ZERO_TONE)
        return 0;
    if (rhs == CHEWING_ZERO_TONE)
        return 0;
    return (lhs - rhs);
}

/* compare with incomplete pinyin and zero tone. */
inline int pinyin_compare_with_tones(const ChewingKey * key_lhs,
                                     const ChewingKey * key_rhs,
                                     int phrase_length){
    int i;
    int result;

    /* compare initial */
    for (i = 0; i < phrase_length; ++i) {
        result = pinyin_compare_initial3
            ((ChewingInitial)key_lhs[i].m_initial,
             (ChewingInitial)key_rhs[i].m_initial);
        if (0 != result)
            return result;
    }

    /* compare middle and final */
    for (i = 0; i < phrase_length; ++i) {
        result = pinyin_compare_middle_and_final3
            ((ChewingMiddle)key_lhs[i].m_middle,
             (ChewingMiddle)key_rhs[i].m_middle,
             (ChewingFinal) key_lhs[i].m_final,
             (ChewingFinal) key_rhs[i].m_final);
        if (0 != result)
            return result;
    }

    /* compare tone */
    for (i = 0; i < phrase_length; ++i) {
        result = pinyin_compare_tone3
            ((ChewingTone)key_lhs[i].m_tone,
             (ChewingTone)key_rhs[i].m_tone);
        if (0 != result)
            return result;
    }

    return 0;
}

inline bool contains_incomplete_pinyin(const ChewingKey * keys,
                                       int phrase_length) {
    for (int i = 0; i < phrase_length; ++i) {
        const ChewingKey key = keys[i];
        if (CHEWING_ZERO_MIDDLE == key.m_middle &&
            CHEWING_ZERO_FINAL == key.m_final) {
            assert(CHEWING_ZERO_TONE == key.m_tone);
            return true;
        }
    }

    return false;
}

inline void compute_chewing_index(const ChewingKey * in_keys,
                                  ChewingKey * out_keys,
                                  int phrase_length) {
    for (int i = 0; i < phrase_length; ++i) {
        ChewingKey key = in_keys[i];
        key.m_tone = CHEWING_ZERO_TONE;
        out_keys[i] = key;
    }
}

inline void compute_incomplete_chewing_index(const ChewingKey * in_keys,
                                             ChewingKey * out_keys,
                                             int phrase_length) {
    for (int i = 0; i < phrase_length; ++i) {
        ChewingKey key;
        key.m_initial = in_keys[i].m_initial;
        out_keys[i] = key;
    }
}

template<size_t phrase_length>
struct PinyinIndexItem2{
    phrase_token_t m_token;
    ChewingKey m_keys[phrase_length];

public:
    PinyinIndexItem2<phrase_length> () {
        memset(m_keys, 0, sizeof(ChewingKey) * phrase_length);
        m_token = null_token;
    }

    PinyinIndexItem2<phrase_length> (const ChewingKey * keys,
                                     phrase_token_t token) {
        memmove(m_keys, keys, sizeof(ChewingKey) * phrase_length);
        m_token = token;
    }
};


/* for find the element in the phrase array */
template<size_t phrase_length>
inline int phrase_exact_compare2(const PinyinIndexItem2<phrase_length> &lhs,
                                 const PinyinIndexItem2<phrase_length> &rhs)
{
    ChewingKey * keys_lhs = (ChewingKey *) lhs.m_keys;
    ChewingKey * keys_rhs = (ChewingKey *) rhs.m_keys;
    return pinyin_exact_compare2(keys_lhs, keys_rhs, phrase_length);
}

template<size_t phrase_length>
inline bool phrase_exact_less_than2(const PinyinIndexItem2<phrase_length> &lhs,
                                    const PinyinIndexItem2<phrase_length> &rhs)
{
    return 0 > phrase_exact_compare2<phrase_length>(lhs, rhs);
}

template<size_t phrase_length>
inline int phrase_compare_with_tones(const PinyinIndexItem2<phrase_length> &lhs,
                                     const PinyinIndexItem2<phrase_length> &rhs)
{
    ChewingKey * keys_lhs = (ChewingKey *) lhs.m_keys;
    ChewingKey * keys_rhs = (ChewingKey *) rhs.m_keys;
    return pinyin_compare_with_tones(keys_lhs, keys_rhs, phrase_length);
}

template<size_t phrase_length>
inline int phrase_less_than_with_tones(const PinyinIndexItem2<phrase_length> &lhs,
                                       const PinyinIndexItem2<phrase_length> &rhs)
{
    return 0 > phrase_compare_with_tones<phrase_length>(lhs, rhs);
}

};

#endif
