/* 
 *  libpinyin
 *  Library to deal with pinyin.
 *  
 *  Copyright (C) 2013 Peng Wu <alexepico@gmail.com>
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

#include "table_info.h"

using namespace pinyin;


static const pinyin_table_info_t reserved_tables[] = {
    {RESERVED, NULL, NULL, NULL, NOT_USED},
    {GB_DICTIONARY, "gb_char.table", "gb_char.bin", "gb_char.dbin", SYSTEM_FILE},
    {GBK_DICTIONARY, "gbk_char.table", "gbk_char.bin", "gbk_char.dbin", SYSTEM_FILE},

    {MERGED_DICTIONARY, "merged.table", "merged.bin", "merged.dbin", SYSTEM_FILE},

    {USER_DICTIONARY, NULL, NULL, "user.bin", USER_FILE}
};
