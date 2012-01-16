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


#include "pinyin_parser2.h"
#include <ctype.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "stl_lite.h"
#include "pinyin_phrase2.h"
#include "pinyin_custom2.h"
#include "chewing_key.h"
#include "pinyin_parser_table.h"
#include "double_pinyin_table.h"
#include "chewing_table.h"


using namespace pinyin;

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

static bool check_chewing_options(pinyin_option_t options, const chewing_index_item_t * item) {
    guint32 flags = item->m_flags;
    assert (flags & IS_CHEWING);

    /* handle incomplete chewing. */
    if (flags & CHEWING_INCOMPLETE) {
        if (!(options & CHEWING_INCOMPLETE))
            return false;
    }

    return true;
}


gint ChewingKey::get_table_index() {
    assert(m_initial <  CHEWING_NUMBER_OF_INITIALS);
    assert(m_middle < CHEWING_NUMBER_OF_MIDDLES);
    assert(m_final < CHEWING_NUMBER_OF_FINALS);

    gint index = chewing_key_table[(m_initial * CHEWING_NUMBER_OF_MIDDLES + m_middle) * CHEWING_NUMBER_OF_FINALS + m_final];
    return index == -1 ? 0 : index;
}

gchar * ChewingKey::get_pinyin_string() {
    assert(m_tone < CHEWING_NUMBER_OF_TONES);
    gint index = get_table_index();
    assert(index < G_N_ELEMENTS(content_table));
    const content_table_item_t & item = content_table[index];

    if (CHEWING_ZERO_TONE == m_tone) {
        return g_strdup(item.m_pinyin_str);
    } else {
        return g_strdup_printf("%s%d", item.m_pinyin_str, m_tone);
    }
}

gchar * ChewingKey::get_chewing_string() {
    assert(m_tone < CHEWING_NUMBER_OF_TONES);
    gint index = get_table_index();
    assert(index < G_N_ELEMENTS(content_table));
    const content_table_item_t & item = content_table[index];

    if (CHEWING_ZERO_TONE == m_tone) {
        return g_strdup(item.m_chewing_str);
    } else {
        return g_strdup_printf("%s%s", item.m_chewing_str,
                               chewing_tone_table[m_tone]);
    }
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

static bool compare_chewing_less_than(const chewing_index_item_t & lhs,
                                      const chewing_index_item_t & rhs){
    return 0 > strcmp(lhs.m_chewing_input, rhs.m_chewing_input);
}

static inline bool search_chewing_index(pinyin_option_t options,
                                        const char * chewing,
                                        ChewingKey & key){
    chewing_index_item_t item;
    memset(&item, 0, sizeof(item));
    item.m_chewing_input = chewing;

    std_lite::pair<const chewing_index_item_t *,
                   const chewing_index_item_t *> range;
    range = std_lite::equal_range
        (chewing_index, chewing_index + G_N_ELEMENTS(chewing_index),
         item, compare_chewing_less_than);

    guint16 range_len = range.second - range.first;
    assert (range_len <= 1);

    if (range_len == 1) {
        const chewing_index_item_t * index = range.first;

        if (!check_chewing_options(options, index))
            return false;

        key = content_table[index->m_table_index].m_chewing_key;
        assert(key.get_table_index() == index->m_table_index);
        return true;
    }

    return false;
}

/* Full Pinyin Parser */
FullPinyinParser2::FullPinyinParser2 (){
    m_parse_steps = g_array_new(TRUE, FALSE, sizeof(parse_value_t));
}


bool FullPinyinParser2::parse_one_key (pinyin_option_t options,
                                       ChewingKey & key,
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
    }

    /* parse pinyin core staff here. */

    /* Note: optimize here? */
    input[parsed_len] = '\0';
    if (!search_pinyin_index(options, input, key)) {
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

#if 0
        /* Heuristic Method:
         *   do maximum forward match first. */
        for (size_t pos = i; pos < next_sep; ++pos) {
            curstep = &g_array_index(m_parse_steps, parse_value_t, pos);
            size_t try_len = std_lite::min
                (pos + max_full_pinyin_length, next_sep);
            for (size_t n = try_len; n > pos; --n) {
                nextstep = &g_array_index(m_parse_steps, parse_value_t, n);

                /* gen next step */
                const char * onepinyin = input + pos;
                gint16 onepinyinlen = n - pos;
                value = parse_value_t();

                ChewingKey key; ChewingKeyRest rest;
                bool parsed = parse_one_key
                    (options, key, onepinyin, onepinyinlen);
                rest.m_raw_begin = pos; rest.m_raw_end = n;

                if (!parsed)
                    continue;

                //printf("onepinyin:%s len:%d\n", onepinyin, onepinyinlen);
                value.m_key = key; value.m_key_rest = rest;
                value.m_num_keys = curstep->m_num_keys + 1;
                value.m_parsed_len = curstep->m_parsed_len + onepinyinlen;
                value.m_last_step = pos;

                /* save next step */
                if (-1 == nextstep->m_last_step)
                    *nextstep = value;
                if (value.m_parsed_len > nextstep->m_parsed_len)
                    *nextstep = value;
                if (value.m_parsed_len == nextstep->m_parsed_len &&
                    value.m_num_keys < nextstep->m_num_keys)
                    *nextstep = value;

                /* maximum forward, set pos to n in next iteration. */
                pos = n - 1;
                break;
            }
        }
#endif

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
                    (options, key, onepinyin, onepinyinlen);
                rest.m_raw_begin = m; rest.m_raw_end = n;
                if (!parsed)
                    continue;

                //printf("onepinyin:%s len:%d\n", onepinyin, onepinyinlen);

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
                if (value.m_parsed_len == nextstep->m_parsed_len &&
                    value.m_num_keys == nextstep->m_num_keys) {

                    /* "kaneiji" -> "ka'nei'ji" */
                    if ((value.m_key.m_initial != CHEWING_ZERO_INITIAL &&
                         !(value.m_key.m_middle == CHEWING_ZERO_MIDDLE &&
                           value.m_key.m_final == CHEWING_ZERO_FINAL)) &&
                        nextstep->m_key.m_initial == CHEWING_ZERO_INITIAL)
                        *nextstep = value;

                    /* "xierqi" -> "xi'er'qi." */
                    if ((value.m_key.m_initial == CHEWING_ZERO_INITIAL &&
                        value.m_key.m_middle == CHEWING_ZERO_MIDDLE &&
                        value.m_key.m_final == CHEWING_ER) &&
                        (nextstep->m_key.m_initial == CHEWING_R &&
                         nextstep->m_key.m_middle == CHEWING_ZERO_MIDDLE &&
                         nextstep->m_key.m_final == CHEWING_ZERO_FINAL))
                        *nextstep = value;
                }
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


bool FullPinyinParser2::post_process(pinyin_option_t options,
                                     ChewingKeyVector & keys,
                                     ChewingKeyRestVector & key_rests) const {
    int i;
    assert(keys->len == key_rests->len);
    gint16 num_keys = keys->len;

    ChewingKey * cur_key = NULL, * next_key = NULL;
    ChewingKeyRest * cur_rest = NULL, * next_rest = NULL;
    guint16 next_tone = CHEWING_ZERO_TONE;

    for (i = 0; i < num_keys - 1; ++i) {
        cur_key = &g_array_index(keys, ChewingKey, i);
        next_key = &g_array_index(keys, ChewingKey, i + 1);

        /* some "'" here */
        if (0 == cur_key->get_table_index())
            continue;
        if (0 == next_key->get_table_index())
            continue;

        /* some tone here */
        if (CHEWING_ZERO_TONE != cur_key->m_tone)
            continue;

        cur_rest = &g_array_index(key_rests, ChewingKeyRest, i);
        next_rest = &g_array_index(key_rests, ChewingKeyRest, i + 1);

        if (options & USE_TONE) {
            next_tone = next_key->m_tone;
            next_key->m_tone = CHEWING_ZERO_TONE;
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
            cur_rest->m_raw_end ++;
            next_rest->m_raw_begin ++;
        }

        /* save back tones */
        if (options & USE_TONE) {
            next_key->m_tone = next_tone;
        }
    }

    return true;
}

#define IS_KEY(x)   (('a' <= x && x <= 'z') || x == ';')

bool DoublePinyinParser2::parse_one_key(pinyin_option_t options,
                                        ChewingKey & key,
                                        const char *str, int len) const {

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

        if (search_pinyin_index(options, sheng, key)) {
            return true;
        } else {
            return false;
        }
    }

    ChewingTone tone = CHEWING_ZERO_TONE;
    options &= ~(PINYIN_CORRECT_ALL|PINYIN_AMB_ALL);

    /* parse tone */
    if (3 == len) {
        if (!(options & USE_TONE))
            return false;
        char ch = str[2];
        if (!('0' < ch && ch <= '5'))
            return false;
        tone = (ChewingTone) (ch - '0');
    }

    if (2 == len || 3 == len) {
        /* parse shengmu here. */
        char ch = str[0];
        if (!IS_KEY(ch))
            return false;

        int charid = ch == ';' ? 26 : ch - 'a';
        const char * sheng = m_shengmu_table[charid].m_shengmu;
        if (NULL == sheng)
            return false;
        if (strcmp(sheng, "'") == 0)
            sheng = "";

        /* parse yunmu here. */
        ch = str[1];
        if (!IS_KEY(ch))
            return false;

        charid = ch == ';' ? 26 : ch - 'a';
        /* first yunmu */
        const char * yun = m_yunmu_table[charid].m_yunmus[0];
        gchar * pinyin = g_strdup_printf("%s%s", sheng, yun);
        if (search_pinyin_index(options, pinyin, key)) {
            key.m_tone = tone;
            g_free(pinyin);
            return true;
        }
        g_free(pinyin);

        /* second yunmu */
        yun = m_yunmu_table[charid].m_yunmus[1];
        pinyin = g_strdup_printf("%s%s", sheng, yun);
        if (search_pinyin_index(options, pinyin, key)) {
            key.m_tone = tone;
            g_free(pinyin);
            return true;
        }
        g_free(pinyin);

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

        ChewingKey key; ChewingKeyRest key_rest;
        for (; i > 0; --i) {
            bool success = parse_one_key(options, key, cur_str, i);
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

/* the chewing string must be freed with g_free. */
static bool search_chewing_symbols(const chewing_symbol_item_t * symbol_table,
                                   const char key, const char ** chewing) {
    *chewing = NULL;
    /* just iterate the table, as we only have < 50 items. */
    while (symbol_table->m_input != '\0') {
        if (symbol_table->m_input == key) {
            *chewing = symbol_table->m_chewing;
            return true;
        }
        symbol_table ++;
    }
    return false;
}

static bool search_chewing_tones(const chewing_tone_item_t * tone_table,
                                 const char key, char * tone) {
    *tone = CHEWING_ZERO_TONE;
    /* just iterate the table, as we only have < 10 items. */
    while (tone_table->m_input != '\0') {
        if (tone_table->m_input == key) {
            *tone = tone_table->m_tone;
            return true;
        }
        tone_table ++;
    }
    return false;
}


bool ChewingParser2::parse_one_key(pinyin_option_t options,
                                   ChewingKey & key,
                                   const char *str, int len) const {
    char tone = CHEWING_ZERO_TONE;

    int symbols_len = len;
    /* probe whether the last key is tone key in str. */
    if (options & USE_TONE) {
        char ch = str[len - 1];
        /* remove tone from input */
        if (search_chewing_tones(m_tone_table, ch, &tone))
            symbols_len --;
    }

    int i;
    gchar * chewing = NULL; const char * onechar = NULL;

    /* probe the possible chewing map in the rest of str. */
    for (i = 0; i < symbols_len; ++i) {
        if (!search_chewing_symbols(m_symbol_table, str[i], &onechar)) {
            g_free(chewing);
            return false;
        }

        if (!chewing) {
            chewing = g_strdup(onechar);
        } else {
            gchar * tmp = chewing;
            chewing = g_strconcat(chewing, onechar, NULL);
            g_free(tmp);
        }
    }

    /* search the chewing in the chewing index table. */
    if (chewing && search_chewing_index(options, chewing, key)) {
        /* save back tone if available. */
        key.m_tone = tone;
        g_free(chewing);
        return true;
    }

    g_free(chewing);
    return false;
}


/* only characters in chewing keyboard scheme are accepted here. */
int ChewingParser2::parse(pinyin_option_t options, ChewingKeyVector & keys,
                          ChewingKeyRestVector & key_rests,
                          const char *str, int len) const {
    g_array_set_size(keys, 0);
    g_array_set_size(key_rests, 0);

    int maximum_len = 0; int i;
    /* probe the longest possible chewing string. */
    for (i = 0; i < len; ++i) {
        if (!in_chewing_scheme(str[i], NULL))
            break;
    }
    maximum_len = i;

    /* maximum forward match for chewing. */
    int parsed_len = 0;
    while (parsed_len < maximum_len) {
        const char * cur_str = str + parsed_len;
        i = std_lite::min(maximum_len - parsed_len,
                          (int)max_chewing_length);

        ChewingKey key; ChewingKeyRest key_rest;
        for (; i > 0; --i) {
            bool success = parse_one_key(options, key, cur_str, i);
            if (success)
                break;
        }

        if (0 == i)        /* no more possible chewings. */
            break;

        key_rest.m_raw_begin = parsed_len; key_rest.m_raw_end = parsed_len + i;
        parsed_len += i;

        /* save the pinyin. */
        g_array_append_val(keys, key);
        g_array_append_val(key_rests, key_rest);
    }

    return parsed_len;
}


bool ChewingParser2::set_scheme(ChewingScheme scheme) {
    switch(scheme) {
    case CHEWING_STANDARD:
        m_symbol_table = chewing_standard_symbols;
        m_tone_table   = chewing_standard_tones;
        return true;
    case CHEWING_IBM:
        m_symbol_table = chewing_ibm_symbols;
        m_tone_table   = chewing_ibm_tones;
        return true;
    case CHEWING_GINYIEH:
        m_symbol_table = chewing_ginyieh_symbols;
        m_tone_table   = chewing_ginyieh_tones;
        return true;
    case CHEWING_ETEN:
        m_symbol_table = chewing_eten_symbols;
        m_tone_table   = chewing_eten_tones;
        return true;
    }

    return false;
}


bool ChewingParser2::in_chewing_scheme(const char key, const char ** symbol)
 const {
    const gchar * chewing = NULL;
    char tone = CHEWING_ZERO_TONE;

    if (search_chewing_symbols(m_symbol_table, key, &chewing)) {
        if (symbol)
            *symbol = chewing;
        return true;
    }

    if (search_chewing_tones(m_tone_table, key, &tone)) {
        if (symbol)
            *symbol = chewing_tone_table[tone];
        return true;
    }

    return false;
}
