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


#include <glib.h>


G_BEGIN_DECLS


/**
 * PinyinStandardOption: type pinyin_standard_option_t.
 *
 * The enums of pinyin standard option.
 *
 */
enum PinyinStandardOption{
    IS_PINYIN = 1U << 1,
    IS_ZHUYIN = 1U << 2,
    PINYIN_INCOMPLETE = 1U << 3,
    ZHUYIN_INCOMPLETE = 1U << 4,
    USE_TONE = 1U << 5,
    FORCE_TONE = 1U << 6,
    USE_DIVIDED_TABLE = 1U << 7,
    USE_RESPLIT_TABLE = 1U << 8,
    DYNAMIC_ADJUST = 1U << 9,
    FULL_PINYIN_SUPPORT_QUOTATION = 1U << 10,
};

/**
 * PinyinFuzzyOption: type pinyin_fuzzy_option_t.
 *
 * The enums of pinyin fuzzy option.
 *
 */
enum PinyinFuzzyOption{
    PINYIN_FUZZY_C_CH = 1U << 10,
    PINYIN_FUZZY_S_SH = 1U << 11,
    PINYIN_FUZZY_Z_ZH = 1U << 12,
    PINYIN_FUZZY_F_H = 1U << 13,
    PINYIN_FUZZY_G_K = 1U << 14,
    PINYIN_FUZZY_L_N = 1U << 15,
    PINYIN_FUZZY_L_R = 1U << 16,
    PINYIN_FUZZY_AN_ANG = 1U << 17,
    PINYIN_FUZZY_EN_ENG = 1U << 18,
    PINYIN_FUZZY_IN_ING = 1U << 19,
    PINYIN_FUZZY_ALL = 0x3FFU << 10
};

/**
 * PinyinCorrectOption: type pinyin_correct_option_t.
 *
 * The enums of pinyin correct option.
 *
 */
enum PinyinCorrectOption{
    PINYIN_CORRECT_GN_NG = 1U << 21,
    PINYIN_CORRECT_MG_NG = 1U << 22,
    PINYIN_CORRECT_IOU_IU = 1U << 23,
    PINYIN_CORRECT_UEI_UI = 1U << 24,
    PINYIN_CORRECT_UEN_UN = 1U << 25,
    PINYIN_CORRECT_UE_VE = 1U << 26,
    PINYIN_CORRECT_V_U = 1U << 27,
    PINYIN_CORRECT_ON_ONG = 1U << 28,
    PINYIN_CORRECT_ALL = 0xFFU << 21,
};

/**
 * ZhuyinCorrectOption: type pinyin_correct_option_t.
 *
 * The enums of zhuyin correct option.
 *
 */
enum ZhuyinCorrectOption{
    ZHUYIN_CORRECT_HSU = 1U << 29,
    ZHUYIN_CORRECT_ETEN26 = 1U << 30,
    ZHUYIN_CORRECT_SHUFFLE = 1U << 31,
    ZHUYIN_CORRECT_ALL = 0x7U << 29
};

/**
 * @brief enums of Full Pinyin Schemes.
 */
enum FullPinyinScheme
{
    FULL_PINYIN_HANYU = 1,
    FULL_PINYIN_LUOMA = 2,
    FULL_PINYIN_SECONDARY_BOPOMOFO = 3,
    FULL_PINYIN_DEFAULT = FULL_PINYIN_HANYU
};

/**
 * @brief enums of Double Pinyin Schemes.
 */
enum DoublePinyinScheme
{
    DOUBLE_PINYIN_ZRM        = 1,
    DOUBLE_PINYIN_MS         = 2,
    DOUBLE_PINYIN_ZIGUANG    = 3,
    DOUBLE_PINYIN_ABC        = 4,
    DOUBLE_PINYIN_PYJJ       = 5,
    DOUBLE_PINYIN_XHE        = 6,
    DOUBLE_PINYIN_CUSTOMIZED = 30,        /* for user's keyboard */
    DOUBLE_PINYIN_DEFAULT    = DOUBLE_PINYIN_MS
};

/**
 * @brief enums of Zhuyin Schemes.
 */
enum ZhuyinScheme
{
    ZHUYIN_STANDARD = 1,
    ZHUYIN_HSU      = 2,
    ZHUYIN_IBM      = 3,
    ZHUYIN_GINYIEH  = 4,
    ZHUYIN_ETEN     = 5,
    ZHUYIN_ETEN26   = 6,
    ZHUYIN_STANDARD_DVORAK = 7,
    ZHUYIN_HSU_DVORAK = 8,
    ZHUYIN_DACHEN_CP26 = 9,
    ZHUYIN_DEFAULT  = ZHUYIN_STANDARD
};


G_END_DECLS
