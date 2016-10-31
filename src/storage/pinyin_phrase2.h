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

#ifndef PINYIN_PHRASE2_H
#define PINYIN_PHRASE2_H

#include "novel_types.h"
#include "chewing_key.h"
#include "pinyin_custom2.h"
#include "pinyin_parser2.h"

namespace pinyin{

/* compare pinyins with chewing internal representations. */
inline int pinyin_compare_initial2(pinyin_option_t options,
                                   ChewingInitial lhs,
                                   ChewingInitial rhs) {
    if (lhs == rhs)
        return 0;

    if ((options & PINYIN_AMB_C_CH) &&
        ((lhs == CHEWING_C && rhs == CHEWING_CH) ||
         (lhs == CHEWING_CH && rhs == CHEWING_C)))
        return 0;

    if ((options & PINYIN_AMB_S_SH) &&
        ((lhs == CHEWING_S && rhs == CHEWING_SH) ||
         (lhs == CHEWING_SH && rhs == CHEWING_S)))
        return 0;

    if ((options & PINYIN_AMB_Z_ZH) &&
        ((lhs == CHEWING_Z && rhs == CHEWING_ZH) ||
         (lhs == CHEWING_ZH && rhs == CHEWING_Z)))
        return 0;

    if ((options & PINYIN_AMB_F_H) &&
        ((lhs == CHEWING_F && rhs == CHEWING_H) ||
         (lhs == CHEWING_H && rhs == CHEWING_F)))
        return 0;

    if ((options & PINYIN_AMB_L_N) &&
        ((lhs == CHEWING_L && rhs == CHEWING_N) ||
         (lhs == CHEWING_N && rhs == CHEWING_L)))
        return 0;

    if ((options & PINYIN_AMB_L_R) &&
        ((lhs == CHEWING_L && rhs == CHEWING_R) ||
         (lhs == CHEWING_R && rhs == CHEWING_L)))
        return 0;

    if ((options & PINYIN_AMB_G_K) &&
        ((lhs == CHEWING_G && rhs == CHEWING_K) ||
         (lhs == CHEWING_K && rhs == CHEWING_G)))
        return 0;

    return (lhs - rhs);
}


inline int pinyin_compare_middle_and_final2(pinyin_option_t options,
                                            ChewingMiddle middle_lhs,
                                            ChewingMiddle middle_rhs,
                                            ChewingFinal final_lhs,
                                            ChewingFinal final_rhs) {
    if (middle_lhs == middle_rhs && final_lhs == final_rhs)
        return 0;

    /* both pinyin and chewing incomplete options will enable this. */
    if (options & (PINYIN_INCOMPLETE | ZHUYIN_INCOMPLETE)) {
        if (middle_lhs == CHEWING_ZERO_MIDDLE &&
            final_lhs == CHEWING_ZERO_FINAL)
            return 0;
        if (middle_rhs == CHEWING_ZERO_MIDDLE &&
            final_rhs == CHEWING_ZERO_FINAL)
            return 0;
    }

    /* compare chewing middle first. */
    int middle_diff = middle_lhs - middle_rhs;
    if (middle_diff)
        return middle_diff;

    if ((options & PINYIN_AMB_AN_ANG) &&
        ((final_lhs == CHEWING_AN && final_rhs == CHEWING_ANG) ||
         (final_lhs == CHEWING_ANG && final_rhs == CHEWING_AN)))
        return 0;

    if ((options & PINYIN_AMB_EN_ENG) &&
        ((final_lhs == CHEWING_EN && final_rhs == CHEWING_ENG) ||
         (final_lhs == CHEWING_ENG && final_rhs == CHEWING_EN)))
        return 0;

    if ((options & PINYIN_AMB_IN_ING) &&
        ((final_lhs == PINYIN_IN && final_rhs == PINYIN_ING) ||
         (final_lhs == PINYIN_ING && final_rhs == PINYIN_IN)))
        return 0;

    return (final_lhs - final_rhs);
}


inline int pinyin_compare_tone2(pinyin_option_t options,
                                ChewingTone lhs,
                                ChewingTone rhs) {
    if (lhs == rhs)
        return 0;
    if (lhs == CHEWING_ZERO_TONE)
        return 0;
    if (rhs == CHEWING_ZERO_TONE)
        return 0;
    return (lhs - rhs);
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
                                 const ChewingKey * in_keys,
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

        /* as chewing zero middle is the first item, and its value is zero,
         * no need to adjust it for incomplete pinyin.
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
                                 const ChewingKey * in_keys,
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

        /* adjust it for incomplete pinyin. */

        /* compute upper middle */
        sel = aKey.m_middle;
        for (k = aKey.m_middle + 1; k <= CHEWING_LAST_MIDDLE; ++k) {
            if (0 != pinyin_compare_middle_and_final2
                (options,
                 (ChewingMiddle)aKey.m_middle, (ChewingMiddle)k,
                 (ChewingFinal)aKey.m_final, (ChewingFinal)aKey.m_final))
                break;
            else
                sel = k;
        }
        aKey.m_middle = (ChewingMiddle)sel;

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


};

#endif
