/* 
 *  libpinyin
 *  Library to deal with pinyin.
 *  
 *  Copyright (C) 2010 Peng Wu
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

#include <stdio.h>
#include <locale.h>
#include "pinyin_internal.h"
#include "utils_helper.h"

static const gchar * table_dir = ".";
static gboolean gen_punct_table = FALSE;

static GOptionEntry entries[] =
{
    {"table-dir", 0, 0, G_OPTION_ARG_FILENAME, &table_dir, "table directory", NULL},
    {"gen-punct-table", 0, 0, G_OPTION_ARG_NONE, &gen_punct_table, "generate punctuation table", NULL},
    {NULL}
};

bool generate_binary_files(const char * pinyin_table_filename,
                           const char * phrase_table_filename,
                           const pinyin_table_info_t * phrase_files,
                           TABLE_PHONETIC_TYPE type) {
    /* generate pinyin index*/
    ChewingLargeTable2 pinyin_table;
    pinyin_table.attach(pinyin_table_filename, ATTACH_READWRITE|ATTACH_CREATE);

    PhraseLargeTable3 phrase_table;
    phrase_table.attach(phrase_table_filename, ATTACH_READWRITE|ATTACH_CREATE);

    /* generate phrase index */
    FacadePhraseIndex phrase_index;

    for (size_t i = 0; i < PHRASE_INDEX_LIBRARY_COUNT; ++i) {
        const pinyin_table_info_t * table_info = phrase_files + i;
        assert(table_info->m_dict_index == i);

        if (SYSTEM_FILE != table_info->m_file_type &&
            DICTIONARY != table_info->m_file_type)
            continue;

        const char * tablename = table_info->m_table_filename;

        gchar * filename = g_build_filename(table_dir, tablename, NULL);
        FILE * tablefile = fopen(filename, "r");

        if (NULL == tablefile) {
            fprintf(stderr, "open %s failed!\n", tablename);
            exit(ENOENT);
        }

        pinyin_table.load_text(tablefile, type);
        fseek(tablefile, 0L, SEEK_SET);
        phrase_table.load_text(tablefile);
        fseek(tablefile, 0L, SEEK_SET);
        phrase_index.load_text(i, tablefile, type);
        fclose(tablefile);
        g_free(filename);
    }

    phrase_index.compact();

    if (!save_phrase_index(phrase_files, &phrase_index))
        exit(ENOENT);

    if (!save_dictionary(phrase_files, &phrase_index))
        exit(ENOENT);

    return true;
}

bool generate_punct_table(const char * tablename) {
    PunctTable punct_table;
    bool retval = punct_table.attach(tablename, ATTACH_CREATE|ATTACH_READWRITE);
    if (!retval) {
        fprintf(stderr, "open %s failed!\n", tablename);
        exit(ENOENT);
    }

    gchar * filename = g_build_filename(table_dir, "punct.table", NULL);
    FILE * tablefile = fopen(filename, "r");
    if (NULL == tablefile) {
        fprintf(stderr, "open %s failed!\n", filename);
        exit(ENOENT);
    }

    punct_table.load_text(tablefile);

    fclose(tablefile);
    g_free(filename);

    return true;
}

int main(int argc, char * argv[]){
    setlocale(LC_ALL, "");

    GError * error = NULL;
    GOptionContext * context;

    context = g_option_context_new("- generate binary files");
    g_option_context_add_main_entries(context, entries, NULL);
    if (!g_option_context_parse(context, &argc, &argv, &error)) {
        g_print("option parsing failed:%s\n", error->message);
        exit(EINVAL);
    }
    g_option_context_free(context);

    SystemTableInfo2 system_table_info;

    const gchar * filename = SYSTEM_TABLE_INFO;
    bool retval = system_table_info.load(filename);
    if (!retval) {
        fprintf(stderr, "load table.conf failed.\n");
        exit(ENOENT);
    }

    const pinyin_table_info_t * phrase_files =
        system_table_info.get_default_tables();

    TABLE_PHONETIC_TYPE type = system_table_info.get_table_phonetic_type();
    generate_binary_files(SYSTEM_PINYIN_INDEX,
                          SYSTEM_PHRASE_INDEX,
                          phrase_files, type);

    phrase_files = system_table_info.get_addon_tables();

    generate_binary_files(ADDON_SYSTEM_PINYIN_INDEX,
                          ADDON_SYSTEM_PHRASE_INDEX,
                          phrase_files, type);

    if (gen_punct_table)
        generate_punct_table(SYSTEM_PUNCT_TABLE);

    return 0;
}
