/* 
 *  libpinyin
 *  Library to deal with pinyin.
 *  
 *  Copyright (C) 2013 Peng Wu <alexepico@gmail.com>
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

#ifndef TABLE_INFO_H
#define TABLE_INFO_H

#include "novel_types.h"


namespace pinyin{

typedef enum {
    PINYIN_TABLE,                 /* use pinyin. */
    ZHUYIN_TABLE,                 /* use zhuyin. */
} TABLE_PHONETIC_TYPE;

typedef enum {
    UNKNOWN_FORMAT,
    BERKELEY_DB_FORMAT,
    KYOTO_CABINET_FORMAT,
} TABLE_DATABASE_FORMAT_TYPE;

typedef enum {
    DEFAULT_TABLE,
    ADDON_TABLE,
} TABLE_TARGET;

typedef enum {
    NOT_USED,                /* not used. */
    SYSTEM_FILE,             /* system phrase file. */
    DICTIONARY,              /* professional dictionary. */
    USER_FILE,               /* user only phrase file. */
} PHRASE_FILE_TYPE;

typedef struct {
    guint8 m_dict_index; /* for assert purpose. */
    const gchar * m_table_filename;
    const gchar * m_system_filename;
    const gchar * m_user_filename;
    PHRASE_FILE_TYPE m_file_type;
} pinyin_table_info_t;


class UserTableInfo;

class SystemTableInfo2{
    friend class UserTableInfo;
private:
    int m_binary_format_version;
    int m_model_data_version;
    gfloat m_lambda;

    TABLE_PHONETIC_TYPE m_table_phonetic_type;
    TABLE_DATABASE_FORMAT_TYPE m_table_database_format_type;

    pinyin_table_info_t m_default_tables[PHRASE_INDEX_LIBRARY_COUNT];

    pinyin_table_info_t m_addon_tables[PHRASE_INDEX_LIBRARY_COUNT];

private:
    void reset();

public:
    SystemTableInfo2();

    ~SystemTableInfo2();

    bool load(const char * filename);

    const pinyin_table_info_t * get_default_tables();

    const pinyin_table_info_t * get_addon_tables();

    gfloat get_lambda();

    TABLE_PHONETIC_TYPE get_table_phonetic_type();
};

class UserTableInfo{
private:
    int m_binary_format_version;
    int m_model_data_version;
    TABLE_DATABASE_FORMAT_TYPE m_table_database_format_type;

private:
    void reset();

public:
    UserTableInfo();

    bool load(const char * filename);

    bool save(const char * filename);

    bool is_conform(const SystemTableInfo2 * sysinfo);

    bool make_conform(const SystemTableInfo2 * sysinfo);
};

};


#endif
