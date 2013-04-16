/* 
 *  libpinyin
 *  Library to deal with pinyin.
 *  
 *  Copyright (C) 2010 Peng Wu
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
#include "utils_helper.h"

static const gchar * table_dir = ".";

static GOptionEntry entries[] =
{
    {"table-dir", 0, 0, G_OPTION_ARG_FILENAME, &table_dir, "table directory", NULL},
    {NULL}
};

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

    SystemTableInfo system_table_info;

    gchar * filename = g_build_filename(table_dir, SYSTEM_TABLE_INFO, NULL);
    bool retval = system_table_info.load(filename);
    if (!retval) {
        fprintf(stderr, "load table.conf failed.\n");
        exit(ENOENT);
    }
    g_free(filename);

    /* generate pinyin index*/
    pinyin_option_t options = USE_TONE;
    ChewingLargeTable chewing_table(options);
    PhraseLargeTable2 phrase_table;

    /* generate phrase index */
    FacadePhraseIndex phrase_index;

    const pinyin_table_info_t * phrase_files =
        system_table_info.get_table_info();

    for (size_t i = 0; i < PHRASE_INDEX_LIBRARY_COUNT; ++i) {
        const pinyin_table_info_t * table_info = phrase_files + i;
        assert(table_info->m_dict_index == i);

        if (SYSTEM_FILE != table_info->m_file_type &&
            DICTIONARY != table_info->m_file_type)
            continue;

        const char * tablename = table_info->m_table_filename;

        filename = g_build_filename(table_dir, tablename, NULL);
        FILE * tablefile = fopen(filename, "r");

        if (NULL == tablefile) {
            fprintf(stderr, "open %s failed!\n", tablename);
            exit(ENOENT);
        }

        chewing_table.load_text(tablefile);
        fseek(tablefile, 0L, SEEK_SET);
        phrase_table.load_text(tablefile);
        fseek(tablefile, 0L, SEEK_SET);
        phrase_index.load_text(i, tablefile);
        fclose(tablefile);
        g_free(filename);
    }

    MemoryChunk * new_chunk = new MemoryChunk;
    chewing_table.store(new_chunk);
    new_chunk->save(SYSTEM_PINYIN_INDEX);
    chewing_table.load(new_chunk);
    
    new_chunk = new MemoryChunk;
    phrase_table.store(new_chunk);
    new_chunk->save(SYSTEM_PHRASE_INDEX);
    phrase_table.load(new_chunk);

    phrase_index.compact();

    if (!save_phrase_index(phrase_files, &phrase_index))
        exit(ENOENT);

    if (!save_dictionary(phrase_files, &phrase_index))
        exit(ENOENT);

    return 0;
}
