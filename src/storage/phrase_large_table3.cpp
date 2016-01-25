/* 
 *  libpinyin
 *  Library to deal with pinyin.
 *  
 *  Copyright (C) 2016 Peng Wu <alexepico@gmail.com>
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

#include "phrase_large_table3.h"

namespace pinyin{


table_entry_header_t PhraseTableEntry::get_header() const {
    table_entry_header_t * head = (table_entry_header_t *) m_chunk.begin();
    return *head;
}

void PhraseTableEntry::set_header(table_entry_header_t header) {
    table_entry_header_t * head = (table_entry_header_t *) m_chunk.begin();
    *head = header;
}

int PhraseTableEntry::search(/* out */ PhraseTokens tokens) const {
    int result = SEARCH_NONE;

    const char * content = (char *) m_chunk.begin() +
        sizeof(table_entry_header_t);
    const phrase_token_t * begin = (phrase_token_t *) content;
    const phrase_token_t * end = (phrase_token_t *) m_chunk.end();

    const phrase_token_t * iter = NULL;
    GArray * array = NULL;

    for (iter = begin; iter != end; ++iter) {
        phrase_token_t token = *iter;

        /* filter out disabled sub phrase indices. */
        array = tokens[PHRASE_INDEX_LIBRARY_INDEX(token)];
        if (NULL == array)
            continue;

        result |= SEARCH_OK;

        g_array_append_val(array, token);
    }

    /* check SEARCH_CONTINUED flag in header */
    table_entry_header_t header = get_header();
    if (header & SEARCH_CONTINUED)
        result |= SEARCH_CONTINUED;

    return result;
}


/* load text method */

bool PhraseLargeTable3::load_text(FILE * infile){
    char pinyin[256];
    char phrase[256];
    phrase_token_t token;
    size_t freq;

    while (!feof(infile)) {
        int num = fscanf(infile, "%256s %256s %u %ld",
                         pinyin, phrase, &token, &freq);

        if (4 != num)
            continue;

        if (feof(infile))
            break;

        glong phrase_len = g_utf8_strlen(phrase, -1);
        ucs4_t * new_phrase = g_utf8_to_ucs4(phrase, -1, NULL, NULL, NULL);
        add_index(phrase_len, new_phrase, token);

        g_free(new_phrase);
    }
    return true;
}


};
