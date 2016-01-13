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

#include <datrie/trie.h>
#include "phrase_large_table3.h"

namespace pinyin{
void PhraseLargeTable3::reset() {
    if (m_index) {
        trie_free(m_index);
        m_index = NULL;
    }
    if (m_content) {
        delete m_content;
        m_content = NULL;
    }
}

PhraseLargeTable3::PhraseLargeTable3() {
    AlphaMap * map = alpha_map_new();
    /* include ucs4 characters. */
    alpha_map_add_range(map, 1, UINT_MAX);
    m_index = trie_new(map);
    alpha_map_free(map);

    m_content = new MemoryChunk;
}

bool PhraseLargeTable3::load(FILE * index, MemoryChunk * content) {
    reset();

    m_index = trie_fread(index);
    if (NULL == m_index)
        return false;
    m_content = content;
    return true;
}

bool PhraseLargeTable3::store(FILE * new_index, MemoryChunk * new_content) {
    int retval = trie_fwrite(m_index, new_index);
    if (retval)
        return false;
    new_content->set_content(0, m_content->begin(), m_content->size());
    return true;
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
