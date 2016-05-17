/* 
 *  libpinyin
 *  Library to deal with pinyin.
 *  
 *  Copyright (C) 2016 Peng Wu <alexepico@gmail.com>
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

#include "phonetic_key_matrix.h"
#include "pinyin_custom2.h"
#include <assert.h>
#include <stdio.h>

namespace pinyin{

bool fill_phonetic_key_matrix_from_chewing_keys(PhoneticKeyMatrix * matrix,
                                                ChewingKeyVector keys,
                                                ChewingKeyRestVector key_rests) {
    assert(keys->len == key_rests->len);

    const ChewingKey * key = NULL;
    const ChewingKeyRest * key_rest = NULL;

    /* last key rest. */
    key_rest = &g_array_index(key_rests, ChewingKeyRest, key_rests->len - 1);

    /* one extra slot for the last key. */
    size_t length = key_rest->m_raw_end + 1;
    matrix->set_size(length);

    /* fill keys and key rests. */
    size_t i;
    for (i = 0; i < keys->len; ++i) {
        key = &g_array_index(keys, ChewingKey, i);
        key_rest = &g_array_index(key_rests, ChewingKeyRest, i);
        matrix->append(key_rest->m_raw_begin, *key, *key_rest);
    }

    /* fill zero keys for "'". */
    ChewingKeyRest * next_key_rest = NULL;
    const ChewingKey zero_key;
    ChewingKeyRest zero_key_rest;
    for (i = 0; i < key_rests->len - 1; ++i) {
        key_rest = &g_array_index(key_rests, ChewingKeyRest, i);
        next_key_rest = &g_array_index(key_rests, ChewingKeyRest, i + 1);

        for (size_t fill = key_rest->m_raw_end;
             fill < next_key_rest->m_raw_begin; ++fill) {
            zero_key_rest.m_raw_begin = fill;
            zero_key_rest.m_raw_end = fill + 1;
            matrix->append(fill, zero_key, zero_key_rest);
        }
    }

    /* fill zero keys for the last key. */
    zero_key_rest.m_raw_begin = length - 1;
    zero_key_rest.m_raw_end = length;
    matrix->append(length - 1, zero_key, zero_key_rest);
    return true;
}


bool fuzzy_syllable_step(pinyin_option_t options,
                         PhoneticKeyMatrix * matrix) {
    size_t length = matrix->size();

    GArray * keys = g_array_new(TRUE, TRUE, sizeof(ChewingKey));
    GArray * key_rests = g_array_new(TRUE, TRUE, sizeof(ChewingKeyRest));

    for (size_t index = 0; index < length; ++index) {
        /* for pinyin initials. */
        matrix->get_items(index, keys, key_rests);
        assert(keys->len == key_rests->len);
        if (0 == keys->len)
            continue;

        size_t i = 0;
        for (i = 0; i < keys->len; ++i) {
            const ChewingKey key = g_array_index(keys, ChewingKey, i);
            const ChewingKeyRest key_rest = g_array_index(key_rests,
                                                          ChewingKeyRest, i);

#define MATCH(AMBIGUITY, ORIGIN, ANOTHER) do {                      \
                if (options & AMBIGUITY) {                          \
                    if (ORIGIN == key.m_initial) {                  \
                        ChewingKey newkey = key;                    \
                        newkey.m_initial = ANOTHER;                 \
                        matrix->append(index, newkey, key_rest);    \
                    }                                               \
                }                                                   \
            } while (0)


            MATCH(PINYIN_AMB_C_CH, CHEWING_C, CHEWING_CH);
            MATCH(PINYIN_AMB_C_CH, CHEWING_CH, CHEWING_C);
            MATCH(PINYIN_AMB_Z_ZH, CHEWING_Z, CHEWING_ZH);
            MATCH(PINYIN_AMB_Z_ZH, CHEWING_ZH, CHEWING_Z);
            MATCH(PINYIN_AMB_S_SH, CHEWING_S, CHEWING_SH);
            MATCH(PINYIN_AMB_S_SH, CHEWING_SH, CHEWING_S);
            MATCH(PINYIN_AMB_L_R, CHEWING_L, CHEWING_R);
            MATCH(PINYIN_AMB_L_R, CHEWING_R, CHEWING_L);
            MATCH(PINYIN_AMB_L_N, CHEWING_L, CHEWING_N);
            MATCH(PINYIN_AMB_L_N, CHEWING_N, CHEWING_L);
            MATCH(PINYIN_AMB_F_H, CHEWING_F, CHEWING_H);
            MATCH(PINYIN_AMB_F_H, CHEWING_H, CHEWING_F);
            MATCH(PINYIN_AMB_G_K, CHEWING_G, CHEWING_K);
            MATCH(PINYIN_AMB_G_K, CHEWING_K, CHEWING_G);

#undef MATCH

        }

        /* for pinyin finals. */
        matrix->get_items(index, keys, key_rests);
        assert(keys->len == key_rests->len);
        assert(0 != keys->len);

        for (i = 0; i < keys->len; ++i) {
            const ChewingKey key = g_array_index(keys, ChewingKey, i);
            const ChewingKeyRest key_rest = g_array_index(key_rests,
                                                          ChewingKeyRest, i);

#define MATCH(AMBIGUITY, ORIGIN, ANOTHER) do {                     \
                if (options & AMBIGUITY) {                         \
                    if (ORIGIN == key.m_final) {                   \
                        ChewingKey newkey = key;                   \
                        newkey.m_final = ANOTHER;                  \
                        matrix->append(index, newkey, key_rest);   \
                    }                                              \
                }                                                  \
            } while (0)


            MATCH(PINYIN_AMB_AN_ANG, CHEWING_AN, CHEWING_ANG);
            MATCH(PINYIN_AMB_AN_ANG, CHEWING_ANG, CHEWING_AN);
            MATCH(PINYIN_AMB_EN_ENG, CHEWING_EN, CHEWING_ENG);
            MATCH(PINYIN_AMB_EN_ENG, CHEWING_ENG, CHEWING_EN);
            MATCH(PINYIN_AMB_IN_ING, PINYIN_IN, PINYIN_ING);
            MATCH(PINYIN_AMB_IN_ING, PINYIN_ING, PINYIN_IN);

#undef MATCH

        }
    }

    g_array_free(keys, TRUE);
    g_array_free(key_rests, TRUE);
    return true;
}


bool dump_phonetic_key_matrix(PhoneticKeyMatrix * matrix) {
    size_t length = matrix->size();

    GArray * keys = g_array_new(TRUE, TRUE, sizeof(ChewingKey));
    GArray * key_rests = g_array_new(TRUE, TRUE, sizeof(ChewingKeyRest));

    for (size_t index = 0; index < length; ++index) {
        matrix->get_items(index, keys, key_rests);
        assert(keys->len == key_rests->len);
        if (0 == keys->len)
            continue;

        printf("Column:%ld:\n", index);

        for (size_t i = 0; i < keys->len; ++i) {
            ChewingKey key = g_array_index(keys, ChewingKey, i);
            ChewingKeyRest key_rest = g_array_index(key_rests,
                                                    ChewingKeyRest, i);

            gchar * pinyin = key.get_pinyin_string();
            printf("ChewingKey:%s\n", pinyin);
            printf("ChewingKeyRest:%hd\t%hd\n",
                   key_rest.m_raw_begin, key_rest.m_raw_end);
            g_free(pinyin);
        }
    }

    g_array_free(keys, TRUE);
    g_array_free(key_rests, TRUE);
    return true;
}

};
