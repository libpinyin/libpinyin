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
#include <assert.h>
#include <stdio.h>
#include "pinyin_custom2.h"

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
        if (0 == keys->len)
            continue;

        size_t i = 0;
        for (i = 0; i < keys->len; ++i) {
            const ChewingKey key = g_array_index(keys, ChewingKey, i);
            const ChewingKeyRest key_rest = g_array_index(key_rests,
                                                          ChewingKeyRest, i);

#define MATCH(AMBIGUITY, ORIGIN, ANOTHER) do {                          \
                if (options & AMBIGUITY) {                              \
                    if (ORIGIN == key.m_initial) {                      \
                        ChewingKey newkey = key;                        \
                        newkey.m_initial = ANOTHER;                     \
                        if (0 != newkey.get_table_index())              \
                            matrix->append(index, newkey, key_rest);    \
                    }                                                   \
                }                                                       \
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

int search_matrix_recur(GArray * cached_keys,
                        FacadeChewingTable2 * table,
                        PhoneticKeyMatrix * matrix,
                        size_t start, size_t end,
                        PhraseIndexRanges ranges,
                        size_t & longest) {
    if (start > end)
        return SEARCH_NONE;

    /* only do chewing table search with 'start' and 'end'. */
    if (start == end) {
        /* exceed the maximum phrase length.  */
        if (cached_keys->len > MAX_PHRASE_LENGTH)
            return SEARCH_NONE;

        /* only "'" here. */
        if (0 == cached_keys->len)
            return SEARCH_CONTINUED;

#if 0
        printf("search table:%d\n", cached_keys->len);
#endif
        return table->search(cached_keys->len,
                             (ChewingKey *)cached_keys->data, ranges);
    }

    int result = SEARCH_NONE;

    const size_t size = matrix->get_column_size(start);
    /* assume pinyin parsers will filter invalid keys. */
    assert(size > 0);

    for (size_t i = 0; i < size; ++i) {
        ChewingKey key; ChewingKeyRest key_rest;
        matrix->get_item(start, i, key, key_rest);

        const size_t newstart = key_rest.m_raw_end;

        const ChewingKey zero_key;
        if (zero_key == key) {
            /* assume only one key here for "'" or the last key. */
            assert(1 == size);
            return search_matrix_recur(cached_keys, table, matrix,
                                       newstart, end, ranges, longest);
        }

        /* push value */
        g_array_append_val(cached_keys, key);
        longest = std_lite::max(longest, newstart);

        result |= search_matrix_recur(cached_keys, table, matrix,
                                      newstart, end, ranges, longest);

        /* pop value */
        g_array_set_size(cached_keys, cached_keys->len - 1);
    }

    return result;
}

int search_matrix(FacadeChewingTable2 * table,
                  PhoneticKeyMatrix * matrix,
                  size_t start, size_t end,
                  PhraseIndexRanges ranges) {
    assert(end < matrix->size());

    const size_t start_len = matrix->get_column_size(start);
    if (0 == start_len)
        return SEARCH_NONE;

    const size_t end_len = matrix->get_column_size(end);
    /* for empty column simply return SEARCH_CONTINUED. */
    if (0 == end_len)
        return SEARCH_CONTINUED;

    GArray * cached_keys = g_array_new(TRUE, TRUE, sizeof(ChewingKey));

    size_t longest = 0;
    int result = search_matrix_recur(cached_keys, table, matrix,
                                     start, end, ranges, longest);

    /* if any recur search return SEARCH_CONTINUED or longest > end,
       then return SEARCH_CONTINUED. */
    if (longest > end)
        result |= SEARCH_CONTINUED;

    g_array_free(cached_keys, TRUE);
    return result;
}

gfloat compute_pronunciation_possibility_recur(pinyin_option_t options,
                                               PhoneticKeyMatrix * matrix,
                                               size_t start, size_t end,
                                               GArray * cached_keys,
                                               PhraseItem & item){
    if (start > end)
        return 0.;

    const size_t phrase_length = item.get_phrase_length();
    if (phrase_length < cached_keys->len)
        return 0.;

    /* only do compute with 'start' and 'end'. */
    if (start == end) {
        if (phrase_length != cached_keys->len)
            return 0.;

        return item.get_pronunciation_possibility
            (options, (ChewingKey *) cached_keys->data);
    }

    gfloat result = 0.;

    const size_t size = matrix->get_column_size(start);
    /* assume pinyin parsers will filter invalid keys. */
    assert(size > 0);

    for (size_t i = 0; i < size; ++i) {
        ChewingKey key; ChewingKeyRest key_rest;
        matrix->get_item(start, i, key, key_rest);

        const size_t newstart = key_rest.m_raw_end;

        const ChewingKey zero_key;
        if (zero_key == key) {
            /* assume only one key here for "'" or the last key. */
            assert(1 == size);
            return compute_pronunciation_possibility_recur
                (options, matrix, newstart, end, cached_keys, item);
        }

        /* push value */
        g_array_append_val(cached_keys, key);

        result += compute_pronunciation_possibility_recur
            (options, matrix, newstart, end, cached_keys, item);

        /* pop value */
        g_array_set_size(cached_keys, cached_keys->len - 1);
    }

    return result;
}

gfloat compute_pronunciation_possibility(pinyin_option_t options,
                                         PhoneticKeyMatrix * matrix,
                                         size_t start, size_t end,
                                         GArray * cached_keys,
                                         PhraseItem & item){
    assert(end < matrix->size());

    assert(matrix->get_column_size(start) > 0);
    assert(matrix->get_column_size(end) > 0);

    return compute_pronunciation_possibility_recur
        (options, matrix, start, end, cached_keys, item);
}

bool increase_pronunciation_possibility_recur(pinyin_option_t options,
                                              PhoneticKeyMatrix * matrix,
                                              size_t start, size_t end,
                                              GArray * cached_keys,
                                              PhraseItem & item, gint32 delta) {
    if (start > end)
        return false;

    const size_t phrase_length = item.get_phrase_length();
    if (phrase_length < cached_keys->len)
        return false;

    /* only increase with 'start' and 'end'. */
    if (start == end) {
        if (phrase_length != cached_keys->len)
            return false;

        item.increase_pronunciation_possibility
            (options, (ChewingKey *) cached_keys->data, delta);
        return true;
    }

    bool result = false;

    const size_t size = matrix->get_column_size(start);
    /* assume pinyin parsers will filter invalid keys. */
    assert(size > 0);

    for (size_t i = 0; i < size; ++i) {
        ChewingKey key; ChewingKeyRest key_rest;
        matrix->get_item(start, i, key, key_rest);

        const size_t newstart = key_rest.m_raw_end;

        const ChewingKey zero_key;
        if (zero_key == key) {
            /* assume only one key here for "'" or the last key. */
            assert(1 == size);
            return increase_pronunciation_possibility_recur
                (options, matrix, newstart, end, cached_keys, item, delta);
        }

        /* push value */
        g_array_append_val(cached_keys, key);

        result = increase_pronunciation_possibility_recur
            (options, matrix, newstart, end, cached_keys, item, delta) || result;

        /* pop value */
        g_array_set_size(cached_keys, cached_keys->len - 1);
    }

    return result;
}

bool increase_pronunciation_possibility(pinyin_option_t options,
                                        PhoneticKeyMatrix * matrix,
                                        size_t start, size_t end,
                                        GArray * cached_keys,
                                        PhraseItem & item, gint32 delta) {
    assert(end < matrix->size());

    assert(matrix->get_column_size(start) > 0);
    assert(matrix->get_column_size(end) > 0);

    return increase_pronunciation_possibility_recur
        (options, matrix, start, end, cached_keys, item, delta);
}

};
