/* 
 *  libpinyin
 *  Library to deal with pinyin.
 *  
 *  Copyright (C) 2006-2007 Peng Wu
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

static GOptionEntry entries[] =
{
    {"table-dir", 0, 0, G_OPTION_ARG_FILENAME, &table_dir, "table directory", NULL},
    {NULL}
};

/* increase all unigram frequency by a constant. */
bool generate_unigram(const pinyin_table_info_t * phrase_files) {
    FacadePhraseIndex phrase_index;

    /* Note: please increase the value when corpus size becomes larger.
     *  To avoid zero value when computing unigram frequency in float format.
     */
    for (size_t i = 0; i < PHRASE_INDEX_LIBRARY_COUNT; ++i) {
        const pinyin_table_info_t * table_info = phrase_files + i;
        assert(table_info->m_dict_index == i);

        if (SYSTEM_FILE != table_info->m_file_type &&
            DICTIONARY != table_info->m_file_type)
            continue;

        guint32 freq = 1;

        const char * binfile = table_info->m_system_filename;

        MemoryChunk * chunk = new MemoryChunk;
        bool retval = chunk->load(binfile);
        if (!retval) {
            fprintf(stderr, "load %s failed!\n", binfile);
            exit(ENOENT);
        }

        phrase_index.load(i, chunk);

        PhraseIndexRange range;
        int result = phrase_index.get_range(i, range);
        if ( result == ERROR_OK ) {
            for (size_t token = range.m_range_begin;
                  token < range.m_range_end; ++token) {
                phrase_index.add_unigram_frequency(token, freq);
            }
        }
    }

    if (!save_phrase_index(phrase_files, &phrase_index))
        exit(ENOENT);

    if (!save_dictionary(phrase_files, &phrase_index))
        exit(ENOENT);

    return true;
}

int main(int argc, char * argv[]){
    setlocale(LC_ALL, "");

    GError * error = NULL;
    GOptionContext * context;

    context = g_option_context_new("- increase uni-gram");
    g_option_context_add_main_entries(context, entries, NULL);
    if (!g_option_context_parse(context, &argc, &argv, &error)) {
        g_print("option parsing failed:%s\n", error->message);
        exit(EINVAL);
    }

    SystemTableInfo2 system_table_info;

    const gchar * filename = SYSTEM_TABLE_INFO;
    bool retval = system_table_info.load(filename);
    if (!retval) {
        fprintf(stderr, "load table.conf failed.\n");
        exit(ENOENT);
    }

    const pinyin_table_info_t * phrase_files =
        system_table_info.get_default_tables();

    generate_unigram(phrase_files);

    phrase_files = system_table_info.get_addon_tables();

    generate_unigram(phrase_files);

    return 0;
}
