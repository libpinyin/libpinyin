/* 
 *  libpinyin
 *  Library to deal with pinyin.
 *  
 *  Copyright (C) 2002,2003,2006 James Su
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

#include "stl_lite.h"
#include "novel_types.h"
#include "pinyin_base.h"
#include "pinyin_phrase.h"
#include "pinyin_large_table.h"

using namespace pinyin;

// Internal data definition

/**
 * struct of pinyin token.
 *
 * this struct store the informations of a pinyin token
 * (an initial or final)
 */
struct PinyinToken
{
    const char *latin;      /**< Latin name of the token. */
    const char *zhuyin;     /**< Zhuyin name in UTF-8. */
    int   latin_len;  /**< length of Latin name. */
    int   zhuyin_len; /**< length of Chinese name. */
};

/**
 * struct to index PinyinToken list.
 */
struct PinyinTokenIndex
{
    int start;
    int num;
};

static const PinyinToken __pinyin_initials[] =
{
    {"", "", 0, 0},
    {"b", "ㄅ", 1, 1},
    {"c", "ㄘ", 1, 1},
    {"ch","ㄔ", 2, 1},
    {"d", "ㄉ", 1, 1},
    {"f", "ㄈ", 1, 1},
    {"h", "ㄏ", 1, 1},
    {"g", "ㄍ", 1, 1},
    {"k", "ㄎ", 1, 1},
    {"j", "ㄐ", 1, 1},
    {"m", "ㄇ", 1, 1},
    {"n", "ㄋ", 1, 1},
    {"l", "ㄌ", 1, 1},
    {"r", "ㄖ", 1, 1},
    {"p", "ㄆ", 1, 1},
    {"q", "ㄑ", 1, 1},
    {"s", "ㄙ", 1, 1},
    {"sh","ㄕ", 2, 1},
    {"t", "ㄊ", 1, 1},
    {"w", "ㄨ", 1, 1},  //Should be omitted in some case.
    {"x", "ㄒ", 1, 1},
    {"y", "ㄧ", 1, 1},  //Should be omitted in some case.
    {"z", "ㄗ", 1, 1},
    {"zh","ㄓ", 2, 1}
};

static const PinyinToken __pinyin_finals[] =
{
    {"", "", 0, 0},
    {"a",   "ㄚ",   1, 1},
    {"ai",  "ㄞ",   2, 1},
    {"an",  "ㄢ",   2, 1},
    {"ang", "ㄤ",   3, 1},
    {"ao",  "ㄠ",   2, 1},
    {"e",   "ㄜ",   1, 1},
    {"ea",  "ㄝ",   2, 1},
    {"ei",  "ㄟ",   2, 1},
    {"en",  "ㄣ",   2, 1},
    {"eng", "ㄥ",   3, 1},
    {"er",  "ㄦ",   2, 1},
    {"i",   "ㄧ",   1, 1},
    {"ia",  "ㄧㄚ", 2, 2},
    {"ian", "ㄧㄢ", 3, 2},
    {"iang","ㄧㄤ", 4, 2},
    {"iao", "ㄧㄠ", 3, 2},
    {"ie",  "ㄧㄝ", 2, 2},
    {"in",  "ㄧㄣ", 2, 2},
    {"ing", "ㄧㄥ", 3, 2},
    {"iong","ㄩㄥ", 4, 2},
    {"iu",  "ㄧㄡ", 2, 2},
    {"ng",  "ㄣ",   2, 1},
    {"o",   "ㄛ",   1, 1},
    {"ong", "ㄨㄥ", 3, 2},
    {"ou",  "ㄡ",   2, 1},
    {"u",   "ㄨ",   1, 1},
    {"ua",  "ㄨㄚ", 2, 2},
    {"uai", "ㄨㄞ", 3, 2},
    {"uan", "ㄨㄢ", 3, 2},
    {"uang","ㄨㄤ", 4, 2},
    {"ue",  "ㄩㄝ", 2, 2},
    {"ueng","ㄨㄥ", 4, 2},
    {"ui",  "ㄨㄟ", 2, 2},
    {"un",  "ㄨㄣ", 2, 2},
    {"uo",  "ㄨㄛ", 2, 2},
    {"v",   "ㄩ",   1, 1},
    {"van", "ㄩㄢ", 3, 2},
    {"ve",  "ㄩㄝ", 2, 2},
    {"vn",  "ㄩㄣ", 2, 2}
};

static const PinyinToken __pinyin_tones [] =
{
    {"", "", 0, 0},
    {"1", "ˉ", 1, 1},
    {"2", "ˊ", 1, 1},
    {"3", "ˇ", 1, 1},
    {"4", "ˋ", 1, 1},
    {"5", "˙", 1, 1}
};

static const PinyinTokenIndex __pinyin_initials_index[] =
{
    //a     b      c      d     e       f      g      h      i      j      k      l      m 
    {-1,0},{1,1}, {2,2}, {4,1}, {-1,0},{5,1}, {7,1}, {6,1}, {-1,0},{9,1}, {8,1}, {12,1},{10,1},
    //n     o      p      q      r      s      t      u      v      w      x      y      z
    {11,1},{-1,0},{14,1},{15,1},{13,1},{16,2},{18,1},{-1,0},{-1,0},{19,1},{20,1},{21,1},{22,2}
};

static const PinyinTokenIndex __pinyin_finals_index[] =
{
    //a     b      c      d      e     f      g      h      i       j      k      l      m 
    {1,5}, {-1,0},{-1,0},{-1,0},{6,6},{-1,0},{-1,0},{-1,0},{12,10},{-1,0},{-1,0},{-1,0},{-1,0},
    //n     o      p      q      r      s      t      u      v      w      x      y      z
    {22,1},{23,3},{-1,0},{-1,0},{-1,0},{-1,0},{-1,0},{26,10},{36,4},{-1,0},{-1,0},{-1,0},{-1,0}
};

#if 0

static const PinyinInitial __shuang_pin_stone_initial_map [] =
{
    PINYIN_ZeroInitial,    // A
    PINYIN_Bo,             // B
    PINYIN_Ci,             // C
    PINYIN_De,             // D
    PINYIN_ZeroInitial,    // E
    PINYIN_Fo,             // F
    PINYIN_Ge,             // G
    PINYIN_He,             // H
    PINYIN_Shi,            // I
    PINYIN_Ji,             // J
    PINYIN_Ke,             // K
    PINYIN_Le,             // L
    PINYIN_Mo,             // M
    PINYIN_Ne,             // N
    PINYIN_ZeroInitial,    // O
    PINYIN_Po,             // P
    PINYIN_Qi,             // Q
    PINYIN_Ri,             // R
    PINYIN_Si,             // S
    PINYIN_Te,             // T
    PINYIN_Chi,            // U
    PINYIN_Zhi,            // V
    PINYIN_Wu,             // W
    PINYIN_Xi,             // X
    PINYIN_Yi,             // Y
    PINYIN_Zi,             // Z
    PINYIN_ZeroInitial,    // ;
};

static const PinyinFinal __shuang_pin_stone_final_map [][2] =
{
    { PINYIN_A,   PINYIN_ZeroFinal },         // A
    { PINYIN_Ia,  PINYIN_Ua        },         // B
    { PINYIN_Uan, PINYIN_ZeroFinal },         // C
    { PINYIN_Ao,  PINYIN_ZeroFinal },         // D
    { PINYIN_E,   PINYIN_ZeroFinal },         // E
    { PINYIN_An,  PINYIN_ZeroFinal },         // F
    { PINYIN_Ang, PINYIN_ZeroFinal },         // G
    { PINYIN_Uang,PINYIN_Iang      },         // H
    { PINYIN_I,   PINYIN_ZeroFinal },         // I
    { PINYIN_Ian, PINYIN_ZeroFinal },         // J
    { PINYIN_Iao, PINYIN_ZeroFinal },         // K
    { PINYIN_In,  PINYIN_ZeroFinal },         // L
    { PINYIN_Ie,  PINYIN_ZeroFinal },         // M
    { PINYIN_Iu,  PINYIN_ZeroFinal },         // N
    { PINYIN_Uo,  PINYIN_O         },         // O
    { PINYIN_Ou,  PINYIN_ZeroFinal },         // P
    { PINYIN_Ing, PINYIN_Er        },         // Q
    { PINYIN_En,  PINYIN_ZeroFinal },         // R
    { PINYIN_Ai,  PINYIN_ZeroFinal },         // S
    { PINYIN_Ng,  PINYIN_Eng       },         // T
    { PINYIN_U,   PINYIN_ZeroFinal },         // U
    { PINYIN_V,   PINYIN_Ui        },         // V
    { PINYIN_Ei,  PINYIN_ZeroFinal },         // W
    { PINYIN_Uai, PINYIN_Ue        },         // X
    { PINYIN_Ong, PINYIN_Iong      },         // Y
    { PINYIN_Un,  PINYIN_ZeroFinal },         // Z
    { PINYIN_ZeroFinal, PINYIN_ZeroFinal },   // ;
};

#endif

static const PinyinInitial __shuang_pin_zrm_initial_map [] =
{
    PINYIN_ZeroInitial,    // A
    PINYIN_Bo,             // B
    PINYIN_Ci,             // C
    PINYIN_De,             // D
    PINYIN_ZeroInitial,    // E
    PINYIN_Fo,             // F
    PINYIN_Ge,             // G
    PINYIN_He,             // H
    PINYIN_Chi,            // I
    PINYIN_Ji,             // J
    PINYIN_Ke,             // K
    PINYIN_Le,             // L
    PINYIN_Mo,             // M
    PINYIN_Ne,             // N
    PINYIN_ZeroInitial,    // O
    PINYIN_Po,             // P
    PINYIN_Qi,             // Q
    PINYIN_Ri,             // R
    PINYIN_Si,             // S
    PINYIN_Te,             // T
    PINYIN_Shi,            // U
    PINYIN_Zhi,            // V
    PINYIN_Wu,             // W
    PINYIN_Xi,             // X
    PINYIN_Yi,             // Y
    PINYIN_Zi,             // Z
    PINYIN_ZeroInitial,    // ;
};

static const PinyinFinal __shuang_pin_zrm_final_map [][2] =
{
    { PINYIN_A,   PINYIN_ZeroFinal },         // A
    { PINYIN_Ou,  PINYIN_ZeroFinal },         // B
    { PINYIN_Iao, PINYIN_ZeroFinal },         // C
    { PINYIN_Uang,PINYIN_Iang      },         // D
    { PINYIN_E,   PINYIN_ZeroFinal },         // E
    { PINYIN_En,  PINYIN_ZeroFinal },         // F
    { PINYIN_Ng,  PINYIN_Eng       },         // G
    { PINYIN_Ang, PINYIN_ZeroFinal },         // H
    { PINYIN_I,   PINYIN_ZeroFinal },         // I
    { PINYIN_An,  PINYIN_ZeroFinal },         // J
    { PINYIN_Ao,  PINYIN_ZeroFinal },         // K
    { PINYIN_Ai,  PINYIN_ZeroFinal },         // L
    { PINYIN_Ian, PINYIN_ZeroFinal },         // M
    { PINYIN_In,  PINYIN_ZeroFinal },         // N
    { PINYIN_Uo,  PINYIN_O         },         // O
    { PINYIN_Un,  PINYIN_ZeroFinal },         // P
    { PINYIN_Iu,  PINYIN_ZeroFinal },         // Q
    { PINYIN_Uan, PINYIN_Er        },         // R
    { PINYIN_Ong, PINYIN_Iong      },         // S
    { PINYIN_Ue,  PINYIN_ZeroFinal },         // T
    { PINYIN_U,   PINYIN_ZeroFinal },         // U
    { PINYIN_V,   PINYIN_Ui        },         // V
    { PINYIN_Ia,  PINYIN_Ua        },         // W
    { PINYIN_Ie,  PINYIN_ZeroFinal },         // X
    { PINYIN_Ing, PINYIN_Uai       },         // Y
    { PINYIN_Ei,  PINYIN_ZeroFinal },         // Z
    { PINYIN_ZeroFinal, PINYIN_ZeroFinal },   // ;
};


static const PinyinInitial __shuang_pin_ms_initial_map [] =
{
    PINYIN_ZeroInitial,    // A
    PINYIN_Bo,             // B
    PINYIN_Ci,             // C
    PINYIN_De,             // D
    PINYIN_ZeroInitial,    // E
    PINYIN_Fo,             // F
    PINYIN_Ge,             // G
    PINYIN_He,             // H
    PINYIN_Chi,            // I
    PINYIN_Ji,             // J
    PINYIN_Ke,             // K
    PINYIN_Le,             // L
    PINYIN_Mo,             // M
    PINYIN_Ne,             // N
    PINYIN_ZeroInitial,    // O
    PINYIN_Po,             // P
    PINYIN_Qi,             // Q
    PINYIN_Ri,             // R
    PINYIN_Si,             // S
    PINYIN_Te,             // T
    PINYIN_Shi,            // U
    PINYIN_Zhi,            // V
    PINYIN_Wu,             // W
    PINYIN_Xi,             // X
    PINYIN_Yi,             // Y
    PINYIN_Zi,             // Z
    PINYIN_ZeroInitial,    // ;
};

static const PinyinFinal __shuang_pin_ms_final_map [][2] =
{
    { PINYIN_A,   PINYIN_ZeroFinal },         // A
    { PINYIN_Ou,  PINYIN_ZeroFinal },         // B
    { PINYIN_Iao, PINYIN_ZeroFinal },         // C
    { PINYIN_Uang,PINYIN_Iang      },         // D
    { PINYIN_E,   PINYIN_ZeroFinal },         // E
    { PINYIN_En,  PINYIN_ZeroFinal },         // F
    { PINYIN_Ng,  PINYIN_Eng       },         // G
    { PINYIN_Ang, PINYIN_ZeroFinal },         // H
    { PINYIN_I,   PINYIN_ZeroFinal },         // I
    { PINYIN_An,  PINYIN_ZeroFinal },         // J
    { PINYIN_Ao,  PINYIN_ZeroFinal },         // K
    { PINYIN_Ai,  PINYIN_ZeroFinal },         // L
    { PINYIN_Ian, PINYIN_ZeroFinal },         // M
    { PINYIN_In,  PINYIN_ZeroFinal },         // N
    { PINYIN_Uo,  PINYIN_O         },         // O
    { PINYIN_Un,  PINYIN_ZeroFinal },         // P
    { PINYIN_Iu,  PINYIN_ZeroFinal },         // Q
    { PINYIN_Uan, PINYIN_Er        },         // R
    { PINYIN_Ong, PINYIN_Iong      },         // S
    { PINYIN_Ue,  PINYIN_ZeroFinal },         // T
    { PINYIN_U,   PINYIN_ZeroFinal },         // U
    { PINYIN_V,   PINYIN_Ui        },         // V
    { PINYIN_Ia,  PINYIN_Ua        },         // W
    { PINYIN_Ie,  PINYIN_ZeroFinal },         // X
    { PINYIN_Uai, PINYIN_V         },         // Y
    { PINYIN_Ei,  PINYIN_ZeroFinal },         // Z
    { PINYIN_Ing, PINYIN_ZeroFinal },         // ;
};


static const PinyinInitial __shuang_pin_ziguang_initial_map [] =
{
    PINYIN_Chi,            // A
    PINYIN_Bo,             // B
    PINYIN_Ci,             // C
    PINYIN_De,             // D
    PINYIN_ZeroInitial,    // E
    PINYIN_Fo,             // F
    PINYIN_Ge,             // G
    PINYIN_He,             // H
    PINYIN_Shi,            // I
    PINYIN_Ji,             // J
    PINYIN_Ke,             // K
    PINYIN_Le,             // L
    PINYIN_Mo,             // M
    PINYIN_Ne,             // N
    PINYIN_ZeroInitial,    // O
    PINYIN_Po,             // P
    PINYIN_Qi,             // Q
    PINYIN_Ri,             // R
    PINYIN_Si,             // S
    PINYIN_Te,             // T
    PINYIN_Zhi,            // U
    PINYIN_ZeroInitial,    // V
    PINYIN_Wu,             // W
    PINYIN_Xi,             // X
    PINYIN_Yi,             // Y
    PINYIN_Zi,             // Z
    PINYIN_ZeroInitial,    // ;
};

static const PinyinFinal __shuang_pin_ziguang_final_map [][2] =
{
    { PINYIN_A,   PINYIN_ZeroFinal },         // A
    { PINYIN_Iao, PINYIN_ZeroFinal },         // B
    { PINYIN_Ing, PINYIN_ZeroFinal },         // C
    { PINYIN_Ie,  PINYIN_ZeroFinal },         // D
    { PINYIN_E,   PINYIN_ZeroFinal },         // E
    { PINYIN_Ian, PINYIN_ZeroFinal },         // F
    { PINYIN_Uang,PINYIN_Iang      },         // G
    { PINYIN_Ong, PINYIN_Iong      },         // H
    { PINYIN_I,   PINYIN_ZeroFinal },         // I
    { PINYIN_Iu,  PINYIN_Er        },         // J
    { PINYIN_Ei,  PINYIN_ZeroFinal },         // K
    { PINYIN_Uan, PINYIN_ZeroFinal },         // L
    { PINYIN_Un,  PINYIN_ZeroFinal },         // M
    { PINYIN_Ui,  PINYIN_Ue        },         // N
    { PINYIN_Uo,  PINYIN_O         },         // O
    { PINYIN_Ai,  PINYIN_ZeroFinal },         // P
    { PINYIN_Ao,  PINYIN_ZeroFinal },         // Q
    { PINYIN_An,  PINYIN_ZeroFinal },         // R
    { PINYIN_Ang, PINYIN_ZeroFinal },         // S
    { PINYIN_Ng,  PINYIN_Eng       },         // T
    { PINYIN_U,   PINYIN_ZeroFinal },         // U
    { PINYIN_V,   PINYIN_ZeroFinal },         // V
    { PINYIN_En,  PINYIN_ZeroFinal },         // W
    { PINYIN_Ia,  PINYIN_Ua        },         // X
    { PINYIN_In,  PINYIN_Uai       },         // Y
    { PINYIN_Ou,  PINYIN_ZeroFinal },         // Z
    { PINYIN_ZeroFinal, PINYIN_ZeroFinal },   // ;
};


static const PinyinInitial __shuang_pin_abc_initial_map [] =
{
    PINYIN_Zhi,            // A
    PINYIN_Bo,             // B
    PINYIN_Ci,             // C
    PINYIN_De,             // D
    PINYIN_Chi,            // E
    PINYIN_Fo,             // F
    PINYIN_Ge,             // G
    PINYIN_He,             // H
    PINYIN_ZeroInitial,    // I
    PINYIN_Ji,             // J
    PINYIN_Ke,             // K
    PINYIN_Le,             // L
    PINYIN_Mo,             // M
    PINYIN_Ne,             // N
    PINYIN_ZeroInitial,    // O
    PINYIN_Po,             // P
    PINYIN_Qi,             // Q
    PINYIN_Ri,             // R
    PINYIN_Si,             // S
    PINYIN_Te,             // T
    PINYIN_ZeroInitial,    // U
    PINYIN_Shi,            // V
    PINYIN_Wu,             // W
    PINYIN_Xi,             // X
    PINYIN_Yi,             // Y
    PINYIN_Zi,             // Z
    PINYIN_ZeroInitial,    // ;
};

static const PinyinFinal __shuang_pin_abc_final_map [][2] =
{
    { PINYIN_A,   PINYIN_ZeroFinal },         // A
    { PINYIN_Ou,  PINYIN_ZeroFinal },         // B
    { PINYIN_In,  PINYIN_Uai       },         // C
    { PINYIN_Ia,  PINYIN_Ua        },         // D
    { PINYIN_E,   PINYIN_ZeroFinal },         // E
    { PINYIN_En,  PINYIN_ZeroFinal },         // F
    { PINYIN_Ng,  PINYIN_Eng       },         // G
    { PINYIN_Ang, PINYIN_ZeroFinal },         // H
    { PINYIN_I,   PINYIN_ZeroFinal },         // I
    { PINYIN_An,  PINYIN_ZeroFinal },         // J
    { PINYIN_Ao,  PINYIN_ZeroFinal },         // K
    { PINYIN_Ai,  PINYIN_ZeroFinal },         // L
    { PINYIN_Ui,  PINYIN_Ue        },         // M
    { PINYIN_Un,  PINYIN_ZeroFinal },         // N
    { PINYIN_Uo,  PINYIN_O         },         // O
    { PINYIN_Uan, PINYIN_ZeroFinal },         // P
    { PINYIN_Ei,  PINYIN_ZeroFinal },         // Q
    { PINYIN_Iu,  PINYIN_Er        },         // R
    { PINYIN_Ong, PINYIN_Iong      },         // S
    { PINYIN_Uang,PINYIN_Iang      },         // T
    { PINYIN_U,   PINYIN_ZeroFinal },         // U
    { PINYIN_V,   PINYIN_ZeroFinal },         // V
    { PINYIN_Ian, PINYIN_ZeroFinal },         // W
    { PINYIN_Ie,  PINYIN_ZeroFinal },         // X
    { PINYIN_Ing, PINYIN_ZeroFinal },         // Y
    { PINYIN_Iao, PINYIN_ZeroFinal },         // Z
    { PINYIN_ZeroFinal, PINYIN_ZeroFinal },   // ;
};

#if 0

static const PinyinInitial __shuang_pin_liushi_initial_map [] =
{
    PINYIN_ZeroInitial,    // A
    PINYIN_Bo,             // B
    PINYIN_Ci,             // C
    PINYIN_De,             // D
    PINYIN_ZeroInitial,    // E
    PINYIN_Fo,             // F
    PINYIN_Ge,             // G
    PINYIN_He,             // H
    PINYIN_Chi,            // I
    PINYIN_Ji,             // J
    PINYIN_Ke,             // K
    PINYIN_Le,             // L
    PINYIN_Mo,             // M
    PINYIN_Ne,             // N
    PINYIN_ZeroInitial,    // O
    PINYIN_Po,             // P
    PINYIN_Qi,             // Q
    PINYIN_Ri,             // R
    PINYIN_Si,             // S
    PINYIN_Te,             // T
    PINYIN_Shi,            // U
    PINYIN_Zhi,            // V
    PINYIN_Wu,             // W
    PINYIN_Xi,             // X
    PINYIN_Yi,             // Y
    PINYIN_Zi,             // Z
    PINYIN_ZeroInitial,    // ;
};

static const PinyinFinal __shuang_pin_liushi_final_map [][2] =
{
    { PINYIN_A,   PINYIN_ZeroFinal },         // A
    { PINYIN_Ao,  PINYIN_ZeroFinal },         // B
    { PINYIN_Ang, PINYIN_ZeroFinal },         // C
    { PINYIN_Uan, PINYIN_ZeroFinal },         // D
    { PINYIN_E,   PINYIN_ZeroFinal },         // E
    { PINYIN_An,  PINYIN_ZeroFinal },         // F
    { PINYIN_Ong, PINYIN_Iong      },         // G
    { PINYIN_Ui,  PINYIN_Ue        },         // H
    { PINYIN_I,   PINYIN_ZeroFinal },         // I
    { PINYIN_Ia,  PINYIN_Ua        },         // J
    { PINYIN_Un,  PINYIN_ZeroFinal },         // K
    { PINYIN_Iu,  PINYIN_ZeroFinal },         // L
    { PINYIN_In,  PINYIN_ZeroFinal },         // M
    { PINYIN_Uang,PINYIN_Iang      },         // N
    { PINYIN_Uo,  PINYIN_O         },         // O
    { PINYIN_Ng,  PINYIN_Eng       },         // P
    { PINYIN_Ing, PINYIN_ZeroFinal },         // Q
    { PINYIN_Ou,  PINYIN_Er        },         // R
    { PINYIN_Ai,  PINYIN_ZeroFinal },         // S
    { PINYIN_Ian, PINYIN_ZeroFinal },         // T
    { PINYIN_U,   PINYIN_ZeroFinal },         // U
    { PINYIN_V,   PINYIN_En        },         // V
    { PINYIN_Ei,  PINYIN_ZeroFinal },         // W
    { PINYIN_Ie,  PINYIN_ZeroFinal },         // X
    { PINYIN_Uai, PINYIN_ZeroFinal },         // Y
    { PINYIN_Iao, PINYIN_ZeroFinal },         // Z
    { PINYIN_ZeroFinal, PINYIN_ZeroFinal },   // ;
};

#endif

static const PinyinInitial __shuang_pin_pyjj_initial_map [] =
{
    PINYIN_ZeroInitial,    // A
    PINYIN_Bo,             // B
    PINYIN_Ci,             // C
    PINYIN_De,             // D
    PINYIN_ZeroInitial,    // E
    PINYIN_Fo,             // F
    PINYIN_Ge,             // G
    PINYIN_He,             // H
    PINYIN_Shi,            // I
    PINYIN_Ji,             // J
    PINYIN_Ke,             // K
    PINYIN_Le,             // L
    PINYIN_Mo,             // M
    PINYIN_Ne,             // N
    PINYIN_ZeroInitial,    // O
    PINYIN_Po,             // P
    PINYIN_Qi,             // Q
    PINYIN_Ri,             // R
    PINYIN_Si,             // S
    PINYIN_Te,             // T
    PINYIN_Chi,            // U
    PINYIN_Zhi,            // V
    PINYIN_Wu,             // W
    PINYIN_Xi,             // X
    PINYIN_Yi,             // Y
    PINYIN_Zi,             // Z
    PINYIN_ZeroInitial,    // ;
};

static const PinyinFinal __shuang_pin_pyjj_final_map [][2] =
{
    { PINYIN_A,   PINYIN_ZeroFinal },         // A
    { PINYIN_Ia,  PINYIN_Ua        },         // B
    { PINYIN_Uan, PINYIN_ZeroFinal },         // C
    { PINYIN_Ao,  PINYIN_ZeroFinal },         // D
    { PINYIN_E,   PINYIN_ZeroFinal },         // E
    { PINYIN_An,  PINYIN_ZeroFinal },         // F
    { PINYIN_Ang, PINYIN_ZeroFinal },         // G
    { PINYIN_Iang,PINYIN_Uang      },         // H
    { PINYIN_I,   PINYIN_ZeroFinal },         // I
    { PINYIN_Ian, PINYIN_ZeroFinal },         // J
    { PINYIN_Iao, PINYIN_ZeroFinal },         // K
    { PINYIN_In,  PINYIN_ZeroFinal },         // L
    { PINYIN_Ie,  PINYIN_ZeroFinal },         // M
    { PINYIN_Iu,  PINYIN_ZeroFinal },         // N
    { PINYIN_Uo,  PINYIN_O         },         // O
    { PINYIN_Ou,  PINYIN_ZeroFinal },         // P
    { PINYIN_Er,  PINYIN_Ing       },         // Q
    { PINYIN_En,  PINYIN_ZeroFinal },         // R
    { PINYIN_Ai,  PINYIN_ZeroFinal },         // S
    { PINYIN_Eng, PINYIN_Ng        },         // T
    { PINYIN_U,   PINYIN_ZeroFinal },         // U
    { PINYIN_V,   PINYIN_Ui        },         // V
    { PINYIN_Ei,  PINYIN_ZeroFinal },         // W
    { PINYIN_Uai, PINYIN_Ue        },         // X
    { PINYIN_Ong, PINYIN_Iong      },         // Y
    { PINYIN_Un,  PINYIN_ZeroFinal },         // Z
    { PINYIN_ZeroFinal, PINYIN_ZeroFinal },   // ;
};

static const PinyinInitial __shuang_pin_xhe_initial_map [] =
{
    PINYIN_ZeroInitial,    // A
    PINYIN_Bo,             // B
    PINYIN_Ci,             // C
    PINYIN_De,             // D
    PINYIN_ZeroInitial,    // E
    PINYIN_Fo,             // F
    PINYIN_Ge,             // G
    PINYIN_He,             // H
    PINYIN_Chi,            // I
    PINYIN_Ji,             // J
    PINYIN_Ke,             // K
    PINYIN_Le,             // L
    PINYIN_Mo,             // M
    PINYIN_Ne,             // N
    PINYIN_ZeroInitial,    // O
    PINYIN_Po,             // P
    PINYIN_Qi,             // Q
    PINYIN_Ri,             // R
    PINYIN_Si,             // S
    PINYIN_Te,             // T
    PINYIN_Shi,            // U
    PINYIN_Zhi,            // V
    PINYIN_Wu,             // W
    PINYIN_Xi,             // X
    PINYIN_Yi,             // Y
    PINYIN_Zi,             // Z
    PINYIN_ZeroInitial,    // ;
};

static const PinyinFinal __shuang_pin_xhe_final_map [][2] =
{
    { PINYIN_A,   PINYIN_ZeroFinal },         // A
    { PINYIN_In,  PINYIN_ZeroFinal },         // B
    { PINYIN_Ao,  PINYIN_ZeroFinal },         // C
    { PINYIN_Ai,  PINYIN_ZeroFinal },         // D
    { PINYIN_E,   PINYIN_ZeroFinal },         // E
    { PINYIN_En,  PINYIN_ZeroFinal },         // F
    { PINYIN_Eng, PINYIN_Ng        },         // G
    { PINYIN_Ang, PINYIN_ZeroFinal },         // H
    { PINYIN_I,   PINYIN_ZeroFinal },         // I
    { PINYIN_An,  PINYIN_ZeroFinal },         // J
    { PINYIN_Uai, PINYIN_Ing       },         // K
    { PINYIN_Iang,PINYIN_Uang      },         // L
    { PINYIN_Ian, PINYIN_ZeroFinal },         // M
    { PINYIN_Iao, PINYIN_ZeroFinal },         // N
    { PINYIN_Uo,  PINYIN_O         },         // O
    { PINYIN_Ie,  PINYIN_ZeroFinal },         // P
    { PINYIN_Iu,  PINYIN_ZeroFinal },         // Q
    { PINYIN_Uan, PINYIN_Er        },         // R
    { PINYIN_Ong, PINYIN_Iong      },         // S
    { PINYIN_Ue,  PINYIN_ZeroFinal },         // T
    { PINYIN_U,   PINYIN_ZeroFinal },         // U
    { PINYIN_V,   PINYIN_Ui        },         // V
    { PINYIN_Ei,  PINYIN_ZeroFinal },         // W
    { PINYIN_Ia,  PINYIN_Ua        },         // X
    { PINYIN_Un,  PINYIN_ZeroFinal },         // Y
    { PINYIN_Ou,  PINYIN_ZeroFinal },         // Z
    { PINYIN_ZeroFinal, PINYIN_ZeroFinal },   // ;
};



static const size_t    __zhuyin_zhuyin_map_start_char = 0x3105;
static const size_t    __zhuyin_zhuyin_map_tone_start_idx = 37;
static const PinyinKey __zhuyin_zhuyin_map [][3] = 
{
    {PinyinKey(PINYIN_Bo),PinyinKey(),PinyinKey()},
    {PinyinKey(PINYIN_Po),PinyinKey(),PinyinKey()},
    {PinyinKey(PINYIN_Mo),PinyinKey(),PinyinKey()},
    {PinyinKey(PINYIN_Fo),PinyinKey(),PinyinKey()},
    {PinyinKey(PINYIN_De),PinyinKey(),PinyinKey()},
    {PinyinKey(PINYIN_Te),PinyinKey(),PinyinKey()},
    {PinyinKey(PINYIN_Ne),PinyinKey(),PinyinKey()},
    {PinyinKey(PINYIN_Le),PinyinKey(),PinyinKey()},
    {PinyinKey(PINYIN_Ge),PinyinKey(),PinyinKey()},
    {PinyinKey(PINYIN_Ke),PinyinKey(),PinyinKey()},
    {PinyinKey(PINYIN_He),PinyinKey(),PinyinKey()},
    {PinyinKey(PINYIN_Ji),PinyinKey(),PinyinKey()},
    {PinyinKey(PINYIN_Qi),PinyinKey(),PinyinKey()},
    {PinyinKey(PINYIN_Xi),PinyinKey(),PinyinKey()},
    {PinyinKey(PINYIN_Zhi),PinyinKey(),PinyinKey()},
    {PinyinKey(PINYIN_Chi),PinyinKey(),PinyinKey()},
    {PinyinKey(PINYIN_Shi),PinyinKey(),PinyinKey()},
    {PinyinKey(PINYIN_Ri),PinyinKey(),PinyinKey()},
    {PinyinKey(PINYIN_Zi),PinyinKey(),PinyinKey()},
    {PinyinKey(PINYIN_Ci),PinyinKey(),PinyinKey()},
    {PinyinKey(PINYIN_Si),PinyinKey(),PinyinKey()},
    {PinyinKey(PINYIN_ZeroInitial,PINYIN_A),PinyinKey(),PinyinKey()},
    {PinyinKey(PINYIN_ZeroInitial,PINYIN_O),PinyinKey(),PinyinKey()},
    {PinyinKey(PINYIN_ZeroInitial,PINYIN_E),PinyinKey(),PinyinKey()},
    {PinyinKey(PINYIN_ZeroInitial,PINYIN_Ea),PinyinKey(),PinyinKey()},
    {PinyinKey(PINYIN_ZeroInitial,PINYIN_Ai),PinyinKey(),PinyinKey()},
    {PinyinKey(PINYIN_ZeroInitial,PINYIN_Ei),PinyinKey(),PinyinKey()},
    {PinyinKey(PINYIN_ZeroInitial,PINYIN_Ao),PinyinKey(),PinyinKey()},
    {PinyinKey(PINYIN_ZeroInitial,PINYIN_Ou),PinyinKey(),PinyinKey()},
    {PinyinKey(PINYIN_ZeroInitial,PINYIN_An),PinyinKey(),PinyinKey()},
    {PinyinKey(PINYIN_ZeroInitial,PINYIN_En),PinyinKey(),PinyinKey()},
    {PinyinKey(PINYIN_ZeroInitial,PINYIN_Ang),PinyinKey(),PinyinKey()},
    {PinyinKey(PINYIN_ZeroInitial,PINYIN_Eng),PinyinKey(),PinyinKey()},
    {PinyinKey(PINYIN_ZeroInitial,PINYIN_Er),PinyinKey(),PinyinKey()},
    {PinyinKey(PINYIN_ZeroInitial,PINYIN_I),PinyinKey(),PinyinKey()},
    {PinyinKey(PINYIN_ZeroInitial,PINYIN_U),PinyinKey(),PinyinKey()},
    {PinyinKey(PINYIN_ZeroInitial,PINYIN_V),PinyinKey(),PinyinKey()},
};

static const size_t __zhuyin_map_start_char = 0x20;
#include "pinyin_zhuyin_map_data.h"

static const PinyinKey (*__zhuyin_maps []) [3] = {
    __zhuyin_zhuyin_map,
    __zhuyin_standard_map,
    __zhuyin_hsu_map,
    __zhuyin_ibm_map,
    __zhuyin_gin_yieh_map,
    __zhuyin_et_map,
    __zhuyin_et26_map,
    0
};


//////////////////////////////////////////////////////////////////////////////
// implementation of PinyinKey

const guint16 PinyinKey::min_value = 0;
const guint16 PinyinKey::max_value = PINYIN_Number_Of_Initials * PINYIN_Number_Of_Finals * PINYIN_Number_Of_Tones - 1;

const char*
PinyinKey::get_initial_string () const
{
    return __pinyin_initials [m_initial].latin;
}

const char*
PinyinKey::get_initial_zhuyin_string () const
{
    if ((m_initial == PINYIN_Wu && m_final == PINYIN_U) ||
        (m_initial == PINYIN_Yi &&
         (m_final == PINYIN_I || m_final == PINYIN_In || m_final == PINYIN_Ing || m_final == PINYIN_Ong ||
          m_final == PINYIN_U || m_final == PINYIN_Ue || m_final == PINYIN_Uan || m_final == PINYIN_Un)))
        return "";

    return __pinyin_initials [m_initial].zhuyin;
}

const char*
PinyinKey::get_final_string () const
{
    return __pinyin_finals [m_final].latin;
}

const char*
PinyinKey::get_final_zhuyin_string () const
{
    if (m_initial == PINYIN_Yi && m_final == PINYIN_Ong) {
        return __pinyin_finals [PINYIN_Iong].zhuyin;
    } else if (m_initial == PINYIN_Yi || m_initial == PINYIN_Ji || m_initial == PINYIN_Qi || m_initial == PINYIN_Xi) {
        switch (m_final) {
            case PINYIN_U:
                return __pinyin_finals [PINYIN_V].zhuyin;
            case PINYIN_Ue:
                return __pinyin_finals [PINYIN_Ve].zhuyin;
            case PINYIN_Uan:
                return __pinyin_finals [PINYIN_Van].zhuyin;
            case PINYIN_Un:
                return __pinyin_finals [PINYIN_Vn].zhuyin;
        }
        if (m_initial == PINYIN_Yi && m_final == PINYIN_E)
            return __pinyin_finals [PINYIN_Ea].zhuyin;
    } else if ((m_initial == PINYIN_Ne || m_initial == PINYIN_Le) && m_final == PINYIN_Ue) {
        return __pinyin_finals [PINYIN_Ve].zhuyin;
    } else if ((m_initial == PINYIN_Zhi || m_initial == PINYIN_Chi || m_initial == PINYIN_Shi ||
                m_initial == PINYIN_Zi  || m_initial == PINYIN_Ci  || m_initial == PINYIN_Si  ||
                m_initial == PINYIN_Ri) && m_final == PINYIN_I) {
        return "";
    }

    return __pinyin_finals [m_final].zhuyin;
}

const char*
PinyinKey::get_tone_string () const
{
    return __pinyin_tones [m_tone].latin;
}

const char*
PinyinKey::get_tone_zhuyin_string () const
{
    return __pinyin_tones [m_tone].zhuyin;
}

const char *
PinyinKey::get_key_string () const
{
    char key [16];
    g_snprintf (key, 15, "%s%s%s", get_initial_string(), get_final_string(), get_tone_string ());

    return g_strdup(key);
}

const char *
PinyinKey::get_key_zhuyin_string () const
{
    char key [32];
    g_snprintf (key, 31, "%s%s%s", get_initial_zhuyin_string(), get_final_zhuyin_string(), get_tone_zhuyin_string ());

    return g_strdup (key);
}

int
PinyinKey::set (const PinyinValidator &validator, const char *str, int len)
{
    if (!str || ! (*str))
        return 0;

    PinyinDefaultParser parser;

    return parser.parse_one_key (validator, *this, str, len);
}

//////////////////////////////////////////////////////////////////////////////
// implementation of PinyinValidator
BitmapPinyinValidator::BitmapPinyinValidator (const PinyinLargeTable *table)
{
    initialize (table);
}

void
BitmapPinyinValidator::initialize (const PinyinLargeTable *table)
{
    memset (m_bitmap, 0, sizeof (m_bitmap));

    if (!table) return;

    for (guint16 val=0; val<=PinyinKey::max_value; ++val)
        if (!table->has_key (PinyinKey (val)))
            m_bitmap [val >> 3] |= (1 << (val % 8));
}

bool
BitmapPinyinValidator::operator () (PinyinKey key) const
{
    if (key.is_empty ()) return false;

    guint16 val = key.get_value ();

    return  (m_bitmap [ val >> 3 ] & (1 << (val % 8))) == 0;
}

//////////////////////////////////////////////////////////////////////////////
// implementation of PinyinParser
PinyinParser::~PinyinParser ()
{
}

struct PinyinReplaceRulePair
{
    PinyinInitial initial;
    PinyinFinal   final;
    PinyinInitial new_initial;
    PinyinFinal   new_final;
};

class PinyinReplaceRulePairLessThan
{
public:
    bool operator () (const PinyinReplaceRulePair &lhs, const PinyinReplaceRulePair &rhs) const {
        if (lhs.initial < rhs.initial) return true;
        if (lhs.initial > rhs.initial) return false;
        return lhs.final < rhs.final;
    }
};

void
PinyinParser::normalize (PinyinKey &key)
{
    static const PinyinReplaceRulePair rules [] = 
    {
#if 0
        {PINYIN_ZeroInitial, PINYIN_I,    PINYIN_Yi, PINYIN_I},
        {PINYIN_ZeroInitial, PINYIN_Ia,   PINYIN_Yi, PINYIN_A},
        {PINYIN_ZeroInitial, PINYIN_Ian,  PINYIN_Yi, PINYIN_An},
        {PINYIN_ZeroInitial, PINYIN_Iang, PINYIN_Yi, PINYIN_Ang},
        {PINYIN_ZeroInitial, PINYIN_Iao,  PINYIN_Yi, PINYIN_Ao},
        {PINYIN_ZeroInitial, PINYIN_Ie,   PINYIN_Yi, PINYIN_E},
        {PINYIN_ZeroInitial, PINYIN_In,   PINYIN_Yi, PINYIN_In},
        {PINYIN_ZeroInitial, PINYIN_Ing,  PINYIN_Yi, PINYIN_Ing},
        {PINYIN_ZeroInitial, PINYIN_Iong, PINYIN_Yi, PINYIN_Ong},
        {PINYIN_ZeroInitial, PINYIN_Iu,   PINYIN_Yi, PINYIN_Ou},
        {PINYIN_ZeroInitial, PINYIN_U,    PINYIN_Wu, PINYIN_U},
        {PINYIN_ZeroInitial, PINYIN_Ua,   PINYIN_Wu, PINYIN_A},
        {PINYIN_ZeroInitial, PINYIN_Uai,  PINYIN_Wu, PINYIN_Ai},
        {PINYIN_ZeroInitial, PINYIN_Uan,  PINYIN_Wu, PINYIN_An},
        {PINYIN_ZeroInitial, PINYIN_Uang, PINYIN_Wu, PINYIN_Ang},
        {PINYIN_ZeroInitial, PINYIN_Ue,   PINYIN_Wu, PINYIN_E},
        {PINYIN_ZeroInitial, PINYIN_Ueng, PINYIN_Wu, PINYIN_Eng},
        {PINYIN_ZeroInitial, PINYIN_Ui,   PINYIN_Wu, PINYIN_Ei},
        {PINYIN_ZeroInitial, PINYIN_Un,   PINYIN_Wu, PINYIN_En},
        {PINYIN_ZeroInitial, PINYIN_Uo,   PINYIN_Wu, PINYIN_O},
        {PINYIN_ZeroInitial, PINYIN_V,    PINYIN_Yi, PINYIN_U},
        {PINYIN_ZeroInitial, PINYIN_Van,  PINYIN_Yi, PINYIN_Uan},
        {PINYIN_ZeroInitial, PINYIN_Ve,   PINYIN_Yi, PINYIN_Ue},
        {PINYIN_ZeroInitial, PINYIN_Vn,   PINYIN_Yi, PINYIN_Un},
#endif
        {PINYIN_Ji,          PINYIN_V,    PINYIN_Ji, PINYIN_U},
        {PINYIN_Ji,          PINYIN_Van,  PINYIN_Ji, PINYIN_Uan},
        {PINYIN_Ji,          PINYIN_Ve,   PINYIN_Ji, PINYIN_Ue},
        {PINYIN_Ji,          PINYIN_Vn,   PINYIN_Ji, PINYIN_Un},
        {PINYIN_Ne,          PINYIN_Ve,   PINYIN_Ne, PINYIN_Ue},
        {PINYIN_Le,          PINYIN_Ve,   PINYIN_Le, PINYIN_Ue},
        {PINYIN_Qi,          PINYIN_V,    PINYIN_Qi, PINYIN_U},
        {PINYIN_Qi,          PINYIN_Van,  PINYIN_Qi, PINYIN_Uan},
        {PINYIN_Qi,          PINYIN_Ve,   PINYIN_Qi, PINYIN_Ue},
        {PINYIN_Qi,          PINYIN_Vn,   PINYIN_Qi, PINYIN_Un},
        {PINYIN_Xi,          PINYIN_V,    PINYIN_Xi, PINYIN_U},
        {PINYIN_Xi,          PINYIN_Van,  PINYIN_Xi, PINYIN_Uan},
        {PINYIN_Xi,          PINYIN_Ve,   PINYIN_Xi, PINYIN_Ue},
        {PINYIN_Xi,          PINYIN_Vn,   PINYIN_Xi, PINYIN_Un}
    };
    static const PinyinReplaceRulePair *rules_start = rules;
    static const PinyinReplaceRulePair *rules_end   = rules + sizeof(rules)/sizeof(PinyinReplaceRulePair);

    PinyinReplaceRulePair kp;

    kp.initial = key.get_initial ();
    kp.final = key.get_final ();

    const PinyinReplaceRulePair *p = std_lite::lower_bound (rules_start, rules_end, kp, PinyinReplaceRulePairLessThan ());

    if (p->initial == kp.initial && p->final == kp.final) {
        key.set_initial (p->new_initial);
        key.set_final (p->new_final);
    }
}

//============== Internal functions used by PinyinDefaultParser ==============
static int
__default_parser_parse_initial (PinyinInitial &initial, const char *str, int len)
{
    int lastlen = 0;

    initial = PINYIN_ZeroInitial;

    if (str && *str >= 'a' && *str <= 'z') {
        int start = __pinyin_initials_index [*str - 'a'].start;
        int end = __pinyin_initials_index [*str - 'a'].num + start;

        if (start > 0) {
            for (int i = start; i < end; ++i) {
                if ((len < 0 || len >= __pinyin_initials [i].latin_len) && __pinyin_initials [i].latin_len >= lastlen) {
                    int j;
                    for (j = 1; j < __pinyin_initials [i].latin_len; ++j) {
                        if (str [j] != __pinyin_initials [i].latin [j])
                            break;
                    }
                    if (j == __pinyin_initials [i].latin_len) {
                        initial = static_cast<PinyinInitial>(i);
                        lastlen = __pinyin_initials [i].latin_len;
                    }
                }
            }
        }
    }

    return lastlen;
}
static int
__default_parser_parse_final (PinyinFinal &final, const char *str, int len)
{
    int lastlen = 0;

    final = PINYIN_ZeroFinal;

    if (str && *str >= 'a' && *str <= 'z') {
        int start = __pinyin_finals_index [*str - 'a'].start;
        int end = __pinyin_finals_index [*str - 'a'].num + start;

        if (start > 0) {
            for (int i = start; i < end; ++i) {
                if ((len < 0 || len >= __pinyin_finals [i].latin_len) && __pinyin_finals [i].latin_len >= lastlen) {
                    int j;
                    for (j = 1; j < __pinyin_finals [i].latin_len; ++j) {
                        if (str [j] != __pinyin_finals [i].latin [j])
                            break;
                    }
                    if (j == __pinyin_finals [i].latin_len) {
                        final = static_cast<PinyinFinal>(i);
                        lastlen = __pinyin_finals [i].latin_len;
                    }
                }
            }
        }
    }

    return lastlen;
}
static int
__default_parser_parse_tone (PinyinTone &tone, const char *str, int len)
{
    tone = PINYIN_ZeroTone;

    if (str && (len >= 1 || len < 0)) {
        int kt = (*str) - '0';
        if (kt >= PINYIN_First && kt <= PINYIN_LastTone) {
            tone = static_cast<PinyinTone>(kt);
            return 1;
        }
    }
    return 0;
}

static int
__default_parser_parse_one_key (const PinyinValidator &validator, PinyinKey &key, const char *str, int len = -1)
{
    int initial_len = 0;
    int final_len   = 0;
    int tone_len    = 0;

    const char *ptr;

    PinyinInitial initial;
    PinyinFinal   final;
    PinyinTone    tone;

    key.clear ();

    if (!str || !len) return 0;

    if (len < 0) len = strlen (str);

    while (len > 0) {
        ptr = str;

        initial = PINYIN_ZeroInitial;
        final   = PINYIN_ZeroFinal;
        tone    = PINYIN_ZeroTone;

        final_len = __default_parser_parse_final (final, ptr, len);
        ptr += final_len;
        len -= final_len;
 
        // An initial is present
        if (final == PINYIN_ZeroFinal) {
            initial_len = __default_parser_parse_initial (initial, ptr, len);
            ptr += initial_len;
            len -= initial_len;
            if (len){
                final_len = __default_parser_parse_final (final, ptr, len);
                ptr += final_len;
                len -= final_len;
            }
        }

        if (len)
            tone_len = __default_parser_parse_tone (tone, ptr, len);
 
        key.set (initial, final, tone);
 
        PinyinParser::normalize (key);

        // A valid key was found, return.
        if (validator (key)) break;

        // The key is invalid, reduce the len and find again.
        len = initial_len + final_len + tone_len - 1;

        initial_len = final_len = tone_len = 0;

        key.clear ();
    }

    len = initial_len + final_len + tone_len;

    return len;
}

struct DefaultParserCacheElement
{
    PinyinKey key;
    PinyinKeyPos pos;
    int num_keys;
    int parsed_len;
    int next_start;
};

typedef  GArray* DefaultParserCache; /* Array of DefaultParserCacheElement */

static int
__default_parser_parse_recursive (const PinyinValidator &validator,
                                  DefaultParserCache    &cache,
                                  int                   &real_start,
                                  int                   &num_keys,
                                  const char            *str,
                                  int                    len,
                                  int                    start)
{
    if (*str == 0 || len == 0) return 0;

    int used_len = 0;

    real_start = 0;
    num_keys = 0;

    if (*str == '\'' || *str == ' ') {
        ++used_len;
        ++str;
        ++start;
        --len;
    }

    if (!isalpha (*str) || !len)
        return 0;

    real_start = start;

    // The best keys start from this position have been found, just return the result.
    DefaultParserCacheElement* element = &g_array_index
	(cache, DefaultParserCacheElement, start);
						       
						       
    if (element->num_keys >=0) {
        num_keys = element->num_keys;
        return element->parsed_len;
    }

    PinyinKey first_key;
    PinyinKey best_first_key;
    PinyinKeyPos pos; 

    int first_len = 0;
    int best_first_len = 0;

    int remained_len = 0;
    int best_remained_len = 0;

    int remained_keys = 0;
    int best_remained_keys = 0;

    int remained_start = 0;
    int best_remained_start = 0;

    first_len = __default_parser_parse_one_key (validator, first_key, str, len);

    if (!first_len) {
	element = &g_array_index(cache, DefaultParserCacheElement, start);

        element->key = PinyinKey ();
        element->num_keys = 0;
        element->parsed_len = 0;
	element->next_start = start;
        return 0;
    }

    best_first_key = first_key;
    best_first_len = first_len;

    if (len > first_len) {
        char ch1 = str [first_len -1];
        char ch2 = str [first_len];

        best_remained_len = __default_parser_parse_recursive (validator,
                                                              cache,
                                                              best_remained_start,
                                                              best_remained_keys,
                                                              str + first_len,
                                                              len - first_len,
                                                              start + first_len);

        // For those keys which the last char is 'g' or 'n' or 'r', try put the end char into the next key.
        if (first_len > 1 &&
            (((ch1=='g' || ch1=='n' || ch1=='r') && (ch2=='a' || ch2=='e' || ch2=='i' || ch2=='o' || ch2=='u' || ch2=='v')) ||
             ((ch1=='a' || ch1=='e' || ch1=='o') && (ch2=='i' || ch2=='n' || ch2=='o' || ch2=='r' || ch2=='u')))) {

            first_len = __default_parser_parse_one_key (validator, first_key, str, first_len - 1);

            if (first_len) {
                remained_len = __default_parser_parse_recursive (validator,
                                                                 cache, 
                                                                 remained_start,
                                                                 remained_keys,
                                                                 str + first_len,
                                                                 len - first_len,
                                                                 start + first_len);


	DefaultParserCacheElement* best_remained_element = &g_array_index
	    (cache, DefaultParserCacheElement, best_remained_start);		

                // A better seq was found.
                if (remained_len != 0 && (remained_len + first_len) >= (best_remained_len + best_first_len) &&
                    (remained_keys <= best_remained_keys || best_remained_keys == 0)) {
#if 1
                    if ((remained_len + first_len) > (best_remained_len + best_first_len) ||
                        remained_keys < best_remained_keys ||
                        best_remained_element->key.get_final () == PINYIN_ZeroFinal ||
                        best_remained_element->key.get_initial () == PINYIN_Wu ||
                        best_remained_element->key.get_initial () == PINYIN_Yi) {
#endif
                        best_first_len = first_len;
                        best_first_key = first_key;
                        best_remained_len = remained_len;
                        best_remained_keys = remained_keys;
                        best_remained_start = remained_start;
#if 1
                    }
#endif
                }
            }
        }
    }

    num_keys = best_remained_keys + 1;
    
    
    element = &g_array_index
	(cache, DefaultParserCacheElement, start);
    
    pos.set_pos(start);
    pos.set_length(best_first_len);

    element->key = best_first_key;
    element->pos = pos;
    element->num_keys = num_keys;
    element->parsed_len = used_len + best_first_len + best_remained_len;
    element->next_start = best_remained_start;

    return element->parsed_len;
}
//============================================================================

PinyinDefaultParser::~PinyinDefaultParser ()
{
}

int
PinyinDefaultParser::parse_one_key (const PinyinValidator &validator, PinyinKey &key, const char *str, int len) const
{
    return __default_parser_parse_one_key (validator, key, str, len);
}

int
PinyinDefaultParser::parse (const PinyinValidator &validator, PinyinKeyVector & keys, PinyinKeyPosVector & poses, const char *str, int len) const
{
    g_array_set_size(keys, 0);
    g_array_set_size(poses, 0);

    if (!str || !len) return 0;

    if (len < 0) len = strlen (str);

    DefaultParserCacheElement elm;

    elm.num_keys = -1L;
    elm.parsed_len = 0;
    elm.next_start = 0;

    DefaultParserCache cache = g_array_new (FALSE, TRUE, sizeof (DefaultParserCacheElement));
    g_array_set_size(cache, len);
    for ( int index = 0 ; index < len ; index++){
	DefaultParserCacheElement * element =
	    &g_array_index(cache,DefaultParserCacheElement, index);
	*element = elm;	
    }
    int start = 0;
    int num_keys = 0;

    len = __default_parser_parse_recursive (validator, cache, start, num_keys, str, len, 0);

    for (size_t i=0; i<(size_t)num_keys; ++i) {
	DefaultParserCacheElement* element = &g_array_index
	    (cache, DefaultParserCacheElement, start);
        g_array_append_val(keys, element->key);
	g_array_append_val(poses, element->pos);
        start = element->next_start;
    }

    return len;
}

PinyinShuangPinParser::PinyinShuangPinParser (PinyinShuangPinScheme scheme)
{
    set_scheme (scheme);
}

PinyinShuangPinParser::PinyinShuangPinParser (const PinyinInitial initial_map[27], const PinyinFinal final_map[27][2])
{
    set_scheme (initial_map, final_map);
}

PinyinShuangPinParser::~PinyinShuangPinParser ()
{
}

int
PinyinShuangPinParser::parse_one_key (const PinyinValidator &validator, PinyinKey &key, const char *str, int len) const
{
    key.clear ();

    if (!str || !len || ! (*str)) return 0;

    if (len < 0) len = strlen (str);

    PinyinInitial initial    = PINYIN_ZeroInitial;
    PinyinFinal   final      = PINYIN_ZeroFinal;
    PinyinFinal   final_cands [4] = { PINYIN_ZeroFinal, PINYIN_ZeroFinal, PINYIN_ZeroFinal, PINYIN_ZeroFinal };

    PinyinTone    tone = PINYIN_ZeroTone;

    int idx [2] = {-1, -1};
    int used_len = 0;

    size_t i;
    bool matched = false;

    for (i = 0; i < 2 && i < (size_t) len; ++i) {
        if (str [i] >= 'a' && str [i] <= 'z') idx [i] = str [i] - 'a';
        else if (str [i] == ';') idx [i] = 26;
    }

    // parse initial or final
    if (idx [0] >= 0) {
        initial = m_initial_map [idx[0]];
        final_cands [0] = m_final_map [idx[0]][0];
        final_cands [1] = m_final_map [idx[0]][1];
    }

    if (initial == PINYIN_ZeroInitial && final_cands [0] == PINYIN_ZeroFinal)
        return 0;

    // parse final, if str [0] == 'o' (idx [0] == 14) then just skip to parse final.
    if (idx [1] >= 0 && (initial != PINYIN_ZeroInitial || idx[0] == 14)) {
        final_cands [2] = m_final_map [idx [1]][0];
        final_cands [3] = m_final_map [idx [1]][1];

        for (i = 2; i < 4; ++i) {
            if (final_cands [i] != PINYIN_ZeroFinal) {
                key.set (initial, final_cands [i]);
                PinyinParser::normalize (key);

                if (validator (key)) {
                    final = final_cands [i];
                    matched = true;
                    used_len = 2;
                    str += 2;
                    len -= 2;
                    break;
                }
            }
        }
    }

    if (!matched) {
        initial = PINYIN_ZeroInitial;
        for (i = 0; i < 2; ++i) {
            key.set (initial, final_cands [i]);
            PinyinParser::normalize (key);

            if (validator (key)) {
                final = final_cands [i];
                matched = true;
                used_len = 1;
                ++str;
                --len;
                break;
            }
        }
    }

    if (!matched) return 0;

    // parse tone
    if (len) {
        int kt = (*str) - '0';
        if (kt >= PINYIN_First && kt <= PINYIN_LastTone) {
            tone = static_cast<PinyinTone>(kt);

            key.set (initial, final, tone);

            if (validator (key)) {
                return used_len + 1;
            }
        }
    }

    return used_len;
}

int
PinyinShuangPinParser::parse (const PinyinValidator &validator, PinyinKeyVector &keys, PinyinKeyPosVector & poses, const char *str, int len) const
{
    g_array_set_size(keys, 0);
    g_array_set_size(poses, 0);

    if (!str || !len || ! (*str)) return 0;

    if (len < 0) len = strlen (str);

    int used_len = 0;

    PinyinKey key;
    PinyinKeyPos pos;

    while (used_len < len) {
        if (*str == '\'' || *str == ' ') {
            ++str;
            ++used_len;
            continue;
        }

        int one_len = parse_one_key (validator, key, str, len);

        if (one_len) {
	    pos.set_pos(used_len);
	    pos.set_length(one_len);
            g_array_append_val(keys, key);
	    g_array_append_val(poses, pos);
        } else {
            break;
        }

        str += one_len;
        used_len += one_len;
    }

    return used_len;
}

void
PinyinShuangPinParser::set_scheme (PinyinShuangPinScheme scheme)
{
    switch (scheme) {
#if 0
    case SHUANG_PIN_STONE:
        set_scheme (__shuang_pin_stone_initial_map, __shuang_pin_stone_final_map);
        break;
#endif
    case SHUANG_PIN_ZRM:
        set_scheme (__shuang_pin_zrm_initial_map, __shuang_pin_zrm_final_map);
        break;
    case SHUANG_PIN_MS:
        set_scheme (__shuang_pin_ms_initial_map, __shuang_pin_ms_final_map);
        break;
    case SHUANG_PIN_ZIGUANG:
        set_scheme (__shuang_pin_ziguang_initial_map, __shuang_pin_ziguang_final_map);
        break;
    case SHUANG_PIN_ABC:
        set_scheme (__shuang_pin_abc_initial_map, __shuang_pin_abc_final_map);
        break;
#if 0
    case SHUANG_PIN_LIUSHI:
        set_scheme (__shuang_pin_liushi_initial_map, __shuang_pin_liushi_final_map);
        break;
#endif
    case SHUANG_PIN_PYJJ:
        set_scheme (__shuang_pin_pyjj_initial_map, __shuang_pin_pyjj_final_map);
        break;
    case SHUANG_PIN_XHE:
        set_scheme (__shuang_pin_xhe_initial_map, __shuang_pin_xhe_final_map);
        break;
    default:
        set_scheme (SHUANG_PIN_DEFAULT);
        return;
    }
}

void
PinyinShuangPinParser::set_scheme (const PinyinInitial initial_map[27], const PinyinFinal final_map[27][2])
{
    for (size_t i = 0; i < 27; ++i) {
        m_initial_map [i] = initial_map [i];
        m_final_map [i][0] = final_map [i][0];
        m_final_map [i][1] = final_map [i][1];
    }
}

void
PinyinShuangPinParser::get_scheme (PinyinInitial initial_map[27], PinyinFinal final_map[27][2])
{
    for (size_t i = 0; i < 27; ++i) {
        initial_map [i] = m_initial_map [i];
        final_map [i][0] = m_final_map [i][0];
        final_map [i][1] = m_final_map [i][1];
    }
}

PinyinZhuYinParser::PinyinZhuYinParser (PinyinZhuYinScheme scheme)
    : m_scheme (scheme)
{
}

PinyinZhuYinParser::~PinyinZhuYinParser ()
{
}

int
PinyinZhuYinParser::parse_one_key (const PinyinValidator &validator, PinyinKey &key, const char *str, int len) const
{
    PinyinKey candkeys[4][3];
    gunichar ch;

    if (len < 0) len = g_utf8_strlen (str, -1);

    for (int i= 0; i < 4 && i < len; ++i) {
        ch = g_utf8_get_char (str);
        if (!get_keys (candkeys[i], ch))
            break;
        str = g_utf8_next_char (str);
    }

    return pack_keys (key, validator, candkeys);
}

int
PinyinZhuYinParser::parse (const PinyinValidator &validator, PinyinKeyVector & keys, PinyinKeyPosVector & poses, const char *str, int len) const
{
    g_array_set_size(keys, 0);
    g_array_set_size(poses, 0);

    if (!str || !len || ! (*str)) return 0;

    int used_len = 0;
 
    PinyinKey key;
    PinyinKeyPos pos;
 
    if (len < 0) len = g_utf8_strlen (str, -1);

    while (used_len < len) {
        if (g_utf8_get_char (str) == ' ') {
            ++used_len;
            str = g_utf8_next_char (str);
            continue;
        }

        int one_len = parse_one_key (validator, key, str, len);

        if (one_len) {
            pos.set_pos (used_len);
            pos.set_length (one_len);
            g_array_append_val (keys, key);
            g_array_append_val (poses, pos);
        } else {
            break;
        }

        /* utf8 next n chars. */
        for ( int i = 0; i < one_len; ++i ) {
            str = g_utf8_next_char (str);
        }
        used_len += one_len;
    }

    return used_len;
}

void
PinyinZhuYinParser::set_scheme (PinyinZhuYinScheme scheme)
{
    m_scheme = scheme;
}

PinyinZhuYinScheme
PinyinZhuYinParser::get_scheme () const
{
    return m_scheme;
}

bool
PinyinZhuYinParser::get_keys (PinyinKey keys[], gunichar ch) const
{
    if (m_scheme == ZHUYIN_ZHUYIN) {
        if (ch == 0x20 || ch == 0x02C9) keys [0].set_tone (PINYIN_First);
        else if (ch == 0x02CA) keys [0].set_tone (PINYIN_Second);
        else if (ch == 0x02C7) keys [0].set_tone (PINYIN_Third);
        else if (ch == 0x02CB) keys [0].set_tone (PINYIN_Fourth);
        else if (ch == 0x02D9) keys [0].set_tone (PINYIN_Fifth);
        else if (ch >= 0x3105 && ch <= 0x3129) {
            keys[0] = __zhuyin_zhuyin_map[ch - 0x3105][0];
            keys[1] = __zhuyin_zhuyin_map[ch - 0x3105][1];
            keys[2] = __zhuyin_zhuyin_map[ch - 0x3105][2];
        }
    } else if (ch >= 0x20 && ch <= 0x7D) {
        keys[0] = __zhuyin_maps[m_scheme][ch - 0x20][0];
        keys[1] = __zhuyin_maps[m_scheme][ch - 0x20][1];
        keys[2] = __zhuyin_maps[m_scheme][ch - 0x20][2];
    } else {
        keys[0].clear ();
        keys[1].clear ();
        keys[2].clear ();
    }

    return !keys[0].is_empty ();
}

struct ZhuYinFinalReplaceRulePair
{
    PinyinFinal final1;
    PinyinFinal final2;
    PinyinFinal new_final;
};

class ZhuYinFinalReplaceRulePairLessThan
{
public:
    bool operator () (const ZhuYinFinalReplaceRulePair &lhs, const ZhuYinFinalReplaceRulePair &rhs) const {
        if (lhs.final1 < rhs.final1) return true;
        if (lhs.final1 > rhs.final1) return false;
        return lhs.final2 < rhs.final2;
    }
};

int
PinyinZhuYinParser::pack_keys (PinyinKey &key, const PinyinValidator &validator, const PinyinKey keys[][3]) const
{
    static const ZhuYinFinalReplaceRulePair final_rules [] =
    {
        {PINYIN_I, PINYIN_A,   PINYIN_Ia},
        {PINYIN_I, PINYIN_An,  PINYIN_Ian},
        {PINYIN_I, PINYIN_Ang, PINYIN_Iang},
        {PINYIN_I, PINYIN_Ao,  PINYIN_Iao},
        {PINYIN_I, PINYIN_Ea,  PINYIN_Ie},
        {PINYIN_I, PINYIN_En,  PINYIN_In},
        {PINYIN_I, PINYIN_Eng, PINYIN_Ing},
        {PINYIN_I, PINYIN_O,   PINYIN_I},
        {PINYIN_I, PINYIN_Ou,  PINYIN_Iu},
        {PINYIN_U, PINYIN_A,   PINYIN_Ua},
        {PINYIN_U, PINYIN_Ai,  PINYIN_Uai},
        {PINYIN_U, PINYIN_An,  PINYIN_Uan},
        {PINYIN_U, PINYIN_Ang, PINYIN_Uang},
        {PINYIN_U, PINYIN_Ei,  PINYIN_Ui},
        {PINYIN_U, PINYIN_En,  PINYIN_Un},
        {PINYIN_U, PINYIN_Eng, PINYIN_Ueng},
        {PINYIN_U, PINYIN_O,   PINYIN_Uo},
        {PINYIN_V, PINYIN_An,  PINYIN_Van},
        {PINYIN_V, PINYIN_Ea,  PINYIN_Ve},
        {PINYIN_V, PINYIN_En,  PINYIN_Vn},
        {PINYIN_V, PINYIN_Eng, PINYIN_Iong}
    };

    static const ZhuYinFinalReplaceRulePair *final_rules_start = final_rules;
    static const ZhuYinFinalReplaceRulePair *final_rules_end   = final_rules + sizeof(final_rules)/sizeof(ZhuYinFinalReplaceRulePair);

    PinyinInitial initial;
    PinyinFinal   final1;
    PinyinFinal   final2;
    PinyinTone    tone;

    PinyinKey     best_key;
    int           best_used_keys = 0;
    int           best_score     = -1;
    bool          best_key_valid = false;

    size_t        num;
    size_t        size [4];
    size_t        possibles [4];

    for (num=0; !keys[num][0].is_empty () && num<4; ++num) {
        for (size[num]=0; !keys[num][size[num]].is_empty () && size[num]<3; ++size[num]);

        possibles[num] = (num > 0 ? possibles[num-1] : 1) * size[num];
    }

    while (num) {
        for (size_t i=0; i<possibles[num-1]; ++i) {
            size_t  n         = i;
            int     score     = 1;
            int     used_keys = 0;

            initial = PINYIN_ZeroInitial;
            final1 = final2 = PINYIN_ZeroFinal;
            tone = PINYIN_ZeroTone;

            for (size_t t=0; t<num; ++t) {
                size_t idx = n % size[t];
                n /= size[t];

                if (keys[t][idx].get_initial () && !initial) {
                    initial = keys[t][idx].get_initial ();
                    if (final1) score = 0;
                } else if (keys[t][idx].get_final () && !(final1 && final2)) {
                    if (!final1) final1 = keys[t][idx].get_final ();
                    else if (!final2) final2 = keys[t][idx].get_final ();
                } else if (keys[t][idx].get_tone () && !tone) {
                    tone = keys[t][idx].get_tone ();
                } else {
                    break;
                }

                used_keys = t+1;

                // No initial and final allowed after tone key.
                if (tone) break;
            }

            // A better candidate has been found.
            if (best_score > score)
                continue;

            // Is it possible?
            if (!initial && !final1 && !final2)
                continue;

            if (final1 && final2) {
                if (final2 == PINYIN_I || final2 == PINYIN_U || final2 == PINYIN_V)
                    std_lite::swap (final1, final2);
 
                // Invalid finals.
                if (final1 != PINYIN_I && final1 != PINYIN_U && final1 != PINYIN_V)
                    continue;
 
                // In such case, there must be no initial,
                // otherwise it's illegal.
                if (final1 == PINYIN_I && final2 == PINYIN_O) {
                    if (!initial) {
                        initial = PINYIN_Yi;
                        final1 = PINYIN_O;
                        final2 = PINYIN_ZeroFinal;
                    } else {
                        continue;
                    }
                } else {
                    ZhuYinFinalReplaceRulePair fp;
                    fp.final1 = final1;
                    fp.final2 = final2;
 
                    const ZhuYinFinalReplaceRulePair *p =
                        std_lite::lower_bound (final_rules_start, final_rules_end, fp, ZhuYinFinalReplaceRulePairLessThan ());
 
                    // It's invalid that got two finals but they are not in our rules
                    if (p != final_rules_end && p->final1 == fp.final1 && p->final2 == fp.final2)
                        final1 = p->new_final;
                    else
                        continue; 
 
                    if (final1 == PINYIN_Ueng && initial)
                        final1 = PINYIN_Ong;
                }
            } else if ((initial == PINYIN_Zhi || initial == PINYIN_Chi || initial == PINYIN_Shi ||
                        initial == PINYIN_Zi  || initial == PINYIN_Ci  || initial == PINYIN_Si  ||
                        initial == PINYIN_Ri) && !final1) {
                final1 = PINYIN_I;
            }

            key.set (initial, final1, tone);
            PinyinParser::normalize (key);

            bool key_valid;
            if (best_score < score ||
                (best_score == score &&
                 (best_used_keys < used_keys ||
                  ((key_valid = validator (key)) && !best_key_valid)))) {

                best_key = key;
                best_used_keys = used_keys;
                best_score = score;
                best_key_valid = key_valid;

                // Break loop if a valid key with tone has been found.
                if (key_valid && final1 && tone) {
                    num = 0;
                    break;
                }
            }
        }

        if (num > (size_t)best_used_keys)
            num = best_used_keys;
        else
            break;
    }

    // CAUTION: The best key maybe not a valid key
    key = best_key;
    // pos.set_length (best_used_keys);
    return best_used_keys;
}

namespace pinyin{

//////////////////////////////////////////////////////////////////////////////
// implementation of PinyinKey comparision classe
int pinyin_compare_initial (const PinyinCustomSettings &custom,
			    PinyinInitial lhs,
			    PinyinInitial rhs)
{
    if ((lhs == rhs) ||

        (custom.use_ambiguities [PINYIN_AmbCiChi] &&
         (lhs == PINYIN_Ci && rhs == PINYIN_Chi)) ||
        (custom.use_ambiguities [PINYIN_AmbChiCi] &&
         (lhs == PINYIN_Chi && rhs == PINYIN_Ci)) ||

        (custom.use_ambiguities [PINYIN_AmbZiZhi] &&
         (lhs == PINYIN_Zi && rhs == PINYIN_Zhi)) ||
        (custom.use_ambiguities [PINYIN_AmbZhiZi] &&
         (lhs == PINYIN_Zhi && rhs == PINYIN_Zi)) ||

        (custom.use_ambiguities [PINYIN_AmbSiShi] &&
         (lhs == PINYIN_Si && rhs == PINYIN_Shi)) ||
        (custom.use_ambiguities [PINYIN_AmbShiSi] &&
         (lhs == PINYIN_Shi && rhs == PINYIN_Si)) ||

        (custom.use_ambiguities [PINYIN_AmbLeNe] &&
         (lhs == PINYIN_Le && rhs == PINYIN_Ne)) ||
        (custom.use_ambiguities [PINYIN_AmbNeLe] &&
         (lhs == PINYIN_Ne && rhs == PINYIN_Le)) ||

        (custom.use_ambiguities [PINYIN_AmbLeRi] &&
         (lhs == PINYIN_Le && rhs == PINYIN_Ri)) ||
        (custom.use_ambiguities [PINYIN_AmbRiLe] &&
         (lhs == PINYIN_Ri && rhs == PINYIN_Le)) ||

        (custom.use_ambiguities [PINYIN_AmbFoHe] &&
         (lhs == PINYIN_Fo && rhs == PINYIN_He)) ||
        (custom.use_ambiguities [PINYIN_AmbHeFo] &&
         (lhs == PINYIN_He && rhs == PINYIN_Fo)) ||

        (custom.use_ambiguities [PINYIN_AmbGeKe] &&
         (lhs == PINYIN_Ge && rhs == PINYIN_Ke)) ||
        (custom.use_ambiguities [PINYIN_AmbKeGe] &&
         (lhs == PINYIN_Ke && rhs == PINYIN_Ge))
        )
        return 0;
    else return (lhs - rhs);
}

int pinyin_compare_final (const PinyinCustomSettings &custom,
			  PinyinFinal lhs,
			  PinyinFinal rhs)
{
    if((lhs == rhs) ||

       (custom.use_ambiguities [PINYIN_AmbAnAng] &&
        (lhs == PINYIN_An && rhs == PINYIN_Ang)) ||
       (custom.use_ambiguities [PINYIN_AmbAngAn] &&
        (lhs == PINYIN_Ang && rhs == PINYIN_An)) ||

       (custom.use_ambiguities [PINYIN_AmbEnEng] &&
        (lhs == PINYIN_En && rhs == PINYIN_Eng)) ||
       (custom.use_ambiguities [PINYIN_AmbEngEn] &&
        (lhs == PINYIN_Eng && rhs == PINYIN_En)) ||

       (custom.use_ambiguities [PINYIN_AmbInIng] &&
        (lhs == PINYIN_In && rhs == PINYIN_Ing)) ||
       (custom.use_ambiguities [PINYIN_AmbIngIn] &&
        (lhs == PINYIN_Ing && rhs == PINYIN_In))
       )
        return 0;
    else if (custom.use_incomplete &&
             (lhs == PINYIN_ZeroFinal || rhs == PINYIN_ZeroFinal))
        return 0;
    else return (lhs - rhs);
}

int pinyin_compare_tone (const PinyinCustomSettings &custom,
			 PinyinTone lhs,
			 PinyinTone rhs)
{
    if(lhs == rhs || !lhs || !rhs)
        return 0;
    else return (lhs - rhs);
}

};
