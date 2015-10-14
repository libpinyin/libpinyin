/* 
 *  libpinyin
 *  Library to deal with pinyin.
 *  
 *  Copyright (C) 2015 Peng Wu <alexepico@gmail.com>
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

#include "chewing_key.h"
#include <assert.h>
#include "pinyin_parser2.h"
#include "pinyin_parser_table.h"
#include "zhuyin_table.h"


gint _ChewingKey::get_table_index() {
    assert(m_initial <  CHEWING_NUMBER_OF_INITIALS);
    assert(m_middle < CHEWING_NUMBER_OF_MIDDLES);
    assert(m_final < CHEWING_NUMBER_OF_FINALS);

    gint index = chewing_key_table[(m_initial * CHEWING_NUMBER_OF_MIDDLES + m_middle) * CHEWING_NUMBER_OF_FINALS + m_final];
    return index == -1 ? 0 : index;
}

gchar * _ChewingKey::get_pinyin_string() {
    assert(m_tone < CHEWING_NUMBER_OF_TONES);
    gint index = get_table_index();
    assert(index < (int) G_N_ELEMENTS(content_table));
    const content_table_item_t & item = content_table[index];

    if (CHEWING_ZERO_TONE == m_tone) {
        return g_strdup(item.m_pinyin_str);
    } else {
        return g_strdup_printf("%s%d", item.m_pinyin_str, m_tone);
    }
}

gchar * _ChewingKey::get_luoma_pinyin_string() {
    assert(m_tone < CHEWING_NUMBER_OF_TONES);
    gint index = get_table_index();
    assert(index < (int) G_N_ELEMENTS(content_table));
    const content_table_item_t & item = content_table[index];

    if (CHEWING_ZERO_TONE == m_tone) {
        return g_strdup(item.m_luoma_pinyin_str);
    } else {
        return g_strdup_printf("%s%d", item.m_luoma_pinyin_str, m_tone);
    }
}

gchar * _ChewingKey::get_secondary_zhuyin_string() {
    assert(m_tone < CHEWING_NUMBER_OF_TONES);
    gint index = get_table_index();
    assert(index < (int) G_N_ELEMENTS(content_table));
    const content_table_item_t & item = content_table[index];

    if (CHEWING_ZERO_TONE == m_tone) {
        return g_strdup(item.m_secondary_zhuyin_str);
    } else {
        return g_strdup_printf("%s%d", item.m_secondary_zhuyin_str, m_tone);
    }
}

gchar * _ChewingKey::get_shengmu_string() {
    gint index = get_table_index();
    assert(index < (int) G_N_ELEMENTS(content_table));
    const content_table_item_t & item = content_table[index];
    return g_strdup(item.m_shengmu_str);
}

gchar * _ChewingKey::get_yunmu_string() {
    gint index = get_table_index();
    assert(index < (int) G_N_ELEMENTS(content_table));
    const content_table_item_t & item = content_table[index];
    return g_strdup(item.m_yunmu_str);
}

gchar * _ChewingKey::get_chewing_string() {
    assert(m_tone < CHEWING_NUMBER_OF_TONES);
    gint index = get_table_index();
    assert(index < (int) G_N_ELEMENTS(content_table));
    const content_table_item_t & item = content_table[index];

    if (CHEWING_ZERO_TONE == m_tone) {
        return g_strdup(item.m_zhuyin_str);
    } else if (CHEWING_1 == m_tone) {
        /* for first tone, usually not display it. */
        return g_strdup(item.m_zhuyin_str);
    } else {
        return g_strdup_printf("%s%s", item.m_zhuyin_str,
                               chewing_tone_table[m_tone]);
    }
}
