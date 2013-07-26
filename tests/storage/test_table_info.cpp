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

#include <stdio.h>
#include <locale.h>
#include "pinyin_internal.h"


int main(int argc, char * argv[]) {
    setlocale(LC_ALL, "");

    SystemTableInfo system_table_info;

    bool retval = system_table_info.load("../../data/table.conf");
    if (!retval) {
        fprintf(stderr, "load table.conf failed.\n");
        exit(ENOENT);
    }

    printf("lambda:%f\n", system_table_info.get_lambda());

    size_t i;
    for (i = 0; i < PHRASE_INDEX_LIBRARY_COUNT; ++i) {
        const pinyin_table_info_t * table_info =
            system_table_info.get_table_info() + i;

        assert(i == table_info->m_dict_index);
        printf("table index:%d\n", table_info->m_dict_index);

        switch(table_info->m_file_type) {
        case NOT_USED:
            printf("not used.\n");
            break;

        case SYSTEM_FILE:
            printf("system file:%s %s %s.\n", table_info->m_table_filename,
                   table_info->m_system_filename, table_info->m_user_filename);
            break;

        case DICTIONARY:
            printf("dictionary:%s %s %s.\n", table_info->m_table_filename,
                   table_info->m_system_filename, table_info->m_user_filename);
            break;

        case USER_FILE:
            printf("user file:%s.\n", table_info->m_user_filename);
            break;

        default:
            assert(false);
        }
    }

    UserTableInfo user_table_info;
    retval = user_table_info.is_conform(&system_table_info);
    assert(!retval);

    user_table_info.make_conform(&system_table_info);
    retval = user_table_info.is_conform(&system_table_info);
    assert(retval);

    assert(user_table_info.save("/tmp/user.conf"));
    assert(user_table_info.load("/tmp/user.conf"));

    retval = user_table_info.is_conform(&system_table_info);
    assert(retval);

    return 0;
}
