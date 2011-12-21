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

#ifndef CHEWING_KEY_H
#define CHEWING_KEY_H

#include <glib.h>
#include "chewing_enum.h"

/** @file chewing_key.h
 *  @brief the definitions of chewing key related classes and structs.
 */

namespace pinyin{


/**
 * @brief enums of Double Pinyin Schemes.
 */
enum DoublePinyinScheme
{
    DOUBLE_PINYIN_ZRM        = 1,
    DOUBLE_PINYIN_MS         = 2,
    DOUBLE_PINYIN_ZIGUANG    = 3,
    DOUBLE_PINYIN_ABC        = 4,
    DOUBLE_PINYIN_PYJJ       = 6,
    DOUBLE_PINYIN_XHE        = 7,
    DOUBLE_PINYIN_CUSTOMIZED = 30,        /* for user's keyboard */
    DOUBLE_PINYIN_DEFAULT    = DOUBLE_PINYIN_MS
};

/**
 * @brief enums of Chewing Schemes.
 */
enum ChewingScheme
{
    CHEWING_STANDARD = 1,
    CHEWING_IBM      = 2,
    CHEWING_GINYIEH  = 3,
    CHEWING_ETEN     = 4,
    CHEWING_DEFAULT  = CHEWING_STANDARD
};


/** Note: The parsed pinyins are stored in the following two
 *          GArrays to speed up chewing table lookup.
 *    As the chewing large table only contains information of struct ChewingKey.
 */

struct ChewingKey
{
    guint16 m_initial : 5;
    guint16 m_middle  : 2;
    guint16 m_final   : 5;
    guint16 m_tone    : 3;

    ChewingKey() {
        m_initial = CHEWING_ZERO_INITIAL;
        m_middle  = CHEWING_ZERO_MIDDLE;
        m_final   = CHEWING_ZERO_FINAL;
        m_tone    = CHEWING_ZERO_TONE;
    }

    ChewingKey(ChewingInitial initial, ChewingMiddle middle,
               ChewingFinal final) {
        m_initial = initial;
        m_middle = middle;
        m_final = final;
        m_tone = CHEWING_ZERO_TONE;
    }

public:
    gint get_table_index();

    /* Note: the return value should be freed by g_free. */
    gchar * get_pinyin_string();
    gchar * get_chewing_string();
};

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

struct ChewingKeyRest
{
    /* Note: the table index is removed,
     *   Please use get_table_index in ChewingKey.
     */
    guint16 m_raw_begin;           /* the begin of the raw input. */
    guint16 m_raw_end;             /* the end of the raw input. */

    ChewingKeyRest() {
        /* the 0th item in pinyin parser table is reserved for invalid. */
        m_raw_begin = 0;
        m_raw_end = 0;
    }

    guint16 length() {
        return m_raw_end - m_raw_begin;
    }
};

};

#endif
