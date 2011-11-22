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


#include <ctype.h>
#include <assert.h>
#include <string.h>
#include "stl_lite.h"
#include "pinyin_parser2.h"
#include "pinyin_phrase2.h"
#include "pinyin_custom2.h"
#include "chewing_key.h"
#include "pinyin_parser_table.h"
#include "double_pinyin_table.h"
#include "chewing_table.h"


using namespace pinyin;

static bool check_pinyin_options(guint32 options, const pinyin_index_item_t * item) {
    guint32 flags = item->m_flags;
    assert (flags & IS_PINYIN);

    /* handle incomplete pinyin. */
    if (flags & PINYIN_INCOMPLETE) {
        if (!(options & PINYIN_INCOMPLETE))
            return false;
    }

    /* handle correct pinyin, currently only one flag per item. */
    flags &= PINYIN_CORRECT_ALL;
    options &= PINYIN_CORRECT_ALL;

    if (flags) {
        if ((flags & options) != flags)
            return false;
    }

    return true;
}

static bool check_chewing_options(guint32 options, const chewing_index_item_t * item) {
    guint32 flags = item->m_flags;
    assert (flags & IS_CHEWING);

    /* handle incomplete chewing. */
    if (flags & CHEWING_INCOMPLETE) {
        if (!(options & CHEWING_INCOMPLETE))
            return false;
    }

    return true;
}


/* methods for Chewing Keys to access pinyin parser table. */
const char * ChewingKeyRest::get_pinyin_string(){
    if (m_table_index == 0)
        return NULL;

    /* check end boundary. */
    assert(m_table_index < G_N_ELEMENTS(content_table));
    return content_table[m_table_index].m_pinyin_str;
}

const char * ChewingKeyRest::get_chewing_string(){
    if (m_table_index == 0)
        return NULL;

    /* check end boundary. */
    assert(m_table_index < G_N_ELEMENTS(content_table));
    return content_table[m_table_index].m_chewing_str;
}


/* Pinyin Parsers */

/* internal information for pinyin parsers. */
struct parse_value_t{
    ChewingKey m_key;
    ChewingKeyRest m_key_rest;
    gint16 m_num_keys;
    gint16 m_parsed_len;
    gint16 m_last_step;

    /* constructor */
public:
    parse_value_t(){
        m_num_keys = 0;
        m_parsed_len = 0;
        m_last_step = -1;
    }
};

/* Full Pinyin Parser */
FullPinyinParser2::FullPinyinParser2 (){
    m_parse_steps = g_array_new(TRUE, FALSE, sizeof(parse_value_t));
}

const guint16 max_full_pinyin_length = 7;  /* include tone. */

static bool compare_less_than(const pinyin_index_item_t & lhs,
                              const pinyin_index_item_t & rhs){
    return 0 > strcmp(lhs.m_pinyin_input, rhs.m_pinyin_input);
}

bool FullPinyinParser2::parse_one_key (guint32 options, ChewingKey & key,
                                       ChewingKeyRest & key_rest,
                                       const char * pinyin, int len) const {
    /* "'" are not accepted in parse_one_key. */
    assert(NULL == strchr(pinyin, '\''));
    gchar * input = g_strndup(pinyin, len);

    guint16 tone = CHEWING_ZERO_TONE; guint16 tone_pos = 0;
    guint16 parsed_len = len;
    key = ChewingKey(); key_rest = ChewingKeyRest();

    if (options & USE_TONE) {
        /* find the tone in the last character. */
        char chr = input[parsed_len - 1];
        if ( '0' < chr && chr <= '5' ) {
            tone = chr - '0';
            parsed_len --;
            tone_pos = parsed_len;
        }
    }

    /* parse pinyin core staff here. */
    pinyin_index_item_t item;
    memset(&item, 0, sizeof(item));

    /* Note: optimize here? */
    for (; parsed_len >= len - 1; --parsed_len) {
        input[parsed_len] = '\0';
        item.m_pinyin_input = input;
        std_lite::pair<const pinyin_index_item_t *,
                       const pinyin_index_item_t *> range;
        range = std_lite::equal_range
            (pinyin_index, pinyin_index + G_N_ELEMENTS(pinyin_index),
             item, compare_less_than);

        guint16 range_len = range.second - range.first;
        assert (range_len <= 1);
        if ( range_len == 1 ) {
            const pinyin_index_item_t * index = range.first;

            if (!check_pinyin_options(options, index))
                continue;

            key_rest.m_table_index = index->m_table_index;
            key = content_table[key_rest.m_table_index].m_chewing_key;
            break;
        }
    }

    if (options & USE_TONE) {
        /* post processing tone. */
        if ( parsed_len == tone_pos ) {
            if (tone != CHEWING_ZERO_TONE) {
                key.m_tone = tone;
                parsed_len ++;
            }
        }
    }

    key_rest.m_raw_begin = 0; key_rest.m_raw_end = parsed_len;
    g_free(input);
    return parsed_len == len;
}


int FullPinyinParser2::parse (guint32 options, ChewingKeyVector & keys,
                              ChewingKeyRestVector & key_rests,
                              const char *str, int len) const {
    size_t i;
    /* clear arrays. */
    g_array_set_size(keys, 0);
    g_array_set_size(key_rests, 0);

    /* init m_parse_steps, and prepare dynamic programming. */
    size_t step_len = len + 1;
    g_array_set_size(m_parse_steps, 0);
    parse_value_t value;
    for (i = 0; i < step_len; ++i) {
        g_array_append_val(m_parse_steps, value);
    }

    size_t str_len = len; size_t next_sep = 0;
    gchar * input = g_strndup(str, len);
    parse_value_t * curstep = NULL, * nextstep = NULL;

    for (i = 0; i < len; ) {
        if (input[i] == '\'') {
            curstep = &g_array_index(m_parse_steps, parse_value_t, i);
            nextstep = &g_array_index(m_parse_steps, parse_value_t, i + 1);

            /* propagate current step into next step. */
            nextstep->m_key = ChewingKey();
            nextstep->m_key_rest = ChewingKeyRest();
            nextstep->m_num_keys = curstep->m_num_keys;
            nextstep->m_parsed_len = curstep->m_parsed_len + 1;
            nextstep->m_last_step = i;
            next_sep = 0;
            continue;
        }

        /* forward to next "'" */
        if ( 0 == next_sep ) {
            size_t k;
            for (k = i;  k < len; ++k) {
                if (input[k] == '\'')
                    break;
            }
            next_sep = k;
            i = next_sep;
        }

        /* dynamic programming here. */
        for (size_t m = i; m < next_sep; ++m) {
            curstep = &g_array_index(m_parse_steps, parse_value_t, m);
            size_t try_len = std_lite::min
                (m + max_full_pinyin_length, next_sep);
            for (size_t n = m + 1; n < try_len + 1; ++n) {
                nextstep = &g_array_index(m_parse_steps, parse_value_t, n);

                /* gen next step */
                const char * onepinyin = input + m;
                gint16 onepinyinlen = n - m;
                value = parse_value_t();

                ChewingKey key; ChewingKeyRest rest;
                bool parsed = parse_one_key
                    (options, key, rest, onepinyin, onepinyinlen);
                rest.m_raw_begin = m; rest.m_raw_end = n;
                if (!parsed)
                    continue;
                value.m_key = key; value.m_key_rest = rest;
                value.m_num_keys = curstep->m_num_keys + 1;
                value.m_parsed_len = curstep->m_parsed_len + onepinyinlen;
                value.m_last_step = m;

                /* save next step */
                if (-1 == nextstep->m_last_step)
                    *nextstep = value;
                if (value.m_parsed_len > nextstep->m_parsed_len)
                    *nextstep = value;
                if (value.m_parsed_len == nextstep->m_parsed_len &&
                    value.m_num_keys < nextstep->m_num_keys)
                    *nextstep = value;
            }
        }
    }

    /* final step for back tracing. */
    gint16 parsed_len = final_step(step_len, keys, key_rests);

    /* post processing for re-split table. */
    if (options & USE_RESPLIT_TABLE) {
        post_process(options, keys, key_rests);
    }

    g_free(input);
    return parsed_len;
}

int FullPinyinParser2::final_step(size_t step_len, ChewingKeyVector & keys,
                                  ChewingKeyRestVector & key_rests) const{
    size_t i;
    gint16 parsed_len = 0;
    parse_value_t * curstep = NULL;

    /* find longest match, which starts from the beginning of input. */
    for (i = step_len - 1; i >= 0; --i) {
        curstep = &g_array_index(m_parse_steps, parse_value_t, i);
        if (i == curstep->m_parsed_len)
            break;
    }
    /* prepare saving. */
    parsed_len = curstep->m_parsed_len;
    gint16 num_keys = curstep->m_num_keys;
    g_array_set_size(keys, num_keys);
    g_array_set_size(key_rests, num_keys);

    /* save the match. */
    while (curstep->m_last_step != -1) {
        gint16 pos = curstep->m_num_keys - 1;

        /* skip "'" */
        if (0 != curstep->m_key_rest.m_table_index) {
            ChewingKey * key = &g_array_index(keys, ChewingKey, pos);
            ChewingKeyRest * rest = &g_array_index
                (key_rests, ChewingKeyRest, pos);
            *key = curstep->m_key; *rest = curstep->m_key_rest;
        }

        /* back ward */
        curstep = &g_array_index(m_parse_steps, parse_value_t,
                                 curstep->m_last_step);
    }
    return parsed_len;
}


bool FullPinyinParser2::post_process(guint32 options,
                                     ChewingKeyVector & keys,
                                     ChewingKeyRestVector & key_rests) const {
    size_t i;
    assert(keys->len == key_rests->len);
    gint16 num_keys = keys->len;

    ChewingKey * cur_key = NULL, * next_key = NULL;
    ChewingKeyRest * cur_rest = NULL, * next_rest = NULL;
    guint16 cur_tone = CHEWING_ZERO_TONE, next_tone = CHEWING_ZERO_TONE;

    for (i = 0; i < num_keys - 1; ++i) {
        cur_rest = &g_array_index(key_rests, ChewingKeyRest, i);
        next_rest = &g_array_index(key_rests, ChewingKeyRest, i + 1);

        /* some "'" here */
        if (cur_rest->m_raw_end != next_rest->m_raw_begin)
            continue;

        cur_key = &g_array_index(keys, ChewingKey, i);
        next_key = &g_array_index(keys, ChewingKey, i + 1);

        if (options & USE_TONE) {
            cur_tone = cur_key->m_tone;
            next_tone = next_key->m_tone;
            cur_key->m_tone = next_key->m_tone = CHEWING_ZERO_TONE;
        }

        /* lookup re-split table */
        size_t k;
        const resplit_table_item_t * item = NULL;
        for (k = 0; k < G_N_ELEMENTS(resplit_table); ++k) {
            item = resplit_table + k;
            /* no ops */
            if (item->m_orig_freq >= item->m_new_freq)
                continue;

            /* use pinyin_exact_compare2 here. */
            if (0 == pinyin_exact_compare2(item->m_orig_keys,
                                           cur_key, 2))
                break;

        }

        /* find the match */
        if (k < G_N_ELEMENTS(resplit_table)) {
            /* do re-split */
            item = resplit_table + k;
            *cur_key = item->m_new_keys[0];
            *next_key = item->m_new_keys[1];
            /* assumes only moved one char in gen_all_resplit script. */
            cur_rest->m_raw_end --;
            next_rest->m_raw_begin --;
        }

        /* save back tones */
        if (options & USE_TONE) {
            cur_key->m_tone = cur_tone;
            next_key->m_tone = next_tone;
        }
    }

    return true;
}


bool DoublePinyinParser2::parse_one_key (guint32 options, ChewingKey & key,
                                         ChewingKeyRest & key_rest,
                                         const char *str, int len) const{
    if (1 == len) {
        if (!(options & PINYIN_INCOMPLETE))
            return false;
        assert(FALSE);
    }

    options &= ~(PINYIN_CORRECT_ALL|PINYIN_AMB_ALL);

    if (2 == len || 3 == len) {
        /* parse shengmu and yunmu here. */
        assert(FALSE);
    }

    if (3 == len) {
        if (!(options & USE_TONE))
            return false;
        assert(FALSE);
    }

    return false;
}


int DoublePinyinParser2::parse (guint32 options, ChewingKeyVector & keys,
                                ChewingKeyRestVector & key_rests,
                                const char *str, int len) const{
    assert(FALSE);
}

bool DoublePinyinParser2::set_scheme(DoublePinyinScheme scheme) {

    switch (scheme) {
    case DOUBLE_PINYIN_ZRM:
        m_shengmu_table = double_pinyin_zrm_sheng;
        m_yunmu_table   = double_pinyin_zrm_yun;
        return true;
    case DOUBLE_PINYIN_MS:
        m_shengmu_table = double_pinyin_mspy_sheng;
        m_yunmu_table   = double_pinyin_mspy_yun;
        return true;
    case DOUBLE_PINYIN_ZIGUANG:
        m_shengmu_table = double_pinyin_zgpy_sheng;
        m_yunmu_table   = double_pinyin_zgpy_yun;
        return true;
    case DOUBLE_PINYIN_ABC:
        m_shengmu_table = double_pinyin_abc_sheng;
        m_yunmu_table   = double_pinyin_abc_yun;
        return true;
    case DOUBLE_PINYIN_PYJJ:
        m_shengmu_table = double_pinyin_pyjj_sheng;
        m_yunmu_table   = double_pinyin_pyjj_yun;
        return true;
    case DOUBLE_PINYIN_XHE:
        m_shengmu_table = double_pinyin_xhe_sheng;
        m_yunmu_table   = double_pinyin_xhe_yun;
        return true;
    case DOUBLE_PINYIN_CUSTOMIZED:
        assert(FALSE);
    };

    return false; /* no such scheme. */
}
