/* 
 *  libpinyin
 *  Library to deal with pinyin.
 *  
 *  Copyright (C) 2011 Peng Wu <alexepico@gmail.com>
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

#ifndef PINYIN_PARSER2_H
#define PINYIN_PARSER2_H

#include <glib.h>
#include "novel_types.h"
#include "chewing_key.h"
#include "pinyin_custom2.h"

namespace pinyin{

typedef struct {
    const char * m_pinyin_str;
    const char * m_shengmu_str;
    const char * m_yunmu_str;
    const char * m_zhuyin_str;
    const char * m_luoma_pinyin_str;
    const char * m_secondary_zhuyin_str;
    ChewingKey   m_chewing_key;
} content_table_item_t;

typedef struct {
    const char * m_pinyin_input;
    guint32      m_flags;
    guint16      m_table_index;
    guint16      m_distance;
} pinyin_index_item_t;

typedef struct {
    const char * m_chewing_input;
    guint32      m_flags;
    guint16      m_table_index;
} chewing_index_item_t;

typedef struct {
    const char * m_orig_key;
    ChewingKey m_orig_struct;
    guint32      m_orig_freq;
    const char * m_new_keys[2];
    ChewingKey m_new_structs[2];
    guint32      m_new_freq;
} divided_table_item_t;

typedef struct {
    const char * m_orig_keys[2];
    ChewingKey m_orig_structs[2];
    guint32      m_orig_freq;
    const char * m_new_keys[2];
    ChewingKey m_new_structs[2];
    guint32      m_new_freq;
} resplit_table_item_t;

typedef struct {
    const char * m_shengmu;
} double_pinyin_scheme_shengmu_item_t;

typedef struct {
    const char * m_yunmus[2];
} double_pinyin_scheme_yunmu_item_t;

typedef struct {
    const char * m_input;
    const char * m_yunmu;
} double_pinyin_scheme_fallback_item_t;


typedef GArray * ParseValueVector;


/**
 * PhoneticParser2:
 *
 * Parse the ascii string into an array of the struct ChewingKeys.
 *
 */
class PhoneticParser2
{
public:
    /**
     * PhoneticParser2::~PhoneticParser2:
     *
     * The destructor of the PhoneticParser2.
     *
     */
    virtual ~PhoneticParser2() {}

public:
    /**
     * PhoneticParser2::parse_one_key:
     * @options: the pinyin options from pinyin_custom2.h.
     * @key: the parsed result of struct ChewingKey.
     * @str: the input of the ascii string.
     * @len: the length of the str.
     * @returns: whether the entire string is parsed as one key.
     *
     * Parse only one struct ChewingKey from a string.
     *
     */
    virtual bool parse_one_key(pinyin_option_t options, ChewingKey & key, gint16 & distance, const char *str, int len) const = 0;

    /**
     * PhoneticParser2::parse:
     * @options: the pinyin options from pinyin_custom2.h.
     * @keys: the parsed result of struct ChewingKeys.
     * @str: the input of the ascii string.
     * @len: the length of the str.
     * @returns: the number of chars were actually used.
     *
     * Parse the ascii string into an array of struct ChewingKeys.
     *
     */
    virtual int parse(pinyin_option_t options, ChewingKeyVector & keys, ChewingKeyRestVector & key_rests, const char *str, int len) const = 0;

};


/**
 * FullPinyinParser2:
 *
 * Parses the full pinyin string into an array of struct ChewingKeys.
 *
 */
class FullPinyinParser2 : public PhoneticParser2
{
    /* Note: some internal pointers to full pinyin table. */
    const pinyin_index_item_t * m_pinyin_index;
    size_t m_pinyin_index_len;

protected:
    ParseValueVector m_parse_steps;

    int final_step(size_t step_len, ChewingKeyVector & keys,
                   ChewingKeyRestVector & key_rests) const;

public:
    FullPinyinParser2();
    virtual ~FullPinyinParser2() {
        g_array_free(m_parse_steps, TRUE);
    }

    virtual bool parse_one_key(pinyin_option_t options, ChewingKey & key, gint16 & distance, const char *str, int len) const;

    /* Note:
     *   the parse method will use dynamic programming to drive parse_one_key.
     */
    virtual int parse(pinyin_option_t options, ChewingKeyVector & keys, ChewingKeyRestVector & key_rests, const char *str, int len) const;

public:
    bool set_scheme(FullPinyinScheme scheme);
};


/**
 * DoublePinyinParser2:
 *
 * Parse the double pinyin string into an array of struct ChewingKeys.
 *
 */
/* The valid input chars of double pinyin is a-z and ';'
 */
class DoublePinyinParser2 : public PhoneticParser2
{
    /* Note: three internal pointers to double pinyin scheme table. */
protected:
    const double_pinyin_scheme_shengmu_item_t  * m_shengmu_table;
    const double_pinyin_scheme_yunmu_item_t    * m_yunmu_table;
    const double_pinyin_scheme_fallback_item_t * m_fallback_table;

public:
    DoublePinyinParser2() {
        m_shengmu_table = NULL;
        m_yunmu_table = NULL;
        m_fallback_table = NULL;

        set_scheme(DOUBLE_PINYIN_DEFAULT);
    }

    virtual ~DoublePinyinParser2() {}

    virtual bool parse_one_key(pinyin_option_t options, ChewingKey & key, gint16 & distance, const char *str, int len) const;

    virtual int parse(pinyin_option_t options, ChewingKeyVector & keys, ChewingKeyRestVector & key_rests, const char *str, int len) const;

public:
    bool set_scheme(DoublePinyinScheme scheme);
};

/* Direct Parser for Pinyin table load. */
class PinyinDirectParser2 : public PhoneticParser2
{
    /* Only support Hanyu Pinyin now. */
    const pinyin_index_item_t * m_pinyin_index;
    size_t m_pinyin_index_len;

public:
    PinyinDirectParser2();

    virtual ~PinyinDirectParser2() {}

    virtual bool parse_one_key(pinyin_option_t options, ChewingKey & key, gint16 & distance, const char *str, int len) const;

    virtual int parse(pinyin_option_t options, ChewingKeyVector & keys, ChewingKeyRestVector & key_rests, const char *str, int len) const;
};


};

#endif
