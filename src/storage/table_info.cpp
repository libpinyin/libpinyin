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

#include "table_info.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <locale.h>

using namespace pinyin;


#if 0
static const pinyin_table_info_t reserved_tables[] = {
    {RESERVED, NULL, NULL, NULL, NOT_USED},
    {GB_DICTIONARY, "gb_char.table", "gb_char.bin", "gb_char.dbin", SYSTEM_FILE},
    {GBK_DICTIONARY, "gbk_char.table", "gbk_char.bin", "gbk_char.dbin", SYSTEM_FILE},

    {MERGED_DICTIONARY, "merged.table", "merged.bin", "merged.dbin", SYSTEM_FILE},

    {USER_DICTIONARY, NULL, NULL, "user.bin", USER_FILE}
};
#endif


SystemTableInfo2::SystemTableInfo2() {
    m_binary_format_version = 0;
    m_model_data_version = 0;
    m_lambda = 0.;

    m_table_phonetic_type = PINYIN_TABLE;

#define INIT_TABLE_INFO(tables, index) do {                 \
        pinyin_table_info_t * table_info = &tables[index];  \
                                                            \
        table_info->m_dict_index = index;                   \
        table_info->m_table_filename = NULL;                \
        table_info->m_system_filename = NULL;               \
        table_info->m_user_filename = NULL;                 \
        table_info->m_file_type = NOT_USED;                 \
    } while (0)

    size_t i;
    for (i = 0; i < PHRASE_INDEX_LIBRARY_COUNT; ++i) {
        INIT_TABLE_INFO(m_default_tables, i);
        INIT_TABLE_INFO(m_addon_tables, i);
    }

#undef INIT_TABLE_INFO
}

SystemTableInfo2::~SystemTableInfo2() {
    reset();
}

void SystemTableInfo2::reset() {
    m_binary_format_version = 0;
    m_model_data_version = 0;
    m_lambda = 0.;

    m_table_phonetic_type = PINYIN_TABLE;

#define FINI_TABLE_INFO(tables, index) do {                \
        pinyin_table_info_t * table_info = &tables[index];  \
                                                            \
        g_free((gchar *)table_info->m_table_filename);      \
        table_info->m_table_filename = NULL;                \
        g_free((gchar *)table_info->m_system_filename);     \
        table_info->m_system_filename = NULL;               \
        g_free((gchar *)table_info->m_user_filename);       \
        table_info->m_user_filename = NULL;                 \
                                                            \
        table_info->m_file_type = NOT_USED;                 \
    } while(0)

    size_t i;
    for (i = 0; i < PHRASE_INDEX_LIBRARY_COUNT; ++i) {
        FINI_TABLE_INFO(m_default_tables, i);
        FINI_TABLE_INFO(m_addon_tables, i);
    }

#undef FINI_TABLE_INFO
}

#define HANDLE(x) do {                          \
        if (0 == strcmp(#x, str))               \
            return x;                           \
    } while (0)


static TABLE_PHONETIC_TYPE to_table_phonetic_type(const char * str) {
    if (0 == strcmp("pinyin", str))
        return PINYIN_TABLE;

    if (0 == strcmp("zhuyin", str))
        return ZHUYIN_TABLE;

    assert(FALSE);
}

static TABLE_DATABASE_FORMAT_TYPE to_table_database_format_type(const char * str) {
    if (0 == strcmp("BerkeleyDB", str))
        return BERKELEY_DB_FORMAT;

    if (0 == strcmp("KyotoCabinet", str))
        return KYOTO_CABINET_FORMAT;

    assert(FALSE);
}

static TABLE_TARGET to_table_target(const char * str) {
    if (0 == strcmp("default", str))
        return DEFAULT_TABLE;

    if (0 == strcmp("addon", str))
        return ADDON_TABLE;

    assert(FALSE);
}

static guint8 to_index_of_default_tables(const char * str) {
    HANDLE(RESERVED);
    HANDLE(GB_DICTIONARY);
    HANDLE(TSI_DICTIONARY);
    HANDLE(GBK_DICTIONARY);
    HANDLE(OPENGRAM_DICTIONARY);
    HANDLE(MERGED_DICTIONARY);
    HANDLE(ADDON_DICTIONARY);
    HANDLE(NETWORK_DICTIONARY);
    HANDLE(USER_DICTIONARY);

    assert(FALSE);
}

static guint8 to_index_of_addon_tables(const char * str) {
    return atoi(str);
}

static gchar * to_string(const char * str) {
    HANDLE(NULL);

    return g_strdup(str);
}

static PHRASE_FILE_TYPE to_file_type(const char * str) {
    HANDLE(NOT_USED);
    HANDLE(SYSTEM_FILE);
    HANDLE(DICTIONARY);
    HANDLE(USER_FILE);

    assert(FALSE);
}

#undef HANDLE

static const char * from_table_database_format_type(const TABLE_DATABASE_FORMAT_TYPE format) {
    if (format == BERKELEY_DB_FORMAT)
        return "BerkeleyDB";

    if (format == KYOTO_CABINET_FORMAT)
        return "KyotoCabinet";

    assert(FALSE);
}


bool SystemTableInfo2::load(const char * filename) {
    reset();

    char * locale = setlocale(LC_NUMERIC, "C");

    FILE * input = fopen(filename, "r");
    if (NULL == input) {
        fprintf(stderr, "open %s failed.\n", filename);
        return false;
    }

    int binver = 0, modelver = 0;
    gfloat lambda = 0.;

    int num = fscanf(input, "binary format version:%d\n", &binver);
    if (1 != num) {
        fclose(input);
        return false;
    }

    num = fscanf(input, "model data version:%d\n", &modelver);
    if (1 != num) {
        fclose(input);
        return false;
    }

    num = fscanf(input, "lambda parameter:%f\n", &lambda);
    if (1 != num) {
        fclose(input);
        return false;
    }

    TABLE_PHONETIC_TYPE type = PINYIN_TABLE;
    char str[256];
    num = fscanf(input, "source table format:%255s\n", str);
    type = to_table_phonetic_type(str);

    TABLE_DATABASE_FORMAT_TYPE format = UNKNOWN_FORMAT;
    num = fscanf(input, "database format:%255s\n", str);
    format = to_table_database_format_type (str);

#if 0
    printf("binver:%d modelver:%d lambda:%f\n", binver, modelver, lambda);
    printf("type:%d\n", type);
#endif

    m_binary_format_version = binver;
    m_model_data_version = modelver;
    m_lambda = lambda;

    /* Note: support pinyin and zhuyin table now. */
    assert(PINYIN_TABLE == type || ZHUYIN_TABLE == type);
    m_table_phonetic_type = type;
    assert(BERKELEY_DB_FORMAT == format || KYOTO_CABINET_FORMAT == format);
    m_table_database_format_type = format;

    int index = 0;
    char tableinfo[256], dictstr[256];
    char tablefile[256], sysfile[256], userfile[256], filetype[256];
    while (!feof(input)) {
        num = fscanf(input, "%255s %255s %255s %255s %255s %255s\n",
                     tableinfo, dictstr, tablefile,
                     sysfile, userfile, filetype);

        if (6 != num)
            continue;

        /* decode the table info and the index. */
        pinyin_table_info_t * tables = NULL;
        TABLE_TARGET target = to_table_target(tableinfo);

        if (DEFAULT_TABLE == target) {
            tables = m_default_tables;
            index = to_index_of_default_tables(dictstr);
        }

        if (ADDON_TABLE == target) {
            tables = m_addon_tables;
            index = to_index_of_addon_tables(dictstr);
        }

        assert(0 <= index && index < PHRASE_INDEX_LIBRARY_COUNT);

        /* save into m_table_info. */
        pinyin_table_info_t * table_info = &tables[index];
        assert(index == table_info->m_dict_index);

        table_info->m_table_filename = to_string(tablefile);
        table_info->m_system_filename = to_string(sysfile);
        table_info->m_user_filename = to_string(userfile);

        table_info->m_file_type = to_file_type(filetype);
    }

    fclose(input);

    setlocale(LC_NUMERIC, locale);

    return true;
}

const pinyin_table_info_t * SystemTableInfo2::get_default_tables() {
    return m_default_tables;
}

const pinyin_table_info_t * SystemTableInfo2::get_addon_tables() {
    return m_addon_tables;
}

gfloat SystemTableInfo2::get_lambda() {
    return m_lambda;
}

TABLE_PHONETIC_TYPE SystemTableInfo2::get_table_phonetic_type() {
    return m_table_phonetic_type;
}


UserTableInfo::UserTableInfo() {
    m_binary_format_version = 0;
    m_model_data_version = 0;
}

void UserTableInfo::reset() {
    m_binary_format_version = 0;
    m_model_data_version = 0;
}

bool UserTableInfo::load(const char * filename) {
    reset();

    char * locale = setlocale(LC_NUMERIC, "C");

    FILE * input = fopen(filename, "r");
    if (NULL == input) {
        fprintf(stderr, "open %s failed.", filename);
        return false;
    }

    int binver = 0, modelver = 0;

    int num = fscanf(input, "binary format version:%d\n", &binver);
    if (1 != num) {
        fclose(input);
        return false;
    }

    num = fscanf(input, "model data version:%d\n", &modelver);
    if (1 != num) {
        fclose(input);
        return false;
    }

    TABLE_DATABASE_FORMAT_TYPE format = UNKNOWN_FORMAT;
    char str[256];
    num = fscanf(input, "database format:%255s\n", str);
    if (EOF != num)
        format = to_table_database_format_type (str);

#if 0
    printf("binver:%d modelver:%d\n", binver, modelver);
#endif

    m_binary_format_version = binver;
    m_model_data_version = modelver;
    m_table_database_format_type = format;

    fclose(input);

    setlocale(LC_NUMERIC, locale);

    return true;
}

bool UserTableInfo::save(const char * filename) {
    char * locale = setlocale(LC_NUMERIC, "C");

    FILE * output = fopen(filename, "w");
    if (NULL == output) {
        fprintf(stderr, "write %s failed.\n", filename);
        return false;
    }

    fprintf(output, "binary format version:%d\n", m_binary_format_version);
    fprintf(output, "model data version:%d\n", m_model_data_version);
    fprintf(output, "database format:%s\n",
            from_table_database_format_type (m_table_database_format_type));

    fclose(output);

    setlocale(LC_NUMERIC, locale);

    return true;
}

bool UserTableInfo::is_conform(const SystemTableInfo2 * sysinfo) {
    if (sysinfo->m_binary_format_version != m_binary_format_version)
        return false;

    if (sysinfo->m_model_data_version != m_model_data_version)
        return false;

    if (sysinfo->m_table_database_format_type != m_table_database_format_type)
        return false;

    return true;
}

bool UserTableInfo::make_conform(const SystemTableInfo2 * sysinfo) {
    m_binary_format_version = sysinfo->m_binary_format_version;
    m_model_data_version = sysinfo->m_model_data_version;
    m_table_database_format_type = sysinfo->m_table_database_format_type;
    return true;
}
