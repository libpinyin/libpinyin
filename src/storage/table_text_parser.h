/*
 * libpinyin
 * Library to deal with pinyin.
 *
 * Copyright (C) 2026 Hong An
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef TABLE_TEXT_PARSER_H
#define TABLE_TEXT_PARSER_H

#include <array>
#include <errno.h>
#include <limits>
#include <stdio.h>
#include <string.h>
#include <glib.h>
#include "novel_types.h"

namespace pinyin{

static inline bool table_text_split_line(char * line,
                                         std::array<char *, 4> & fields) {
    char * cur = line;

    for (size_t i = 0; i < fields.size(); ++i) {
        cur += strspn(cur, " \t");
        if (*cur == '\0' || *cur == '\r' || *cur == '\n')
            return false;

        fields[i] = cur;
        cur += strcspn(cur, " \t\r\n");
        if (*cur != '\0') {
            *cur = '\0';
            ++cur;
        }
    }

    return true;
}

template<typename Integer>
static inline bool table_text_parse_integer(const char * field,
                                            Integer & value) {
    if (field == NULL || *field == '\0' || *field == '-')
        return false;

    errno = 0;
    char * end = NULL;
    guint64 parsed = g_ascii_strtoull(field, &end, 10);
    if (errno != 0 || end == field || *end != '\0' ||
        parsed > (guint64) std::numeric_limits<Integer>::max())
        return false;

    value = (Integer) parsed;
    return true;
}

static inline bool table_text_copy_field(char dest[256], const char * field) {
    return field != NULL && g_strlcpy(dest, field, 256) < 256;
}

static inline bool table_text_read_pinyin_record(FILE * infile,
                                                 char pinyin[256],
                                                 char phrase[256],
                                                 phrase_token_t * token,
                                                 size_t * freq) {
    char line[4096];

    while (fgets(line, sizeof(line), infile)) {
        std::array<char *, 4> fields;
        if (!table_text_split_line(line, fields))
            continue;

        if (!table_text_copy_field(pinyin, fields[0]) ||
            !table_text_copy_field(phrase, fields[1]) ||
            !table_text_parse_integer<phrase_token_t>(fields[2], *token) ||
            !table_text_parse_integer<size_t>(fields[3], *freq))
            continue;

        return true;
    }

    return false;
}

static inline bool table_text_read_punct_record(FILE * infile,
                                                phrase_token_t * token,
                                                char phrase[256],
                                                char punct[256],
                                                size_t * freq) {
    char line[4096];

    while (fgets(line, sizeof(line), infile)) {
        std::array<char *, 4> fields;
        if (!table_text_split_line(line, fields))
            continue;

        if (!table_text_parse_integer<phrase_token_t>(fields[0], *token) ||
            !table_text_copy_field(phrase, fields[1]) ||
            !table_text_copy_field(punct, fields[2]) ||
            !table_text_parse_integer<size_t>(fields[3], *freq))
            continue;

        return true;
    }

    return false;
}

};

#endif
