/* 
 *  libpinyin
 *  Library to deal with pinyin.
 *  
 *  Copyright (C) 2025 Peng Wu <alexepico@gmail.com>
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

#ifndef TKRZWDB_UTILS_H
#define TKRZWDB_UTILS_H

#include <assert.h>
#include <tkrzw_dbm.h>
#include <tkrzw_file_util.h>
#include <memory>
#include <string>

namespace pinyin{

inline int32_t attach_options(guint32 flags, bool & writable) {
    int32_t options = 0;

    if (flags & ATTACH_READONLY)
        writable = false;
    if (flags & ATTACH_READWRITE) {
        assert( !( flags & ATTACH_READONLY ) );
        writable = true;
        options = tkrzw::File::OPEN_DEFAULT;
    }
    if ( !(flags & ATTACH_CREATE) )
        options |= tkrzw::File::OPEN_NO_CREATE;

    return options;
}

inline bool copy_tkrzwdb(tkrzw::DBM* srcdb, tkrzw::DBM* destdb) {
    if (!srcdb || !destdb)
        return false;

    std::unique_ptr<tkrzw::DBM::Iterator> iter = srcdb->MakeIterator();
    if (!iter)
        return false;

    iter->First();

    std::string key;
    std::string value;
    tkrzw::Status get_status;

    while ((get_status = iter->Get(&key, &value)) == tkrzw::Status::SUCCESS) {
        tkrzw::Status set_status = destdb->Set(key, value);

        assert (set_status == tkrzw::Status::SUCCESS);
        if (set_status != tkrzw::Status::SUCCESS)
            return false;

        iter->Next();
    }

    assert (get_status == tkrzw::Status::NOT_FOUND_ERROR);
    return (get_status == tkrzw::Status::NOT_FOUND_ERROR);
}

};

#endif
