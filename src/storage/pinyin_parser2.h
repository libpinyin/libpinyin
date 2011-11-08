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

#ifndef PINYIN_PARSER2_H
#define PINYIN_PARSER2_H

#include "chewing_key.h"

typedef struct {
    const char * m_pinyin_str;
    const char * m_chewing_str;
    ChewingKey   m_chewing_key;
} content_table_item_t;

typedef struct {
    const char * m_pinyin_input;
    guint32      m_flags;
    guint16      m_table_index;
} pinyin_index_item_t;

typedef struct {
    const char * m_chewing_input;
    guint32      m_flags;
    guint16      m_table_index;
} chewing_index_item_t;

typedef struct {
    ChewingKey   m_orig_key;
    ChewingKey   m_first_key;
    ChewingKey   m_second_key;
    guint32      m_freq;
} divided_table_item_t;

typedef struct {
    ChewingKey   m_orig_first_key;
    ChewingKey   m_orig_second_key;
    ChewingKey   m_new_first_key;
    ChewingKey   m_new_second_key;
    gfloat       m_benefit;
} resplit_table_item_t;

#endif
