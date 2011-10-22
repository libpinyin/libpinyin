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

#ifndef CHEWING_CUSTOM_H
#define CHEWING_CUSTOM_H

namespace pinyin{


/**
 * @brief enums of pinyin ambiguities.
 *
 * Some pinyin element maybe confused by somebody,
 * We allow these ambiguities.
 */
enum PinyinAmbiguityBeta{
    PINYIN_AMB_ANY = 0,
    PINYIN_AMB_C_Ch,
    PINYIN_AMB_Z_Zh,
    PINYIN_AMB_S_Sh,
    PINYIN_AMB_L_N ,
    PINYIN_AMB_F_H ,
    PINYIN_AMB_L_R ,
    PINYIN_AMB_K_G ,
    PINYIN_AMB_AN_ANG,
    PINYIN_AMB_EN_ENG,
    PINYIN_AMB_IN_ING,
    PINYIN_AMB_LAST = PINYIN_AMB_IN_ING
};

/**
 * @brief enums of pinyin corrections.
 */

enum PinyinCorrectionBeta{
    PINYIN_CORRECT_ANY = 0,
    PINYIN_CORRECT_GN_NG,
    PINYIN_CORRECT_MG_NG,
    PINYIN_CORRECT_IOU_IU,
    PINYIN_CORRECT_UEI_UI,
    PINYIN_CORRECT_UEN_UN,
    PINYIN_CORRECT_UE_VE,
    PINYIN_CORRECT_V_U,
    PINYIN_CORRECT_ON_ONG,
    PINYIN_CORRECT_LAST = PINYIN_CORRECT_ON_ONG,
};

};

#endif
