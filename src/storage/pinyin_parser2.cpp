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
#include "pinyin_custom2.h"
#include "chewing_key.h"
#include "pinyin_parser2.h"
#include "pinyin_parser_table.h"


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
        m_last_step = 0;
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

    /* init m_parse_steps. */
    int step_len = len + 1;
    g_array_set_size(m_parse_steps, 0);
    parse_value_t onestep;
    for (i = 0; i < step_len; ++i) {
        g_array_append_val(m_parse_steps, onestep);
    }

    /* split "'" here. */
    gchar * input = g_strndup(str, len);
    gchar ** inputs = g_strsplit(input, "'", -1);
    g_free(input);
    /* parse each input */
    for (i = 0; inputs[i]; ++i) {
        input = inputs[i];
        /* dynamic programming here. */
        size_t str_len = strlen(input);
        for (size_t m = 0; m < str_len; ++m) {
            size_t try_len = std_lite::min
                (m + max_full_pinyin_length, str_len);
            for (size_t n = m + 1; n < try_len + 1; ++n) {
                /* gen next step */
            }
        }
    }
    g_strfreev(inputs);

    /* post processing for re-split table. */

    /* final step for back tracing. */
}
