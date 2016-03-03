/* 
 *  libpinyin
 *  Library to deal with pinyin.
 *  
 *  Copyright (C) 2016 Peng Wu <alexepico@gmail.com>
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

#ifndef BDB_UTILS_H
#define BDB_UTILS_H

#include <assert.h>
#include <db.h>

inline u_int32_t attach_option(guint32 flags) {
    u_int32_t db_flags = 0;

    if (flags & ATTACH_READONLY)
        db_flags |= DB_RDONLY;
    if (flags & ATTACH_READWRITE)
        assert(!(flags & ATTACH_READONLY));
    if (flags & ATTACH_CREATE)
        db_flags |= DB_CREATE;

    return db_flags;
}

#endif
