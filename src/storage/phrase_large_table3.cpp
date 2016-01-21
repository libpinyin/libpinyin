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

/**
 * Data Structure:
 * m_chunk consists of table entry header and array of tokens.
 */

class PhraseTableEntry{
    friend class PhraseLargeTable3;
protected:
    MemoryChunk m_chunk;

private:
    /* Disallow used outside. */
    PhraseTableEntry() {}

public:
    /* search method */
    int search(/* in */ const ucs4_t phrase[], /* out */ PhraseTokens tokens) const;

    /* add_index/remove_index method */
    int add_index(/* in */ const ucs4_t phrase[], /* in */ phrase_token_t token);
    int remove_index(/* in */ const ucs4_t phrase[], /* in */ phrase_token_t token);

    /* get length method */
    int get_length() const;

    /* mask out method */
    bool mask_out(phrase_token_t mask, phrase_token_t value);
};

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
