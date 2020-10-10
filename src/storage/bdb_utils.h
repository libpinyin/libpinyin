/* 
 *  libpinyin
 *  Library to deal with pinyin.
 *  
 *  Copyright (C) 2016 Peng Wu <alexepico@gmail.com>
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

#ifndef BDB_UTILS_H
#define BDB_UTILS_H

#include <assert.h>
#include <db.h>

namespace pinyin{

inline u_int32_t attach_options(guint32 flags) {
    u_int32_t db_flags = 0;

    if (flags & ATTACH_READONLY)
        db_flags |= DB_RDONLY;
    if (flags & ATTACH_READWRITE)
        assert(!(flags & ATTACH_READONLY));
    if (flags & ATTACH_CREATE)
        db_flags |= DB_CREATE;

    return db_flags;
}


inline bool copy_bdb(DB * srcdb, DB * destdb) {
    int ret = 0;

    DBC * cursorp = NULL;
    DBT key, data;
    /* Get a cursor */
    srcdb->cursor(srcdb, NULL, &cursorp, 0);

    if (NULL == cursorp)
        return false;

    /* Initialize our DBTs. */
    memset(&key, 0, sizeof(DBT));
    memset(&data, 0, sizeof(DBT));

    /* Iterate over the database, retrieving each record in turn. */
    while ((ret = cursorp->c_get(cursorp, &key, &data, DB_NEXT)) == 0) {
        ret = destdb->put(destdb, NULL, &key, &data, 0);
        assert(0 == ret);

        /* Initialize our DBTs. */
        memset(&key, 0, sizeof(DBT));
        memset(&data, 0, sizeof(DBT));
    }
    assert(DB_NOTFOUND == ret);

    /* Cursors must be closed */
    if ( cursorp != NULL )
        cursorp->c_close(cursorp);

    return true;
}

};
#endif
