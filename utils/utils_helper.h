/* 
 *  libpinyin
 *  Library to deal with pinyin.
 *  
 *  Copyright (C) 2012 Peng Wu <alexepico@gmail.com>
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


#ifndef UTILS_HELPER_H
#define UTILS_HELPER_H


#define TAGLIB_GET_TOKEN(var, index)                                    \
    phrase_token_t var = null_token;                                    \
    {                                                                   \
        const char * string = (const char *) g_ptr_array_index          \
            (values, index);                                            \
        var = atoi(string);                                             \
    }

#define TAGLIB_GET_PHRASE_STRING(var, index)                            \
    const char * var = NULL;                                            \
    {                                                                   \
        var = (const char *) g_ptr_array_index                          \
            (values, index);                                            \
    }

#define TAGLIB_GET_TAGVALUE(type, var, conv)                            \
    type var;                                                           \
    {                                                                   \
        gpointer value = NULL;                                          \
        assert(g_hash_table_lookup_extended                             \
               (required, #var, NULL, &value));                         \
        var = conv((const char *)value);                                \
    }

#define TAGLIB_PARSE_SEGMENTED_LINE(phrase_index, var, line)            \
    phrase_token_t var = null_token;                                    \
    do {                                                                \
        if (0 == strlen(line))                                          \
            break;                                                      \
                                                                        \
        gchar ** strs = g_strsplit_set(line, " \t", 2);                 \
        if (2 != g_strv_length(strs))                                   \
            assert(false);                                              \
                                                                        \
        phrase_token_t _token = atoi(strs[0]);                          \
        const char * phrase = strs[1];                                  \
        if (null_token != _token)                                       \
            assert(taglib_validate_token_with_string                    \
                   (phrase_index, _token, phrase));                     \
                                                                        \
        var = _token;                                                   \
                                                                        \
        g_strfreev(strs);                                               \
    } while(false);


inline bool load_phrase_index(const pinyin_table_info_t * phrase_files,
                              FacadePhraseIndex * phrase_index) {
    MemoryChunk * chunk = NULL;
    for (size_t i = 0; i < PHRASE_INDEX_LIBRARY_COUNT; ++i) {
        const pinyin_table_info_t * table_info = phrase_files + i;

        if (SYSTEM_FILE != table_info->m_file_type)
            continue;

        const char * binfile = table_info->m_system_filename;

        chunk = new MemoryChunk;
        bool retval = chunk->load(binfile);
        if (!retval) {
            fprintf(stderr, "load %s failed!\n", binfile);
            delete chunk;
            return false;
        }

        phrase_index->load(i, chunk);
    }
    return true;
}

inline bool save_phrase_index(const pinyin_table_info_t * phrase_files,
                              FacadePhraseIndex * phrase_index) {
    MemoryChunk * new_chunk = NULL;
    for (size_t i = 0; i < PHRASE_INDEX_LIBRARY_COUNT; ++i) {
        const pinyin_table_info_t * table_info = phrase_files + i;

        if (SYSTEM_FILE != table_info->m_file_type)
            continue;

        const char * binfile = table_info->m_system_filename;

        new_chunk = new MemoryChunk;
        phrase_index->store(i, new_chunk);
        bool retval = new_chunk->save(binfile);
        if (!retval) {
            fprintf(stderr, "save %s failed.", binfile);
            delete new_chunk;
            return false;
        }

        phrase_index->load(i, new_chunk);
    }
    return true;
}

inline bool save_dictionary(const pinyin_table_info_t * phrase_files,
                            FacadePhraseIndex * phrase_index) {
    MemoryChunk * new_chunk = NULL;
    for (size_t i = 0; i < PHRASE_INDEX_LIBRARY_COUNT; ++i) {
        const pinyin_table_info_t * table_info = phrase_files + i;

        if (DICTIONARY != table_info->m_file_type)
            continue;

        const char * binfile = table_info->m_system_filename;

        new_chunk = new MemoryChunk;
        phrase_index->store(i, new_chunk);
        bool retval = new_chunk->save(binfile);
        if (!retval) {
            fprintf(stderr, "save %s failed.", binfile);
            delete new_chunk;
            return false;
        }

        phrase_index->load(i, new_chunk);
    }
    return true;
}

#endif
