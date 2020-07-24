/* 
 *  libpinyin
 *  Library to deal with pinyin.
 *  
 *  Copyright (C) 2015 Peng Wu <alexepico@gmail.com>
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

#include "zhuyin_parser2.h"
#include <ctype.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "stl_lite.h"
#include "pinyin_phrase3.h"
#include "pinyin_custom2.h"
#include "chewing_key.h"
#include "pinyin_parser_table.h"
#include "zhuyin_table.h"


using namespace pinyin;

const guint16 max_chewing_length       = 4;  /* include tone. */

const guint16 max_chewing_dachen26_length = 12; /* include tone. */

const guint16 max_utf8_length = 6;


static bool check_chewing_options(pinyin_option_t options, const chewing_index_item_t * item) {
    guint32 flags = item->m_flags;
    assert (flags & IS_ZHUYIN);

    /* handle incomplete chewing. */
    if (flags & ZHUYIN_INCOMPLETE) {
        if (!(options & ZHUYIN_INCOMPLETE))
            return false;
    }

    /* handle correct chewing, currently only one flag per item. */
    flags &= ZHUYIN_CORRECT_ALL;
    options &= ZHUYIN_CORRECT_ALL;

    if (flags) {
        if ((flags & options) != flags)
            return false;
    }

    return true;
}

static bool compare_chewing_less_than(const chewing_index_item_t & lhs,
                                      const chewing_index_item_t & rhs){
    return 0 > strcmp(lhs.m_chewing_input, rhs.m_chewing_input);
}

static inline bool search_chewing_index(pinyin_option_t options,
                                        const chewing_index_item_t * chewing_index,
                                        size_t len,
                                        const char * chewing,
                                        ChewingKey & key){
    chewing_index_item_t item;
    memset(&item, 0, sizeof(item));
    item.m_chewing_input = chewing;

    std_lite::pair<const chewing_index_item_t *,
                   const chewing_index_item_t *> range;
    range = std_lite::equal_range
        (chewing_index, chewing_index + len,
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


/* the chewing string must be freed with g_free. */
static bool search_chewing_symbols(const zhuyin_symbol_item_t * symbol_table,
                                   const char key, const char ** chewing) {
    *chewing = "";
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

static bool search_chewing_tones(const zhuyin_tone_item_t * tone_table,
                                 const char key, unsigned char * tone) {
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

static int search_chewing_symbols2(const zhuyin_symbol_item_t * symbol_table,
                                   const char key,
                                   const char ** first,
                                   const char ** second) {
    int num = 0;
    *first = NULL; *second = NULL;

    /* just iterate the table, as we only have < 50 items. */
    while (symbol_table->m_input != '\0') {
        if (symbol_table->m_input == key) {
            ++num;
            if (NULL == *first) {
                *first = symbol_table->m_chewing;
            } else {
                *second = symbol_table->m_chewing;
            }
        }

        /* search done */
        if (symbol_table->m_input > key)
            break;

        symbol_table++;
    }

    assert(0 <= num && num <= 2);
    return num;
}

bool ZhuyinSimpleParser2::parse_one_key(pinyin_option_t options,
                                        ChewingKey & key,
                                        gint16 & distance,
                                        const char * str, int len) const {
    options &= ~PINYIN_AMB_ALL;
    unsigned char tone = CHEWING_ZERO_TONE;

    int symbols_len = len;
    /* probe whether the last key is tone key in str. */
    if (options & USE_TONE) {
        char ch = str[len - 1];
        /* remove tone from input */
        if (search_chewing_tones(m_tone_table, ch, &tone))
            symbols_len --;

        /* check the force tone option */
        if (options & FORCE_TONE && CHEWING_ZERO_TONE == tone)
            return false;
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
    if (chewing && search_chewing_index(options, zhuyin_index,
                                        G_N_ELEMENTS(zhuyin_index),
                                        chewing, key)) {
        /* save back tone if available. */
        key.m_tone = tone;
        g_free(chewing);
        return true;
    }

    g_free(chewing);
    return false;
}

/* only characters in chewing keyboard scheme are accepted here. */
int ZhuyinSimpleParser2::parse(pinyin_option_t options,
                               ChewingKeyVector & keys,
                               ChewingKeyRestVector & key_rests,
                               const char *str, int len) const {
    /* add keyboard mapping specific options. */
    options |= m_options;

    g_array_set_size(keys, 0);
    g_array_set_size(key_rests, 0);

    int maximum_len = 0; int i;
    /* probe the longest possible chewing string. */
    for (i = 0; i < len; ++i) {
        gchar ** symbols = NULL;
        if (!in_chewing_scheme(options, str[i], symbols)) {
            g_strfreev(symbols);
            break;
        }
        g_strfreev(symbols);
    }
    maximum_len = i;

    /* maximum forward match for chewing. */
    int parsed_len = 0;
    while (parsed_len < maximum_len) {
        const char * cur_str = str + parsed_len;
        i = std_lite::min(maximum_len - parsed_len,
                          (int)max_chewing_length);

        gint16 distance = 0;
        ChewingKey key; ChewingKeyRest key_rest;
        for (; i > 0; --i) {
            bool success = parse_one_key(options, key, distance, cur_str, i);
            if (success)
                break;
        }

        if (0 == i)        /* no more possible chewings. */
            break;

        if (!key.is_valid_zhuyin())
            break;

        key_rest.m_raw_begin = parsed_len; key_rest.m_raw_end = parsed_len + i;
        parsed_len += i;

        /* save the pinyin. */
        g_array_append_val(keys, key);
        g_array_append_val(key_rests, key_rest);
    }

    return parsed_len;
}


bool ZhuyinSimpleParser2::set_scheme(ZhuyinScheme scheme) {
    m_options = ZHUYIN_CORRECT_SHUFFLE;

    switch(scheme) {
    case ZHUYIN_STANDARD:
        m_symbol_table = chewing_standard_symbols;
        m_tone_table   = chewing_standard_tones;
        return true;
    case ZHUYIN_IBM:
        m_symbol_table = chewing_ibm_symbols;
        m_tone_table   = chewing_ibm_tones;
        return true;
    case ZHUYIN_GINYIEH:
        m_symbol_table = chewing_ginyieh_symbols;
        m_tone_table   = chewing_ginyieh_tones;
        return true;
    case ZHUYIN_ETEN:
        m_symbol_table = chewing_eten_symbols;
        m_tone_table   = chewing_eten_tones;
        return true;
    case ZHUYIN_STANDARD_DVORAK:
        m_symbol_table = chewing_standard_dvorak_symbols;
        m_tone_table   = chewing_standard_dvorak_tones;
    default:
        assert(FALSE);
    }

    return false;
}

bool ZhuyinSimpleParser2::in_chewing_scheme(pinyin_option_t options,
                                            const char key,
                                            gchar ** & symbols) const {
    symbols = NULL;
    GPtrArray * array = g_ptr_array_new();

    const gchar * chewing = NULL;
    unsigned char tone = CHEWING_ZERO_TONE;

    if (search_chewing_symbols(m_symbol_table, key, &chewing)) {
        g_ptr_array_add(array, g_strdup(chewing));
        g_ptr_array_add(array, NULL);
        /* must be freed by g_strfreev. */
        symbols = (gchar **) g_ptr_array_free(array, FALSE);
        return true;
    }

    if (!(options & USE_TONE)) {
        g_ptr_array_free(array, TRUE);
        return false;
    }

    if (search_chewing_tones(m_tone_table, key, &tone)) {
        g_ptr_array_add(array, g_strdup(chewing_tone_table[tone]));
        g_ptr_array_add(array, NULL);
        /* must be freed by g_strfreev. */
        symbols = (gchar **) g_ptr_array_free(array, FALSE);
        return true;
    }

    g_ptr_array_free(array, TRUE);
    return false;
}

bool ZhuyinDiscreteParser2::parse_one_key(pinyin_option_t options,
                                          ChewingKey & key,
                                          gint16 & distance,
                                          const char * str, int len) const {
    if (0 == len)
        return false;

    options &= ~PINYIN_AMB_ALL;

    int index = 0;
    const char * initial = "";
    const char * middle = "";
    const char * final = "";
    unsigned char tone = CHEWING_ZERO_TONE;

    /* probe initial */
    if (search_chewing_symbols(m_initial_table, str[index], &initial)) {
        index++;
    }

    if (index == len)
        goto probe;

    /* probe middle */
    if (search_chewing_symbols(m_middle_table, str[index], &middle)) {
        index++;
    }

    if (index == len)
        goto probe;

    /* probe final */
    if (search_chewing_symbols(m_final_table, str[index], &final)) {
        index++;
    }

    if (index == len) {
        /* check the force tone option. */
        if (options & USE_TONE && options & FORCE_TONE)
            return false;
        goto probe;
    }

    /* probe tone */
    if (options & USE_TONE) {
        if (search_chewing_tones(m_tone_table, str[index], &tone)) {
            index ++;
        }
    }

probe:
    /* check the force tone option. */
    if (options & FORCE_TONE && CHEWING_ZERO_TONE == tone) {
        return false;
    }

    gchar * chewing = g_strconcat(initial, middle, final, NULL);

    /* search the chewing in the chewing index table. */
    if (index == len && search_chewing_index(options, m_chewing_index,
                                             m_chewing_index_len,
                                             chewing, key)) {
        /* save back tone if available. */
        key.m_tone = tone;
        g_free(chewing);
        return true;
    }

    g_free(chewing);
    return false;
}

/* only characters in chewing keyboard scheme are accepted here. */
int ZhuyinDiscreteParser2::parse(pinyin_option_t options,
                                 ChewingKeyVector & keys,
                                 ChewingKeyRestVector & key_rests,
                                 const char *str, int len) const {
    /* add keyboard mapping specific options. */
    options |= m_options;

    g_array_set_size(keys, 0);
    g_array_set_size(key_rests, 0);

    int maximum_len = 0; int i;
    /* probe the longest possible chewing string. */
    for (i = 0; i < len; ++i) {
        gchar ** symbols = NULL;
        if (!in_chewing_scheme(options, str[i], symbols)) {
            g_strfreev(symbols);
            break;
        }
        g_strfreev(symbols);
    }
    maximum_len = i;

    /* maximum forward match for chewing. */
    int parsed_len = 0;
    while (parsed_len < maximum_len) {
        const char * cur_str = str + parsed_len;
        i = std_lite::min(maximum_len - parsed_len,
                          (int)max_chewing_length);

        gint16 distance = 0;
        ChewingKey key; ChewingKeyRest key_rest;
        for (; i > 0; --i) {
            bool success = parse_one_key(options, key, distance, cur_str, i);
            if (success)
                break;
        }

        if (0 == i)        /* no more possible chewings. */
            break;

        if (!key.is_valid_zhuyin())
            break;

        key_rest.m_raw_begin = parsed_len; key_rest.m_raw_end = parsed_len + i;
        parsed_len += i;

        /* save the pinyin. */
        g_array_append_val(keys, key);
        g_array_append_val(key_rests, key_rest);
    }

    return parsed_len;
}

bool ZhuyinDiscreteParser2::set_scheme(ZhuyinScheme scheme) {
    m_options = 0;

#define INIT_PARSER(index, table) {                     \
        m_chewing_index = index;                        \
        m_chewing_index_len = G_N_ELEMENTS(index);      \
        m_initial_table = chewing_##table##_initials;   \
        m_middle_table  = chewing_##table##_middles;    \
        m_final_table   = chewing_##table##_finals;     \
        m_tone_table    = chewing_##table##_tones;      \
    }

    switch(scheme) {
    case ZHUYIN_HSU:
        m_options = ZHUYIN_CORRECT_HSU;
        INIT_PARSER(hsu_zhuyin_index, hsu);
        break;
    case ZHUYIN_ETEN26:
        m_options = ZHUYIN_CORRECT_ETEN26;
        INIT_PARSER(eten26_zhuyin_index, eten26);
        break;
    case ZHUYIN_HSU_DVORAK:
        m_options = ZHUYIN_CORRECT_HSU;
        INIT_PARSER(hsu_zhuyin_index, hsu_dvorak);
        break;
    default:
        assert(FALSE);
    }

#undef INIT_PARSER

    return true;
}

bool ZhuyinDiscreteParser2::in_chewing_scheme(pinyin_option_t options,
                                              const char key,
                                              gchar ** & symbols) const {
    symbols = NULL;
    GPtrArray * array = g_ptr_array_new();

    const gchar * first = NULL, * second = NULL;
    unsigned char tone = CHEWING_ZERO_TONE;

    if (search_chewing_symbols2(m_initial_table, key, &first, &second)) {
        if (first)
            g_ptr_array_add(array, g_strdup(first));
        if (second)
            g_ptr_array_add(array, g_strdup(second));
    }

    if (search_chewing_symbols2(m_middle_table, key, &first, &second)) {
        if (first)
            g_ptr_array_add(array, g_strdup(first));
        if (second)
            g_ptr_array_add(array, g_strdup(second));
    }

    if (search_chewing_symbols2(m_final_table, key,  &first, &second)) {
        if (first)
            g_ptr_array_add(array, g_strdup(first));
        if (second)
            g_ptr_array_add(array, g_strdup(second));
    }

    if (!(options & USE_TONE))
        goto end;

    if (search_chewing_tones(m_tone_table, key, &tone)) {
        g_ptr_array_add(array, g_strdup(chewing_tone_table[tone]));
    }

end:
    assert(array->len <= 3);

    if (array->len) {
        g_ptr_array_add(array, NULL);
        /* must be freed by g_strfreev. */
        symbols = (gchar **) g_ptr_array_free(array, FALSE);
        return true;
    }

    g_ptr_array_free(array, TRUE);
    return false;
}

ZhuyinDaChenCP26Parser2::ZhuyinDaChenCP26Parser2() {
    m_chewing_index = zhuyin_index;
    m_chewing_index_len = G_N_ELEMENTS(zhuyin_index);

    m_initial_table = chewing_dachen_cp26_initials;
    m_middle_table  = chewing_dachen_cp26_middles;
    m_final_table   = chewing_dachen_cp26_finals;
    m_tone_table    = chewing_dachen_cp26_tones;
}

static int count_same_chars(const char * str, int len) {
    assert(len > 0);

    int count = 0;
    const char cur_char = str[0];

    for (int i = 0; i < len; ++i) {
        if (cur_char != str[i])
            break;
        ++count;
    }

    assert(count >= 1);
    return count;
}

bool ZhuyinDaChenCP26Parser2::parse_one_key(pinyin_option_t options,
                                            ChewingKey & key,
                                            gint16 & distance,
                                            const char *str, int len) const {
    if (0 == len)
        return false;

    options &= ~PINYIN_AMB_ALL;

    const char * initial = "";
    const char * middle  = "";
    const char * final   = "";
    unsigned char tone   = CHEWING_ZERO_TONE;

    gchar * input = g_strndup(str, len);
    int index = 0;

    char ch;
    const char * first = NULL;
    const char * second = NULL;

    /* probe whether the last key is tone key in input. */
    if (options & USE_TONE) {
        ch = input[len - 1];
        /* remove tone from input */
        if (search_chewing_tones(m_tone_table, ch, &tone))
            len --;

        /* check the force tone option. */
        if (options & FORCE_TONE && CHEWING_ZERO_TONE == tone) {
            g_free(input);
            return false;
        }
    }

    if (0 == len)
        return false;

    int choice; int count;

    /* probe initial */
    do {
        ch = input[index];
        count = count_same_chars(input + index, len - index);
        if (search_chewing_symbols2(m_initial_table, ch, &first, &second)) {
            index += count;
            if (NULL == second) {
                initial = first;
                break;
            } else {
                choice = (count - 1) % 2;
                if (0 == choice)
                    initial = first;
                if (1 == choice)
                    initial = second;
            }
        }
    } while (0);

    if (index == len)
        goto probe;

    first = NULL; second = NULL;
    /* probe middle */
    do {
        ch = input[index];
        count = count_same_chars(input + index, len - index);
        /* handle 'u' */
        if ('u' == ch) {
            choice = (count - 1) % 3;
            if (0 == choice)
                middle = "ㄧ";
            if (1 == choice)
                final = "ㄚ";
            if (2 == choice) {
                middle = "ㄧ";
                final = "ㄚ";
            }
        }
        /* handle 'm' */
        if ('m' == ch) {
            choice = (count - 1) % 2;
            if (0 == choice)
                middle = "ㄩ";
            if (1 == choice)
                final = "ㄡ";
        }
        /* handle 'j' */
        if ('j' == ch) {
            middle = "ㄨ";
        }
        if (search_chewing_symbols2(m_middle_table, ch, &first, &second)) {
            index += count;
            assert(NULL == second);
        }
    } while(0);

    if (index == len)
        goto probe;

    /* probe final */
    do {
        /* for 'u' and 'm' */
        if (0 != strlen(final))
            break;

        ch = input[index];
        count = count_same_chars(input + index, len - index);
        if (search_chewing_symbols2(m_final_table, ch, &first, &second)) {
            index += count;
            if (NULL == second) {
                final = first;
                break;
            } else {
                choice = (count - 1) % 2;
                if (0 == choice)
                    final = first;
                if (1 == choice)
                    final = second;
            }
        }
    } while(0);

    if (index == len)
        goto probe;

probe:
    gchar * chewing = g_strconcat(initial, middle, final, NULL);

    /* search the chewing in the chewing index table. */
    if (index == len && search_chewing_index(options, m_chewing_index,
                                             m_chewing_index_len,
                                             chewing, key)) {
        /* save back tone if available. */
        key.m_tone = tone;
        g_free(chewing);
        g_free(input);
        return true;
    }

    g_free(chewing);
    g_free(input);
    return false;
}

int ZhuyinDaChenCP26Parser2::parse(pinyin_option_t options,
                                   ChewingKeyVector & keys,
                                   ChewingKeyRestVector & key_rests,
                                   const char *str, int len) const {
    g_array_set_size(keys, 0);
    g_array_set_size(key_rests, 0);

    int maximum_len = 0; int i;
    /* probe the longest possible chewing string. */
    for (i = 0; i < len; ++i) {
        gchar ** symbols = NULL;
        if (!in_chewing_scheme(options, str[i], symbols)) {
            g_strfreev(symbols);
            break;
        }
        g_strfreev(symbols);
    }
    maximum_len = i;

    /* maximum forward match for chewing. */
    int parsed_len = 0;
    const char * cur_str = NULL;

    while (parsed_len < maximum_len) {
        cur_str = str + parsed_len;
        i = std_lite::min(maximum_len - parsed_len,
                          (int)max_chewing_dachen26_length);

        gint16 distance = 0;
        ChewingKey key; ChewingKeyRest key_rest;
        for (; i > 0; --i) {
            bool success = parse_one_key(options, key, distance, cur_str, i);
            if (success)
                break;
        }

        if (0 == i)        /* no more possible chewings. */
            break;

        if (!key.is_valid_zhuyin())
            break;

        key_rest.m_raw_begin = parsed_len; key_rest.m_raw_end = parsed_len + i;
        parsed_len += i;

        /* save the pinyin. */
        g_array_append_val(keys, key);
        g_array_append_val(key_rests, key_rest);
    }

#if 0
    /* for the last partial input */
    options |= CHEWING_INCOMPLETE;

    cur_str = str + parsed_len;
    i = std_lite::min(maximum_len - parsed_len,
                      (int) max_chewing_dachen26_length);
    for (; i > 0; --i) {
        bool success = parse_one_key(options, key, cur_str, i);
        if (success)
            break;
    }

    if (!key.is_valid_zhuyin())
        return parsed_len;

    if (i > 0) { /* found one */
        key_rest.m_raw_begin = parsed_len; key_rest.m_raw_end = parsed_len + i;
        parsed_len += i;

        /* save the pinyin. */
        g_array_append_val(keys, key);
        g_array_append_val(key_rests, key_rest);
    }
#endif

    return parsed_len;
}


bool ZhuyinDaChenCP26Parser2::in_chewing_scheme(pinyin_option_t options,
                                                const char key,
                                                gchar ** & symbols) const {
    symbols = NULL;
    GPtrArray * array = g_ptr_array_new();

    const gchar * first = NULL, * second = NULL;
    unsigned char tone = CHEWING_ZERO_TONE;

    if (search_chewing_symbols2(m_initial_table, key, &first, &second)) {
        if (first)
            g_ptr_array_add(array, g_strdup(first));
        if (second)
            g_ptr_array_add(array, g_strdup(second));
    }

    if (search_chewing_symbols2(m_middle_table, key, &first, &second)) {
        if (first)
            g_ptr_array_add(array, g_strdup(first));
        if (second)
            g_ptr_array_add(array, g_strdup(second));
    }

    if (search_chewing_symbols2(m_final_table, key,  &first, &second)) {
        if (first)
            g_ptr_array_add(array, g_strdup(first));
        if (second)
            g_ptr_array_add(array, g_strdup(second));
    }

    /* handles for "i" */
    if ('i' == key) {
        g_ptr_array_add(array, g_strdup("ㄧㄚ"));
    }

    if (!(options & USE_TONE))
        goto end;

    if (search_chewing_tones(m_tone_table, key, &tone)) {
        g_ptr_array_add(array, g_strdup(chewing_tone_table[tone]));
    }

end:
    assert(array->len <= 3);

    if (array->len) {
        g_ptr_array_add(array, NULL);
        /* must be freed by g_strfreev. */
        symbols = (gchar **) g_ptr_array_free(array, FALSE);
        return true;
    }

    g_ptr_array_free(array, TRUE);
    return false;
}

ZhuyinDirectParser2::ZhuyinDirectParser2 (){
    m_chewing_index = zhuyin_index;
    m_chewing_index_len = G_N_ELEMENTS(zhuyin_index);
}

bool ZhuyinDirectParser2::parse_one_key(pinyin_option_t options,
                                        ChewingKey & key,
                                        gint16 & distance,
                                        const char *str, int len) const {
    options &= ~PINYIN_AMB_ALL;
    /* by default, chewing will use the first tone. */
    unsigned char tone = CHEWING_1;

    if (0 == len)
        return false;

    const gchar * last_char = NULL;
    for (const char * p = str; p < str + len; p = g_utf8_next_char(p)) {
        last_char = p;
    }

    /* probe tone first. */
    if (options & USE_TONE) {
        gchar buffer[max_utf8_length + 1];
        memset(buffer, 0, sizeof(buffer));
        g_utf8_strncpy(buffer, last_char, 1);

        /* for loop chewing_tone_table. */
        int i = 1;
        for (; i < (int) G_N_ELEMENTS(chewing_tone_table); ++i) {
            const char * symbol = chewing_tone_table[i];
            if (0 == strcmp(symbol, buffer)) {
                tone = i;
                len -= strlen(buffer);
                break;
            }
        }

        /* check the force tone option. */
        if (options & FORCE_TONE && CHEWING_ZERO_TONE == tone) {
            return false;
        }
    }

    gchar * chewing = g_strndup(str, len);
    /* search the chewing in the chewing index table. */
    if (len && search_chewing_index(options, m_chewing_index,
                                    m_chewing_index_len, chewing, key)) {
        /* save back tone if available. */
        key.m_tone = tone;
        g_free(chewing);

        assert(tone != CHEWING_ZERO_TONE);
        return true;
    }

    g_free(chewing);
    return false;
}

int ZhuyinDirectParser2::parse(pinyin_option_t options,
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
#if 0
            /* as direct parser handles data source,
               assume the data is correct when loading. */
            if (!key.is_valid_zhuyin())
                return parsed_len;
#endif

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
