/* 
 *  libpinyin
 *  Library to deal with pinyin.
 *  
 *  Copyright (C) 2011 Peng Wu <alexepico@gmail.com>
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

#ifndef CHEWING_KEY_H
#define CHEWING_KEY_H

/** @file chewing_key.h
 *  @brief the definitions of chewing key related classes and structs.
 */

namespace pinyin{
/**
 * @brief enums of chewing initial element.
 */
enum ChewingInitial
{
    CHEWING_ZERO_INITIAL = 0,  /* zero initial. */
    CHEWING_B  = 1,            /* "ㄅ". */
    CHEWING_C  = 2,            /* "ㄘ". */
    CHEWING_CH = 3,            /* "ㄔ". */
    CHEWING_D  = 4,            /* "ㄉ". */
    CHEWING_F  = 5,            /* "ㄈ". */
    CHEWING_H  = 6,            /* "ㄏ". */
    CHEWING_G  = 7,            /* "ㄍ". */
    CHEWING_K  = 8,            /* "ㄎ". */
    CHEWING_J  = 9,            /* "ㄐ". */
    CHEWING_M  = 10,           /* "ㄇ". */
    CHEWING_N  = 11,           /* "ㄋ". */
    CHEWING_L  = 12,           /* "ㄌ". */
    CHEWING_R  = 13,           /* "ㄖ". */
    CHEWING_P  = 14,           /* "ㄆ". */
    CHEWING_Q  = 15,           /* "ㄑ". */
    CHEWING_S  = 16,           /* "ㄙ". */
    CHEWING_SH = 17,           /* "ㄕ". */
    CHEWING_T  = 18,           /* "ㄊ". */
    PINYIN_W   = 19,           /* Invalid Chewing. */
    CHEWING_X  = 20,           /* "ㄒ". */
    PINYIN_Y   = 21,           /* Invalid Chewing. */
    CHEWING_Z  = 22,           /* "ㄗ". */
    CHEWING_ZH = 23,           /* "ㄓ". */
    CHEWING_LAST_INITIAL = CHEWING_ZH,
    CHEWING_NUMBER_OF_INITIALS = CHEWING_LAST_INITIAL + 1
};


/**
 * @brief enums of chewing middle element.
 */

enum ChewingMiddle
{
    CHEWING_ZERO_MIDDLE = 0,   /* zero middle. */
    CHEWING_I  = 1,            /* "ㄧ". */
    CHEWING_U  = 2,            /* "ㄨ". */
    CHEWING_V  = 3,            /* "ㄩ". */
    CHEWING_LAST_MIDDLE = CHEWING_V,
    CHEWING_NUMBER_OF_MIDDLES = CHEWING_LAST_MIDDLE + 1
};

/**
 * @brief enums of chewing final element.
 */
enum ChewingFinal
{
    CHEWING_ZERO_FINAL = 0,   /* zero final */
    CHEWING_A    = 1,         /* "ㄚ". */
    CHEWING_AI   = 2,         /* "ㄞ". */
    CHEWING_AN   = 3,         /* "ㄢ". */
    CHEWING_ANG  = 4,         /* "ㄤ". */
    CHEWING_AO   = 5,         /* "ㄠ". */
    CHEWING_E    = 6,         /* "ㄝ" and "ㄜ". */
    INVALID_EA   = 7,         /* Invalid Pinyin/Chewing. */
    CHEWING_EI   = 8,         /* "ㄟ". */
    CHEWING_EN   = 9,         /* "ㄣ". */
    CHEWING_ENG  = 10,        /* "ㄥ". */
    CHEWING_ER   = 11,        /* "ㄦ". */
    CHEWING_NG   = 12,        /* "ㄫ". */
    CHEWING_O    = 13,        /* "ㄛ". */
    PINYIN_ONG   = 14,        /* "ueng". */
    CHEWING_OU   = 15,        /* "ㄡ". */
    PINYIN_IN    = 16,        /* "ien". */
    PINYIN_ING   = 17,        /* "ieng". */
    CHEWING_LAST_FINAL = PINYIN_ING,
    CHEWING_NUMBER_OF_FINALS = CHEWING_LAST_FINAL + 1
};


/**
 * @brief enums of chewing tone element.
 */
enum ChewingTone
{
    CHEWING_ZERO_TONE = 0,     /* zero tone. */
    CHEWING_1  = 1,
    CHEWING_2  = 2,
    CHEWING_3  = 3,
    CHEWING_4  = 4,
    CHEWING_5  = 5,
    CHEWING_LAST_TONE = CHEWING_5,
    CHEWING_NUMBER_OF_TONES = CHEWING_LAST_TONE + 1
};

struct ChewingKey
{
    guint16 m_initial : 5;
    guint16 m_middle  : 2;
    guint16 m_final   : 5;
    guint16 m_tone    : 3;
};

};

#endif
