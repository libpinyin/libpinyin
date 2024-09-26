/* 
 *  libpinyin
 *  Library to deal with pinyin.
 *  
 *  Copyright (C) 2024 Peng Wu <alexepico@gmail.com>
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


#include "punct_table.h"

using namespace pinyin;

static const ucs4_t null_char = 0;

PunctTableEntry::PunctTableEntry() {
    m_ucs4_cache = g_array_new(TRUE, TRUE, sizeof(ucs4_t));
    m_utf8_cache = g_string_new(NULL);
}

PunctTableEntry::~PunctTableEntry() {
    g_array_free(m_ucs4_cache, TRUE);
    m_ucs4_cache = NULL;
    g_string_free(m_utf8_cache, TRUE);
    m_utf8_cache = NULL;
}

bool PunctTableEntry::escape(const gchar * punct, gint maxlen) {
    if (maxlen == -1)
        maxlen = G_MAXINT;

    g_array_set_size(m_ucs4_cache, 0);

    glong ucs4_len = 0;
    gunichar * ucs4_str = g_utf8_to_ucs4(punct, maxlen, NULL, &ucs4_len, NULL);

    g_array_append_vals(m_ucs4_cache, ucs4_str, ucs4_len);
    g_array_append_val(m_ucs4_cache, null_char);

    g_free(ucs4_str);
    return true;
}

int PunctTableEntry::unescape(const ucs4_t * punct, gint maxlen) {
    if (maxlen == -1)
        maxlen = G_MAXINT;

    g_string_set_size(m_utf8_cache, 0);

    int index = 0;
    while (index < maxlen) {
        g_string_append_unichar(m_utf8_cache, punct[index]);
        index++;
        if (punct[index] == null_char)
            break;
    }

    return index + 1;
}

bool PunctTableEntry::get_all_punctuations(gchar ** & puncts) {
    assert(puncts == NULL);

    size_t size = m_chunk.size();
    if (size == 0)
        return false;

    GPtrArray * array = g_ptr_array_new();
    ucs4_t * begin = (ucs4_t *) m_chunk.begin();
    ucs4_t * end = (ucs4_t *) m_chunk.end();

    while (begin < end) {
        int len = unescape(begin, end - begin);
        g_ptr_array_add(array, g_strdup(m_utf8_cache->str));
        begin += len;
    }

    g_ptr_array_add(array, NULL);
    /* must be freed by g_strfreev. */
    puncts = (gchar **) g_ptr_array_free(array, FALSE);
    return true;
}

bool PunctTableEntry::append_punctuation(const gchar * punct) {
    gchar ** puncts = NULL;

    get_all_punctuations(puncts);
    if (puncts && g_strv_contains(puncts, punct)) {
        fprintf(stderr, "duplicated punctuations: %s\n", punct);
        g_strfreev(puncts);
        return false;
    }
    g_strfreev(puncts);

    if (!escape(punct))
        return false;

    m_chunk.append_content
        (m_ucs4_cache->data, m_ucs4_cache->len * sizeof(ucs4_t));

    return true;
}

bool PunctTableEntry::remove_punctuation(const gchar * punct) {
    if (m_chunk.size() == 0)
        return false;

    if (!escape(punct))
        return false;

    ucs4_t * begin = (ucs4_t *) m_chunk.begin();
    ucs4_t * end = (ucs4_t *) m_chunk.end();
    int index = 0;

    int len = m_ucs4_cache->len;
    while (begin + index + len <= end) {
        /* match the punctuation */
        if (0 == memcmp(begin + index, m_ucs4_cache->data,
                        len * sizeof(ucs4_t))) {
            m_chunk.remove_content
                (index * sizeof(ucs4_t),
                 len * sizeof(ucs4_t));
            return true;
        }

        /* check the next punctuation index */
        while (null_char != *(begin + index))
            index++;
        index++;
    }

    return false;
}
