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

#ifndef CHEWING_KEY_H
#define CHEWING_KEY_H

#include <glib.h>
#include "chewing_enum.h"

using namespace pinyin;

G_BEGIN_DECLS

/** @file chewing_key.h
 *  @brief the definitions of chewing key related classes and structs.
 */


/** Note: The parsed pinyins are stored in the following two
 *          GArrays to speed up chewing table lookup.
 *    As the chewing large table only contains information of struct ChewingKey.
 */

struct _ChewingKey
{
    guint16 m_initial : 5;
    guint16 m_middle  : 2;
    guint16 m_final   : 5;
    guint16 m_tone    : 3;
    guint16 m_zero_padding : 1;

    _ChewingKey() {
        m_initial = CHEWING_ZERO_INITIAL;
        m_middle  = CHEWING_ZERO_MIDDLE;
        m_final   = CHEWING_ZERO_FINAL;
        m_tone    = CHEWING_ZERO_TONE;
        m_zero_padding = 0;
    }

    _ChewingKey(ChewingInitial initial, ChewingMiddle middle,
               ChewingFinal final) {
        m_initial = initial;
        m_middle = middle;
        m_final = final;
        m_tone = CHEWING_ZERO_TONE;
        m_zero_padding = 0;
    }

public:
    gint get_table_index();
    bool is_valid_zhuyin();

    /* Note: the return value should be freed by g_free. */
    gchar * get_pinyin_string();
    gchar * get_shengmu_string();
    gchar * get_yunmu_string();
    gchar * get_zhuyin_string();
    gchar * get_luoma_pinyin_string();
    gchar * get_secondary_zhuyin_string();
};

typedef struct _ChewingKey ChewingKey;

static inline bool operator == (ChewingKey lhs, ChewingKey rhs) {
    if (lhs.m_initial != rhs.m_initial)
        return false;
    if (lhs.m_middle  != rhs.m_middle)
        return false;
    if (lhs.m_final   != rhs.m_final)
        return false;
    if (lhs.m_tone    != rhs.m_tone)
        return false;
    return true;
}

static inline bool operator != (ChewingKey lhs, ChewingKey rhs) {
    return !(lhs == rhs);
}

struct _ChewingKeyRest
{
    /* Note: the table index is removed,
     *   Please use get_table_index in ChewingKey.
     */
    guint16 m_raw_begin;           /* the begin of the raw input. */
    guint16 m_raw_end;             /* the end of the raw input. */

    _ChewingKeyRest() {
        /* the 0th item in pinyin parser table is reserved for invalid. */
        m_raw_begin = 0;
        m_raw_end = 0;
    }

    guint16 length() {
        return m_raw_end - m_raw_begin;
    }
};

typedef struct _ChewingKeyRest ChewingKeyRest;

G_END_DECLS

#endif
