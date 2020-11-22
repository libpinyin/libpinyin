/* 
 *  libpinyin
 *  Library to deal with pinyin.
 *  
 *  Copyright (C) 2016 Peng Wu <alexepico@gmail.com>
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

#include "chewing_large_table2.h"
#include "pinyin_parser2.h"
#include "zhuyin_parser2.h"

void ChewingLargeTable2::init_entries() {
    assert(NULL == m_entries);

    m_entries = g_ptr_array_new();
    /* NULL for the first pointer. */
    g_ptr_array_set_size(m_entries, MAX_PHRASE_LENGTH + 1);

#define CASE(len) case len:                         \
    {                                               \
        ChewingTableEntry<len> * entry =            \
            new ChewingTableEntry<len>;             \
        g_ptr_array_index(m_entries, len) = entry;  \
        break;                                      \
    }

    for (size_t i = 1; i < m_entries->len; i++) {
        switch(i) {
            CASE(1);
            CASE(2);
            CASE(3);
            CASE(4);
            CASE(5);
            CASE(6);
            CASE(7);
            CASE(8);
            CASE(9);
            CASE(10);
            CASE(11);
            CASE(12);
            CASE(13);
            CASE(14);
            CASE(15);
            CASE(16);
        default:
            assert(false);
        }
    }

#undef CASE

}

void ChewingLargeTable2::fini_entries() {
    assert(NULL != m_entries);

    assert(MAX_PHRASE_LENGTH + 1 == m_entries->len);

#define CASE(len) case len:                     \
    {                                           \
        ChewingTableEntry<len> * entry =        \
            (ChewingTableEntry<len> *)          \
            g_ptr_array_index(m_entries, len);  \
        delete entry;                           \
        break;                                  \
    }

    for (size_t i = 1; i < m_entries->len; i++) {
        switch(i) {
            CASE(1);
            CASE(2);
            CASE(3);
            CASE(4);
            CASE(5);
            CASE(6);
            CASE(7);
            CASE(8);
            CASE(9);
            CASE(10);
            CASE(11);
            CASE(12);
            CASE(13);
            CASE(14);
            CASE(15);
            CASE(16);
        default:
            assert(false);
        }
    }

#undef CASE

    g_ptr_array_free(m_entries, TRUE);
    m_entries = NULL;
}

/* load text method */
bool ChewingLargeTable2::load_text(FILE * infile, TABLE_PHONETIC_TYPE type) {
    char pinyin[256];
    char phrase[256];
    phrase_token_t token;
    size_t freq;

    while (!feof(infile)) {
#ifdef __APPLE__
        int num = fscanf(infile, "%255s %255[^ \t] %u %ld",
                         pinyin, phrase, &token, &freq);
#else
        int num = fscanf(infile, "%255s %255s %u %ld",
                         pinyin, phrase, &token, &freq);
#endif

        if (4 != num)
            continue;

        if(feof(infile))
            break;

        glong len = g_utf8_strlen(phrase, -1);

        ChewingKeyVector keys;
        ChewingKeyRestVector key_rests;

        keys = g_array_new(FALSE, FALSE, sizeof(ChewingKey));
        key_rests = g_array_new(FALSE, FALSE, sizeof(ChewingKeyRest));

        switch (type) {
        case PINYIN_TABLE: {
            PinyinDirectParser2 parser;
            pinyin_option_t options = USE_TONE;
            parser.parse(options, keys, key_rests, pinyin, strlen(pinyin));
            break;
        }

        case ZHUYIN_TABLE: {
            ZhuyinDirectParser2 parser;
            pinyin_option_t options = USE_TONE | FORCE_TONE;
            parser.parse(options, keys, key_rests, pinyin, strlen(pinyin));
            break;
        }
        };

        if (len != keys->len) {
            fprintf(stderr, "ChewingLargeTable2::load_text:%s\t%s\t%u\t%ld\n",
                    pinyin, phrase, token, freq);
            continue;
        }

        add_index(keys->len, (ChewingKey *)keys->data, token);

        g_array_free(keys, TRUE);
        g_array_free(key_rests, TRUE);
    }

    return true;
}

/* search method */
int ChewingLargeTable2::search(int phrase_length,
                               /* in */ const ChewingKey keys[],
                               /* out */ PhraseIndexRanges ranges) const {
    ChewingKey index[MAX_PHRASE_LENGTH];
    assert(NULL != m_db);

    if (contains_incomplete_pinyin(keys, phrase_length)) {
        compute_incomplete_chewing_index(keys, index, phrase_length);
        return search_internal(phrase_length, index, keys, ranges);
    } else {
        compute_chewing_index(keys, index, phrase_length);
        return search_internal(phrase_length, index, keys, ranges);
    }

    return SEARCH_NONE;
}

/* add/remove index method */
int ChewingLargeTable2::add_index(int phrase_length,
                                  /* in */ const ChewingKey keys[],
                                  /* in */ phrase_token_t token) {
    ChewingKey index[MAX_PHRASE_LENGTH];
    assert(NULL != m_db);
    int result = ERROR_OK;

    /* for in-complete chewing index */
    compute_incomplete_chewing_index(keys, index, phrase_length);
    result = add_index_internal(phrase_length, index, keys, token);
    assert(ERROR_OK == result || ERROR_INSERT_ITEM_EXISTS == result);
    if (ERROR_OK != result)
        return result;

    /* for chewing index */
    compute_chewing_index(keys, index, phrase_length);
    result = add_index_internal(phrase_length, index, keys, token);
    assert(ERROR_OK == result || ERROR_INSERT_ITEM_EXISTS == result);
    return result;
}

int ChewingLargeTable2::remove_index(int phrase_length,
                                     /* in */ const ChewingKey keys[],
                                     /* in */ phrase_token_t token) {
    ChewingKey index[MAX_PHRASE_LENGTH];
    assert(NULL != m_db);
    int result = ERROR_OK;

    /* for in-complete chewing index */
    compute_incomplete_chewing_index(keys, index, phrase_length);
    result = remove_index_internal(phrase_length, index, keys, token);
    assert(ERROR_OK == result || ERROR_REMOVE_ITEM_DONOT_EXISTS == result);
    if (ERROR_OK != result)
        return result;

    /* for chewing index */
    compute_chewing_index(keys, index, phrase_length);
    result = remove_index_internal(phrase_length, index, keys, token);
    assert(ERROR_OK == result || ERROR_REMOVE_ITEM_DONOT_EXISTS == result);
    return result;
}
