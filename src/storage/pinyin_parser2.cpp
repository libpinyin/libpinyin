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


#include "pinyin_parser2.h"
#include <ctype.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "stl_lite.h"
#include "pinyin_phrase3.h"
#include "pinyin_custom2.h"
#include "chewing_key.h"
#include "pinyin_parser_table.h"
#include "double_pinyin_table.h"
#include "phonetic_key_matrix.h"


namespace pinyin{

static bool check_pinyin_options(pinyin_option_t options, const pinyin_index_item_t * item) {
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


/* Pinyin Parsers */

/* internal information for pinyin parsers. */
struct parse_value_t{
    ChewingKey m_key;
    ChewingKeyRest m_key_rest;
    gint16 m_num_keys;
    gint16 m_parsed_len;
    gint16 m_distance;
    gint16 m_last_step;

    /* constructor */
public:
    parse_value_t(){
        m_num_keys = 0;
        m_parsed_len = 0;
        m_distance = 0;
        m_last_step = -1;
    }
};

const guint16 max_full_pinyin_length   = 7;  /* include tone. */

const guint16 max_double_pinyin_length = 3;  /* include tone. */

const guint16 max_chewing_length       = 4;  /* include tone. */

static bool compare_pinyin_less_than(const pinyin_index_item_t & lhs,
                                     const pinyin_index_item_t & rhs){
    return 0 > strcmp(lhs.m_pinyin_input, rhs.m_pinyin_input);
}

static inline bool search_pinyin_index(pinyin_option_t options,
                                       const char * pinyin,
                                       ChewingKey & key){
    pinyin_index_item_t item;
    memset(&item, 0, sizeof(item));
    item.m_pinyin_input = pinyin;

    std_lite::pair<const pinyin_index_item_t *,
                   const pinyin_index_item_t *> range;
    range = std_lite::equal_range
        (pinyin_index, pinyin_index + G_N_ELEMENTS(pinyin_index),
         item, compare_pinyin_less_than);

    guint16 range_len = range.second - range.first;
    assert(range_len <= 1);
    if (range_len == 1) {
        const pinyin_index_item_t * index = range.first;

        if (!check_pinyin_options(options, index))
            return false;

        key = content_table[index->m_table_index].m_chewing_key;
        assert(key.get_table_index() == index->m_table_index);
        return true;
    }

    return false;
}

static inline bool search_pinyin_index2(pinyin_option_t options,
                                        const pinyin_index_item_t * index,
                                        size_t len,
                                        const char * pinyin,
                                        ChewingKey & key,
                                        gint16 & distance){
    pinyin_index_item_t item;
    memset(&item, 0, sizeof(item));
    item.m_pinyin_input = pinyin;

    std_lite::pair<const pinyin_index_item_t *,
                   const pinyin_index_item_t *> range;
    range = std_lite::equal_range
        (index, index + len,
         item, compare_pinyin_less_than);

    guint16 range_len = range.second - range.first;
    assert(range_len <= 1);
    if (range_len == 1) {
        const pinyin_index_item_t * index = range.first;

        if (!check_pinyin_options(options, index))
            return false;

        key = content_table[index->m_table_index].m_chewing_key;
        distance = index->m_distance;
        assert(key.get_table_index() == index->m_table_index);
        return true;
    }

    return false;
}


/* Full Pinyin Parser */
FullPinyinParser2::FullPinyinParser2 (){
    m_pinyin_index = NULL; m_pinyin_index_len = 0;
    m_parse_steps = g_array_new(TRUE, FALSE, sizeof(parse_value_t));

    set_scheme(FULL_PINYIN_DEFAULT);
}

bool FullPinyinParser2::parse_one_key (pinyin_option_t options,
                                       ChewingKey & key,
                                       gint16 & distance,
                                       const char * pinyin, int len) const {
    /* "'" are not accepted in parse_one_key. */
    gchar * input = g_strndup(pinyin, len);
    assert(NULL == strchr(input, '\''));

    guint16 tone = CHEWING_ZERO_TONE; guint16 tone_pos = 0;
    guint16 parsed_len = len;
    key = ChewingKey();

    if (options & USE_TONE) {
        /* find the tone in the last character. */
        char chr = input[parsed_len - 1];
        if ( '0' < chr && chr <= '5' ) {
            tone = chr - '0';
            parsed_len --;
            tone_pos = parsed_len;
        }

        /* check the force tone option. */
        if (options & FORCE_TONE && CHEWING_ZERO_TONE == tone) {
            g_free(input);
            return false;
        }
    }

    /* parse pinyin core staff here. */

    /* Note: optimize here? */
    input[parsed_len] = '\0';
    if (!search_pinyin_index2(options, m_pinyin_index, m_pinyin_index_len,
                              input, key, distance)) {
        g_free(input);
        return false;
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

    g_free(input);
    return parsed_len == len;
}


int FullPinyinParser2::parse (pinyin_option_t options, ChewingKeyVector & keys,
                              ChewingKeyRestVector & key_rests,
                              const char *str, int len) const {
    int i;
    /* clear arrays. */
    g_array_set_size(keys, 0);
    g_array_set_size(key_rests, 0);

    /* init m_parse_steps, and prepare dynamic programming. */
    int step_len = len + 1;
    g_array_set_size(m_parse_steps, 0);
    parse_value_t value;
    for (i = 0; i < step_len; ++i) {
        g_array_append_val(m_parse_steps, value);
    }

    size_t next_sep = 0;
    gchar * input = g_strndup(str, len);
    parse_value_t * curstep = NULL, * nextstep = NULL;

    for (i = 0; i < len; ++i) {
        if (input[i] == '\'') {
            curstep = &g_array_index(m_parse_steps, parse_value_t, i);
            nextstep = &g_array_index(m_parse_steps, parse_value_t, i + 1);

            /* propagate current step into next step. */
            nextstep->m_key = ChewingKey();
            nextstep->m_key_rest = ChewingKeyRest();
            nextstep->m_num_keys = curstep->m_num_keys;
            nextstep->m_parsed_len = curstep->m_parsed_len + 1;
            nextstep->m_distance = curstep->m_distance;
            nextstep->m_last_step = i;
            next_sep = 0;
            continue;
        }

        /* forward to next "'" */
        if ( 0 == next_sep ) {
            int k;
            for (k = i;  k < len; ++k) {
                if (input[k] == '\'')
                    break;
            }
            next_sep = k;
        }

        /* dynamic programming here. */
        /* for (size_t m = i; m < next_sep; ++m) */
        {
            size_t m = i;
            curstep = &g_array_index(m_parse_steps, parse_value_t, m);
            size_t try_len = std_lite::min
                (m + max_full_pinyin_length, next_sep);
            for (size_t n = m + 1; n < try_len + 1; ++n) {
                nextstep = &g_array_index(m_parse_steps, parse_value_t, n);

                /* gen next step */
                gint16 distance = 0;
                const char * onepinyin = input + m;
                gint16 onepinyinlen = n - m;
                value = parse_value_t();

                ChewingKey key; ChewingKeyRest rest;
                bool parsed = parse_one_key
                    (options, key, distance, onepinyin, onepinyinlen);
                rest.m_raw_begin = m; rest.m_raw_end = n;
                if (!parsed)
                    continue;

                //printf("onepinyin:%s len:%d\n", onepinyin, onepinyinlen);

                value.m_key = key; value.m_key_rest = rest;
                value.m_num_keys = curstep->m_num_keys + 1;
                value.m_parsed_len = curstep->m_parsed_len + onepinyinlen;
                value.m_distance = curstep->m_distance + distance;
                value.m_last_step = m;

                /* save next step */
                /* no previous result */
                if (-1 == nextstep->m_last_step)
                    *nextstep = value;
                /* prefer the longest pinyin */
                if (value.m_parsed_len > nextstep->m_parsed_len)
                    *nextstep = value;
                /* prefer the shortest keys with the same pinyin length */
                if (value.m_parsed_len == nextstep->m_parsed_len &&
                    value.m_num_keys < nextstep->m_num_keys)
                    *nextstep = value;

                /* handle with the same pinyin length and the number of keys */
                if (value.m_parsed_len == nextstep->m_parsed_len &&
                    value.m_num_keys == nextstep->m_num_keys &&
                    value.m_distance < nextstep->m_distance)
                    *nextstep = value;

#if 0
                /* prefer the 'a' at the end of clause,
                 * ex: "zheyanga$" -> "zhe'yang'a$".
                 */
                if (value.m_parsed_len == len &&
                    (value.m_parsed_len == nextstep->m_parsed_len &&
                     value.m_num_keys == nextstep->m_num_keys &&
                     value.m_distance == nextstep->m_distance) &&
                    (nextstep->m_key.m_initial != CHEWING_ZERO_INITIAL &&
                     nextstep->m_key.m_middle == CHEWING_ZERO_MIDDLE &&
                     nextstep->m_key.m_final == CHEWING_A) &&
                    (value.m_key.m_initial == CHEWING_ZERO_INITIAL &&
                     value.m_key.m_middle == CHEWING_ZERO_MIDDLE &&
                     value.m_key.m_final == CHEWING_A))
                    *nextstep = value;
#endif
            }
        }
    }

    /* final step for back tracing. */
    gint16 parsed_len = final_step(step_len, keys, key_rests);

#if 0
    /* post processing for re-split table. */
    if (options & USE_RESPLIT_TABLE) {
        post_process2(options, keys, key_rests, str, len);
    }
#endif

    g_free(input);
    return parsed_len;
}

int FullPinyinParser2::final_step(size_t step_len, ChewingKeyVector & keys,
                                  ChewingKeyRestVector & key_rests) const{
    int i;
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
        if (0 != curstep->m_key.get_table_index()) {
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

bool FullPinyinParser2::set_scheme(FullPinyinScheme scheme){
    switch(scheme){
    case FULL_PINYIN_HANYU:
        m_pinyin_index = pinyin_index;
        m_pinyin_index_len = G_N_ELEMENTS(pinyin_index);
        break;
    case FULL_PINYIN_LUOMA:
        m_pinyin_index = luoma_pinyin_index;
        m_pinyin_index_len = G_N_ELEMENTS(luoma_pinyin_index);
        break;
    case FULL_PINYIN_SECONDARY_ZHUYIN:
        m_pinyin_index = secondary_zhuyin_index;
        m_pinyin_index_len = G_N_ELEMENTS(secondary_zhuyin_index);
        break;
    default:
        assert(false);
    }
    return true;
}

#define IS_KEY(x)   (('a' <= x && x <= 'z') || x == ';')

bool DoublePinyinParser2::parse_one_key(pinyin_option_t options,
                                        ChewingKey & key,
                                        gint16 & distance,
                                        const char *str, int len) const {
    options &= ~(PINYIN_CORRECT_ALL|PINYIN_AMB_ALL);

    /* force tone requires at least 3 characters. */
    if (options & FORCE_TONE && 3 != len)
        return false;

    if (1 == len) {
        if (!(options & PINYIN_INCOMPLETE))
            return false;

        char ch = str[0];
        if (!IS_KEY(ch))
            return false;

        int charid = ch == ';' ? 26 : ch - 'a';
        const char * sheng = m_shengmu_table[charid].m_shengmu;
        if (NULL == sheng || strcmp(sheng, "'") == 0)
            return false;

        if (search_pinyin_index(options, sheng, key))
            return true;
        else
            return false;
    }

    ChewingTone tone = CHEWING_ZERO_TONE;
    options &= ~(PINYIN_INCOMPLETE|ZHUYIN_INCOMPLETE);
    options |= PINYIN_CORRECT_UE_VE | PINYIN_CORRECT_V_U;

    /* parse tone */
    if (3 == len) {
        if (!(options & USE_TONE))
            return false;
        char ch = str[2];
        if (!('0' < ch && ch <= '5'))
            return false;
        tone = (ChewingTone) (ch - '0');

        /* check the force tone option. */
        if (options & FORCE_TONE && CHEWING_ZERO_TONE == tone)
            return false;
    }

    if (2 == len || 3 == len) {
        /* parse shengmu here. */
        char ch = str[0];
        if (!IS_KEY(ch))
            return false;

        int charid = ch == ';' ? 26 : ch - 'a';
        const char * sheng = m_shengmu_table[charid].m_shengmu;
        gchar * pinyin = NULL;
        if (NULL == sheng)
            goto fallback;
        if (0 == strcmp(sheng, "'"))
            sheng = "";

        /* parse yunmu here. */
        ch = str[1];
        if (!IS_KEY(ch))
            return false;

        do {

            charid = ch == ';' ? 26 : ch - 'a';
            /* first yunmu */
            const char * yun = m_yunmu_table[charid].m_yunmus[0];
            if (NULL == yun)
                break;

            pinyin = g_strdup_printf("%s%s", sheng, yun);
            if (search_pinyin_index(options, pinyin, key)) {
                key.m_tone = tone;
                g_free(pinyin);
                return true;
            }
            g_free(pinyin);

            /* second yunmu */
            yun = m_yunmu_table[charid].m_yunmus[1];
            if (NULL == yun)
                break;

            pinyin = g_strdup_printf("%s%s", sheng, yun);
            if (search_pinyin_index(options, pinyin, key)) {
                key.m_tone = tone;
                g_free(pinyin);
                return true;
            }
            g_free(pinyin);
        } while(0);

    fallback:
        /* support fallback table for double pinyin. */
        if (m_fallback_table) {
            gchar * input = g_strndup(str, 2);

            const char * yunmu = NULL;
            const double_pinyin_scheme_fallback_item_t * item =
                m_fallback_table;

            /* as the fallback table is short, just iterate the table. */
            while(NULL != item->m_input) {
                if (0 == strcmp(item->m_input, input))
                    yunmu = item->m_yunmu;
                item++;
            }

            if (NULL != yunmu && search_pinyin_index(options, yunmu, key)) {
                key.m_tone = tone;
                g_free(input);
                return true;
            }
            g_free(input);
            return false;
        }
    }

    return false;
}


/* only 'a'-'z' and ';' are accepted here. */
int DoublePinyinParser2::parse(pinyin_option_t options, ChewingKeyVector & keys,
                               ChewingKeyRestVector & key_rests,
                               const char *str, int len) const {
    g_array_set_size(keys, 0);
    g_array_set_size(key_rests, 0);

    int maximum_len = 0; int i;
    /* probe the longest possible double pinyin string. */
    for (i = 0; i < len; ++i) {
        const char ch = str[i];
        if (!(IS_KEY(ch) || ('0' < ch && ch <= '5')))
            break;
    }
    maximum_len = i;

    /* maximum forward match for double pinyin. */
    int parsed_len = 0;
    while (parsed_len < maximum_len) {
        const char * cur_str = str + parsed_len;
        i = std_lite::min(maximum_len - parsed_len,
                          (int)max_double_pinyin_length);

        gint16 distance = 0;
        ChewingKey key; ChewingKeyRest key_rest;
        for (; i > 0; --i) {
            bool success = parse_one_key(options, key, distance, cur_str, i);
            if (success)
                break;
        }

        if (0 == i)        /* no more possible double pinyins. */
            break;

        key_rest.m_raw_begin = parsed_len; key_rest.m_raw_end = parsed_len + i;
        parsed_len += i;

        /* save the pinyin */
        g_array_append_val(keys, key);
        g_array_append_val(key_rests, key_rest);
    }

    return parsed_len;
}

#undef IS_KEY

bool DoublePinyinParser2::set_scheme(DoublePinyinScheme scheme) {
    /* most double pinyin schemes doesn't use fallback table. */
    m_fallback_table = NULL;

    switch (scheme) {
    case DOUBLE_PINYIN_ZRM:
        m_shengmu_table  = double_pinyin_zrm_sheng;
        m_yunmu_table    = double_pinyin_zrm_yun;
        m_fallback_table = double_pinyin_zrm_fallback;
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
        m_shengmu_table  = double_pinyin_pyjj_sheng;
        m_yunmu_table    = double_pinyin_pyjj_yun;
        m_fallback_table = double_pinyin_pyjj_fallback;
        return true;
    case DOUBLE_PINYIN_XHE:
        m_shengmu_table  = double_pinyin_xhe_sheng;
        m_yunmu_table    = double_pinyin_xhe_yun;
        m_fallback_table = double_pinyin_xhe_fallback;
        return true;
    case DOUBLE_PINYIN_CUSTOMIZED:
        assert(FALSE);
    };

    return false; /* no such scheme. */
}


PinyinDirectParser2::PinyinDirectParser2 (){
    m_pinyin_index = pinyin_index;
    m_pinyin_index_len = G_N_ELEMENTS(pinyin_index);
}

bool PinyinDirectParser2::parse_one_key(pinyin_option_t options,
                                        ChewingKey & key,
                                        gint16 & distance,
                                        const char *str, int len) const {
    /* "'" are not accepted in parse_one_key. */
    gchar * input = g_strndup(str, len);
    assert(NULL == strchr(input, '\''));

    guint16 tone = CHEWING_ZERO_TONE; guint16 tone_pos = 0;
    guint16 parsed_len = len;
    key = ChewingKey();

    if (options & USE_TONE) {
        /* find the tone in the last character. */
        char chr = input[parsed_len - 1];
        if ( '0' < chr && chr <= '5' ) {
            tone = chr - '0';
            parsed_len --;
            tone_pos = parsed_len;
        }

        /* check the force tone option. */
        if (options & FORCE_TONE && CHEWING_ZERO_TONE == tone) {
            g_free(input);
            return false;
        }
    }

    /* parse pinyin core staff here. */

    /* Note: optimize here? */
    input[parsed_len] = '\0';
    if (!search_pinyin_index2(options, m_pinyin_index, m_pinyin_index_len,
                              input, key, distance)) {
        g_free(input);
        return false;
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

    g_free(input);
    return parsed_len == len;
}

int PinyinDirectParser2::parse(pinyin_option_t options,
                               ChewingKeyVector & keys,
                               ChewingKeyRestVector & key_rests,
                               const char *str, int len) const {
    g_array_set_size(keys, 0);
    g_array_set_size(key_rests, 0);

    int parsed_len = 0;
    int i = 0, cur = 0, next = 0;
    while (cur < len) {
        /* probe next position */
        for (i = cur; i < len; ++i) {
            if (' ' == str[i] || '\'' == str[i])
                break;
        }
        next = i;

        gint16 distance = 0;
        ChewingKey key; ChewingKeyRest key_rest;
        if (parse_one_key(options, key, distance, str + cur, next - cur)) {
            key_rest.m_raw_begin = cur; key_rest.m_raw_end = next;

            /* save the pinyin. */
            g_array_append_val(keys, key);
            g_array_append_val(key_rests, key_rest);
        } else {
            return parsed_len;
        }

        /* skip consecutive spaces. */
        for (i = next; i < len; ++i) {
            if (' ' != str[i] && '\'' != str[i])
                break;
        }

        cur = i;
        parsed_len = i;
    }

    return parsed_len;
}

}
