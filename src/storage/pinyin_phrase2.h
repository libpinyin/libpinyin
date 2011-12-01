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

#ifndef PINYIN_PHRASE2_H
#define PINYIN_PHRASE2_H

#include "novel_types.h"
#include "chewing_key.h"
#include "pinyin_custom2.h"
#include "pinyin_parser2.h"

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


inline int pinyin_compare_with_ambiguities2(pinyin_option_t options,
                                            const ChewingKey * key_lhs,
                                            const ChewingKey * key_rhs,
                                            int phrase_length){
    int i;
    int result;

    /* compare initial */
    for (i = 0; i < phrase_length; ++i) {
        result = pinyin_compare_initial2
            (options,
             (ChewingInitial)key_lhs[i].m_initial,
             (ChewingInitial)key_rhs[i].m_initial);
        if (0 != result)
            return result;
    }

    /* compare middle and final */
    for (i = 0; i < phrase_length; ++i) {
        result = pinyin_compare_middle_and_final2
            (options,
             (ChewingMiddle)key_lhs[i].m_middle,
             (ChewingMiddle)key_rhs[i].m_middle,
             (ChewingFinal) key_lhs[i].m_final,
             (ChewingFinal) key_rhs[i].m_final);
        if (0 != result)
            return result;
    }

    /* compare tone */
    for (i = 0; i < phrase_length; ++i) {
        result = pinyin_compare_tone2
            (options,
             (ChewingTone)key_lhs[i].m_tone,
             (ChewingTone)key_rhs[i].m_tone);
        if (0 != result)
            return result;
    }

    return 0;
}

/* compute pinyin lower bound */
inline void compute_lower_value2(pinyin_option_t options,
                                  ChewingKey * in_keys,
                                  ChewingKey * out_keys,
                                  int phrase_length) {
    ChewingKey aKey;

    for (int i = 0; i < phrase_length; ++i) {
        int k; int sel;
        aKey = in_keys[i];

        /* compute lower initial */
        sel = aKey.m_initial;
        for (k = aKey.m_initial - 1; k >= CHEWING_ZERO_INITIAL; --k) {
            if (0 != pinyin_compare_initial2
                (options, (ChewingInitial)aKey.m_initial, (ChewingInitial)k))
                break;
            else
                sel = k;
        }
        aKey.m_initial = (ChewingInitial)sel;

        /* compute lower middle, skipped as no fuzzy pinyin here.
         * if needed in future, still use pinyin_compare_middle_and_final2
         * to check lower bound.
         */

        /* compute lower final */
        sel = aKey.m_final;
        for (k = aKey.m_final - 1; k >= CHEWING_ZERO_FINAL; --k) {
            if (0 != pinyin_compare_middle_and_final2
                (options,
                 (ChewingMiddle)aKey.m_middle, (ChewingMiddle) aKey.m_middle,
                 (ChewingFinal)aKey.m_final, (ChewingFinal)k))
                break;
            else
                sel = k;
        }
        aKey.m_final = (ChewingFinal)sel;

        /* compute lower tone */
        sel = aKey.m_tone;
        for (k = aKey.m_tone - 1; k >= CHEWING_ZERO_TONE; --k) {
            if (0 != pinyin_compare_tone2
                (options, (ChewingTone)aKey.m_tone, (ChewingTone)k))
                break;
            else
                sel = k;
        }
        aKey.m_tone = (ChewingTone)sel;

        /* save the result */
        out_keys[i] = aKey;
    }
}

/* compute pinyin upper bound */
inline void compute_upper_value2(pinyin_option_t options,
                                 ChewingKey * in_keys,
                                 ChewingKey * out_keys,
                                 int phrase_length) {
    ChewingKey aKey;

    for (int i = 0; i < phrase_length; ++i) {
        int k; int sel;
        aKey = in_keys[i];

        /* compute upper initial */
        sel = aKey.m_initial;
        for (k = aKey.m_initial + 1; k <= CHEWING_LAST_INITIAL; ++k) {
            if (0 != pinyin_compare_initial2
                (options, (ChewingInitial)aKey.m_initial, (ChewingInitial)k))
                break;
            else
                sel = k;
        }
        aKey.m_initial = (ChewingInitial)sel;

        /* compute upper middle, skipped as no fuzzy pinyin here.
         * if needed in future, still use pinyin_compare_middle_and_final2
         * to check upper bound.
         */

        /* compute upper final */
        sel = aKey.m_final;
        for (k = aKey.m_final + 1; k <= CHEWING_LAST_FINAL; ++k) {
            if (0 != pinyin_compare_middle_and_final2
                (options,
                 (ChewingMiddle)aKey.m_middle, (ChewingMiddle)aKey.m_middle,
                 (ChewingFinal)aKey.m_final, (ChewingFinal)k))
                break;
            else
                sel = k;
        }
        aKey.m_final = (ChewingFinal)sel;

        /* compute upper tone */
        sel = aKey.m_tone;
        for (k = aKey.m_tone + 1; k <= CHEWING_LAST_TONE; ++k) {
            if (0 != pinyin_compare_tone2
                (options, (ChewingTone)aKey.m_tone, (ChewingTone)k))
                break;
            else
                sel = k;
        }
        aKey.m_tone = (ChewingTone)sel;

        /* save the result */
        out_keys[i] = aKey;
    }
}


template<int phrase_length>
struct PinyinIndexItem2{
    phrase_token_t m_token;
    ChewingKey m_keys[phrase_length];
public:
    PinyinIndexItem2<phrase_length> (ChewingKey * keys, phrase_token_t token) {
        memmove(m_keys, keys, sizeof(ChewingKey) * phrase_length);
        m_token = token;
    }
};


/* for find the element in the phrase array */
template<int phrase_length>
inline int phrase_exact_compare2(const PinyinIndexItem2<phrase_length> &lhs,
                                 const PinyinIndexItem2<phrase_length> &rhs)
{
    ChewingKey * keys_lhs = (ChewingKey *) lhs.m_keys;
    ChewingKey * keys_rhs = (ChewingKey *) rhs.m_keys;
    return pinyin_exact_compare2(keys_lhs, keys_rhs, phrase_length);
}

template<int phrase_length>
inline bool phrase_exact_less_than2(const PinyinIndexItem2<phrase_length> &lhs,
                                    const PinyinIndexItem2<phrase_length> &rhs)
{
    return 0 > phrase_exact_compare2<phrase_length>(lhs, rhs);
}

};

#endif
