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

#ifndef ZHUYIN_PARSER2_H
#define ZHUYIN_PARSER2_H

#include <glib.h>
#include "novel_types.h"
#include "chewing_key.h"
#include "pinyin_custom2.h"
#include "pinyin_parser2.h"

namespace pinyin{

typedef struct {
    const char m_input;
    const char * m_chewing;
} zhuyin_symbol_item_t;

typedef struct {
    const char m_input;
    const char m_tone;
} zhuyin_tone_item_t;


/**
 * ZhuyinParser2:
 *
 * Parse the chewing input string into an array of struct ChewingKeys.
 *
 */
class ZhuyinParser2 : public PhoneticParser2
{
public:
    virtual ~ZhuyinParser2() {}

public:
    /**
     * ZhuyinParser2::in_chewing_scheme:
     * @options: the pinyin options.
     * @key: the user input ascii character.
     * @symbol: the corresponding chewing symbol.
     * @returns: whether the character is in the chewing scheme.
     *
     * Check whether the input character is in the chewing keyboard mapping.
     *
     */
    virtual bool in_chewing_scheme(pinyin_option_t options, const char key, gchar ** & symbols) const = 0;
};


 /**
 * ZhuyinSimpleParser2:
 *
 * Parse the chewing string into an array of struct ChewingKeys.
 *
 * Several keyboard scheme are supported:
 * * ZHUYIN_STANDARD  Standard ZhuYin keyboard, which maps 1 to Bo(ㄅ), q to Po(ㄆ) etc.
 * * ZHUYIN_IBM       IBM ZhuYin keyboard, which maps 1 to Bo(ㄅ), 2 to Po(ㄆ) etc.
 * * ZHUYIN_GINYIEH   Gin-Yieh ZhuYin keyboard.
 * * ZHUYIN_ETEN      Eten (倚天) ZhuYin keyboard.
 * * ZHUYIN_STANDARD_DVORAK      Standard Dvorak ZhuYin keyboard
 *
 */

class ZhuyinSimpleParser2 : public ZhuyinParser2
{
    /* internal options for chewing parsing. */
    pinyin_option_t m_options;

    /* Note: some internal pointers to chewing scheme table. */
protected:
    const zhuyin_symbol_item_t * m_symbol_table;
    const zhuyin_tone_item_t   * m_tone_table;

public:
    ZhuyinSimpleParser2() {
        m_symbol_table = NULL; m_tone_table = NULL;
        set_scheme(ZHUYIN_DEFAULT);
    }

    virtual ~ZhuyinSimpleParser2() {}

    virtual bool parse_one_key(pinyin_option_t options, ChewingKey & key, gint16 & distance, const char *str, int len) const;

    virtual int parse(pinyin_option_t options, ChewingKeyVector & keys, ChewingKeyRestVector & key_rests, const char *str, int len) const;

public:
    bool set_scheme(ZhuyinScheme scheme);
    virtual bool in_chewing_scheme(pinyin_option_t options, const char key, gchar ** & symbols) const;
};


/**
 * ZhuyinDiscreteParser2:
 *
 * Parse the chewing string into an array of struct ChewingKeys.
 *
 * Initially will support HSU, HSU Dvorak and ETEN26.
 *
 */

class ZhuyinDiscreteParser2 : public ZhuyinParser2
{
protected:
    /* internal options for chewing parsing. */
    pinyin_option_t m_options;

    /* some internal pointers to chewing scheme table. */
    const chewing_index_item_t * m_chewing_index;
    size_t m_chewing_index_len;
    const zhuyin_symbol_item_t * m_initial_table;
    const zhuyin_symbol_item_t * m_middle_table;
    const zhuyin_symbol_item_t * m_final_table;
    const zhuyin_tone_item_t   * m_tone_table;

public:
    ZhuyinDiscreteParser2() {
        m_options = 0;
        m_chewing_index = NULL; m_chewing_index_len = 0;
        m_initial_table = NULL; m_middle_table = NULL;
        m_final_table   = NULL; m_tone_table = NULL;
        set_scheme(ZHUYIN_HSU);
    }

    virtual ~ZhuyinDiscreteParser2() {}

    virtual bool parse_one_key(pinyin_option_t options, ChewingKey & key, gint16 & distance, const char *str, int len) const;

    virtual int parse(pinyin_option_t options, ChewingKeyVector & keys, ChewingKeyRestVector & key_rests, const char *str, int len) const;

public:
    bool set_scheme(ZhuyinScheme scheme);
    virtual bool in_chewing_scheme(pinyin_option_t options, const char key, gchar ** & symbols) const;
};


class ZhuyinDaChenCP26Parser2 : public ZhuyinParser2
{
    /* some internal pointers to chewing scheme table. */
    const chewing_index_item_t * m_chewing_index;
    size_t m_chewing_index_len;
    const zhuyin_symbol_item_t * m_initial_table;
    const zhuyin_symbol_item_t * m_middle_table;
    const zhuyin_symbol_item_t * m_final_table;
    const zhuyin_tone_item_t   * m_tone_table;

public:
    ZhuyinDaChenCP26Parser2();

    virtual ~ZhuyinDaChenCP26Parser2() {}

    virtual bool parse_one_key(pinyin_option_t options, ChewingKey & key, gint16 & distance, const char *str, int len) const;

    virtual int parse(pinyin_option_t options, ChewingKeyVector & keys, ChewingKeyRestVector & key_rests, const char *str, int len) const;

public:
    virtual bool in_chewing_scheme(pinyin_option_t options, const char key, gchar ** & symbols) const;
};


/* Direct Parser for Zhuyin table load. */
class ZhuyinDirectParser2 : public PhoneticParser2
{
    const chewing_index_item_t * m_chewing_index;
    size_t m_chewing_index_len;

public:
    ZhuyinDirectParser2();

    virtual ~ZhuyinDirectParser2() {}

    virtual bool parse_one_key(pinyin_option_t options, ChewingKey & key, gint16 & distance, const char *str, int len) const;

    virtual int parse(pinyin_option_t options, ChewingKeyVector & keys, ChewingKeyRestVector & key_rests, const char *str, int len) const;
};


};

#endif
