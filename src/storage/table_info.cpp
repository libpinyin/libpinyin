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
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <locale.h>

using namespace pinyin;


static const pinyin_table_info_t reserved_tables[] = {
    {RESERVED, NULL, NULL, NULL, NOT_USED},
    {GB_DICTIONARY, "gb_char.table", "gb_char.bin", "gb_char.dbin", SYSTEM_FILE},
    {GBK_DICTIONARY, "gbk_char.table", "gbk_char.bin", "gbk_char.dbin", SYSTEM_FILE},

    {MERGED_DICTIONARY, "merged.table", "merged.bin", "merged.dbin", SYSTEM_FILE},

    {USER_DICTIONARY, NULL, NULL, "user.bin", USER_FILE}
};


SystemTableInfo::SystemTableInfo() {
    m_binary_format_version = 0;
    m_model_data_version = 0;
    m_lambda = 0.;

    size_t i;
    for (i = 0; i < PHRASE_INDEX_LIBRARY_COUNT; ++i) {
        pinyin_table_info_t * table_info = &m_table_info[i];

        table_info->m_dict_index = i;
        table_info->m_table_filename = NULL;
        table_info->m_system_filename = NULL;
        table_info->m_user_filename = NULL;
        table_info->m_file_type = NOT_USED;
    }
}

SystemTableInfo::~SystemTableInfo() {
    reset();
}

void SystemTableInfo::reset() {
    m_binary_format_version = 0;
    m_model_data_version = 0;
    m_lambda = 0.;

    size_t i;
    for (i = 0; i < PHRASE_INDEX_LIBRARY_COUNT; ++i) {
        pinyin_table_info_t * table_info = &m_table_info[i];

        g_free((gchar *)table_info->m_table_filename);
        table_info->m_table_filename = NULL;
        g_free((gchar *)table_info->m_system_filename);
        table_info->m_system_filename = NULL;
        g_free((gchar *)table_info->m_user_filename);
        table_info->m_user_filename = NULL;

        table_info->m_file_type = NOT_USED;
    }
}

void SystemTableInfo::postfix_tables() {
    size_t i;
    for (i = 0; i < G_N_ELEMENTS(reserved_tables); ++i) {
        const pinyin_table_info_t * postfix = &reserved_tables[i];

        guint8 index = postfix->m_dict_index;
        pinyin_table_info_t * table_info = &m_table_info[index];
        assert(table_info->m_dict_index == index);

        table_info->m_table_filename = g_strdup(postfix->m_table_filename);
        table_info->m_system_filename = g_strdup(postfix->m_system_filename);
        table_info->m_user_filename = g_strdup(postfix->m_user_filename);
        table_info->m_file_type = postfix->m_file_type;
    }
}

static gchar * to_string(const char * str) {
    if (0 == strcmp(str, "NULL"))
        return NULL;

    return g_strdup(str);
}

static PHRASE_FILE_TYPE to_file_type(const char * str) {
#define HANDLE(x) {                             \
        if (0 == strcmp(str, #x))               \
            return x;                           \
    }

    HANDLE(NOT_USED);
    HANDLE(SYSTEM_FILE);
    HANDLE(DICTIONARY);
    HANDLE(USER_FILE);

    assert(false);

#undef HANDLE
}

bool SystemTableInfo::load(const char * filename) {
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

#if 0
    printf("binver:%d modelver:%d lambda:%f\n", binver, modelver, lambda);
#endif

    m_binary_format_version = binver;
    m_model_data_version = modelver;
    m_lambda = lambda;

    int index = 0;
    char tablefile[256], sysfile[256], userfile[256], filetype[256];
    while (!feof(input)) {
        num = fscanf(input, "%d %s %s %s %s\n",
                     &index, tablefile, sysfile, userfile, filetype);

        if (5 != num)
            continue;

        if (!(0 <= index && index < PHRASE_INDEX_LIBRARY_COUNT))
            continue;

        /* save into m_table_info. */
        pinyin_table_info_t * table_info = &m_table_info[index];
        assert(index == table_info->m_dict_index);

        table_info->m_table_filename = to_string(tablefile);
        table_info->m_system_filename = to_string(sysfile);
        table_info->m_user_filename = to_string(userfile);

        table_info->m_file_type = to_file_type(filetype);
    }

    fclose(input);

    /* postfix reserved tables. */
    postfix_tables();

    setlocale(LC_NUMERIC, locale);

    return true;
}

const pinyin_table_info_t * SystemTableInfo::get_table_info() {
    return m_table_info;
}

gfloat SystemTableInfo::get_lambda() {
    return m_lambda;
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

#if 0
    printf("binver:%d modelver:%d\n", binver, modelver);
#endif

    m_binary_format_version = binver;
    m_model_data_version = modelver;

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

    fclose(output);

    setlocale(LC_NUMERIC, locale);

    return true;
}

bool UserTableInfo::is_conform(const SystemTableInfo * sysinfo) {
    if (sysinfo->m_binary_format_version != m_binary_format_version)
        return false;

    if (sysinfo->m_model_data_version != m_model_data_version)
        return false;

    return true;
}

bool UserTableInfo::make_conform(const SystemTableInfo * sysinfo) {
    m_binary_format_version = sysinfo->m_binary_format_version;
    m_model_data_version = sysinfo->m_model_data_version;
    return true;
}
