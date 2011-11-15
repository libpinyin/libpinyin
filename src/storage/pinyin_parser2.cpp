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

#include <assert.h>
#include "pinyin_custom2.h"
#include "chewing_key.h"
#include "pinyin_parser2.h"
#include "pinyin_parser_table.h"


using namespace pinyin;

static bool check_pinyin_options(guint32 options, pinyin_index_item_t * item) {
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

static bool check_chewing_options(guint32 options, chewing_index_item_t * item) {
    guint32 flags = item->m_flags;
    assert (flags & IS_CHEWING);

    /* handle incomplete chewing. */
    if (flags & CHEWING_INCOMPLETE) {
        if (!(options & CHEWING_INCOMPLETE))
            return false;
    }

    return true;
}

const char * ChewingKeyRest::get_pinyin_string(){
    if (m_index == 0)
        return NULL;

    /* check end boundary. */
    assert(m_index < G_N_ELEMENTS(content_table));
    return content_table[m_index].m_pinyin_str;
}

const char * ChewingKeyRest::get_chewing_string(){
    if (m_index == 0)
        return NULL;

    /* check end boundary. */
    assert(m_index < G_N_ELEMENTS(content_table));
    return content_table[m_index].m_chewing_str;
}
