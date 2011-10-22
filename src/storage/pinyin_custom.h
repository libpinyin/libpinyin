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


#ifndef PINYIN_CUSTOM_H
#define PINYIN_CUSTOM_H


namespace pinyin{


/**
 * @brief enums of pinyin ambiguities.
 *
 * Some pinyin element maybe confused by somebody,
 * We allow these ambiguities.
 */
enum PinyinAmbiguity
{
    PINYIN_AmbAny= 0,
    PINYIN_AmbCiChi,
    PINYIN_AmbChiCi,
    PINYIN_AmbZiZhi,
    PINYIN_AmbZhiZi,
    PINYIN_AmbSiShi,
    PINYIN_AmbShiSi,
    PINYIN_AmbLeNe,
    PINYIN_AmbNeLe,
    PINYIN_AmbFoHe,
    PINYIN_AmbHeFo,
    PINYIN_AmbLeRi,
    PINYIN_AmbRiLe,
    PINYIN_AmbKeGe,
    PINYIN_AmbGeKe,
    PINYIN_AmbAnAng,
    PINYIN_AmbAngAn,
    PINYIN_AmbEnEng,
    PINYIN_AmbEngEn,
    PINYIN_AmbInIng,
    PINYIN_AmbIngIn,
    PINYIN_AmbLast = PINYIN_AmbIngIn
};

/**
 * @brief enums of pinyin corrections.
 *
 * These options will be enabled in the second major libpinyin release.
 */

enum PinyinCorrection{
    PINYIN_CorrectAny = 0,
    PINYIN_CorrectVtoU,
    PINYIN_CorrectLast = PINYIN_CorrectVtoU,
};


/**
 * @brief Structure to hold pinyin custom settings.
 *
 * user can custom the behavor of libpinyin by these settings.
 */
struct PinyinCustomSettings
{
    bool use_incomplete;
    /**< allow incomplete pinyin key which only has inital. */

    bool use_tone;
    /**< allow pinyin tone. */

    bool use_ambiguities [PINYIN_AmbLast + 1];
    /**< allow ambiguous pinyin elements or not. */

    bool use_corrections [PINYIN_CorrectLast + 1];
    /**< allow pinyin corrections or not. */

    PinyinCustomSettings ()
        :use_incomplete (true), use_tone (true)
    {
        for (size_t i=0; i<=PINYIN_AmbLast; ++i)
            use_ambiguities [i] = false;
        for (size_t i=0; i<=PINYIN_CorrectLast; ++i)
            use_corrections [i] = false;
    }

    void set_use_incomplete (bool use) { use_incomplete = use; }
    void set_use_tone (bool use) { use_tone = use; }
    void set_use_ambiguities (PinyinAmbiguity amb, bool use)
    {
        if (amb == PINYIN_AmbAny)
            for (size_t i=0; i<=PINYIN_AmbLast; ++i)
                use_ambiguities [i] = use;
        else {
            use_ambiguities [0] = false;
            use_ambiguities [static_cast<size_t>(amb)] = use;
            for (size_t i=1; i<=PINYIN_AmbLast; ++i)
                if (use_ambiguities [i]) {
                    use_ambiguities [0] = true;
                    break;
                }
        }
    }

    void set_use_corrections (PinyinCorrection correct, bool use)
    {
        size_t i;
        if (correct == PINYIN_CorrectAny)
            for (i=0; i<=PINYIN_CorrectLast; ++i)
                use_corrections [i] = use;
        else {
            use_corrections [0] = false;
            use_corrections [static_cast<size_t>(correct)] = use;
            for (i = 1; i<=PINYIN_CorrectLast; ++i)
                if (use_corrections [i]) {
                    use_corrections [0] = true;
                    break;
                }
        }
    }

    bool operator == (const PinyinCustomSettings &rhs) const
    {
        size_t i;
        if (use_incomplete != rhs.use_incomplete)
            return false;

        if (use_tone != rhs.use_tone)
            return false;

        for (i=0; i <= PINYIN_AmbLast; ++i)
            if (use_ambiguities [i] != rhs.use_ambiguities [i])
                return false;

        for (i=0; i <= PINYIN_CorrectLast; ++i)
            if (use_corrections [i] != rhs.use_corrections [i])
                return false;

        return true;
    }

    bool operator != (const PinyinCustomSettings &rhs) const
    {
        return !(*this == rhs);
    }

    guint32 to_value () const
    {
        guint32 val = 0;
        size_t i;

        if (use_incomplete) val |= 1;
        if (use_tone) val |= (1 << 1);

        for (i=0; i <= PINYIN_AmbLast; ++i)
            if (use_ambiguities [i])
                val |= (1 << (i + 2));

        for (i=0; i <= PINYIN_CorrectLast; ++i)
            if (use_corrections [i])
                val |= (1 << (i + PINYIN_AmbLast + 3 ));

        return val;
    }

    void from_value (guint32 val)
    {
        size_t i;
        use_incomplete = (val & 1) != 0;
        use_tone = (val & (1 << 1)) != 0;

        for (i=0; i <= PINYIN_AmbLast; ++i)
            use_ambiguities [i] = (val & (1 << (i + 2))) != 0;

        for (i=0; i <= PINYIN_CorrectLast; ++i)
            use_corrections [i] = (val & (1 << (i + PINYIN_AmbLast + 3))) != 0;
    }
};

};

#endif
