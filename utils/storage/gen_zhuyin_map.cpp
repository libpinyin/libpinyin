/* 
 *  libpinyin
 *  Library to deal with pinyin.
 *  
 *  Copyright (C) 2006 James Su <suzhe@tsinghua.org.cn>
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


#include "pinyin_base.h"
#include <stdio.h>
#include <string.h>

using namespace pinyin;

static const char *map_names [] = {
    "__zhuyin_standard_map",
    "__zhuyin_hsu_map",
    "__zhuyin_ibm_map",
    "__zhuyin_gin_yieh_map",
    "__zhuyin_et_map",
    "__zhuyin_et26_map",
    0
};

static const char *input_keys [] = {
	 "1qaz2wsxedcrfv5tgbyhnujm8ik,9ol.0p;/-7634",		/* standard kb */
	 "bpmfdtnlgkhjvcjvcrzasexuyhgeiawomnkllsdfj",		/* hsu */
	 "1234567890-qwertyuiopasdfghjkl;zxcvbn/m,.",		/* IBM */
	 "2wsx3edcrfvtgb6yhnujm8ik,9ol.0p;/-['=1qaz",		/* Gin-yieh */
	 "bpmfdtnlvkhg7c,./j;'sexuaorwiqzy890-=1234",		/* ET  */
	 "bpmfdtnlvkhgvcgycjqwsexuaorwiqzpmntlhdfjk",		/* ET26 */
     0
};

static PinyinKey pinyin_keys [] =
{
    PinyinKey (PINYIN_Bo),                    PinyinKey (PINYIN_Po),                    PinyinKey (PINYIN_Mo),                    PinyinKey (PINYIN_Fo),
    PinyinKey (PINYIN_De),                    PinyinKey (PINYIN_Te),                    PinyinKey (PINYIN_Ne),                    PinyinKey (PINYIN_Le),
    PinyinKey (PINYIN_Ge),                    PinyinKey (PINYIN_Ke),                    PinyinKey (PINYIN_He),                    PinyinKey (PINYIN_Ji),
    PinyinKey (PINYIN_Qi),                    PinyinKey (PINYIN_Xi),                    PinyinKey (PINYIN_Zhi),                   PinyinKey (PINYIN_Chi),
    PinyinKey (PINYIN_Shi),                   PinyinKey (PINYIN_Ri),                    PinyinKey (PINYIN_Zi),                    PinyinKey (PINYIN_Ci),
    PinyinKey (PINYIN_Si),                    PinyinKey (PINYIN_ZeroInitial,PINYIN_I),  PinyinKey (PINYIN_ZeroInitial,PINYIN_U),  PinyinKey (PINYIN_ZeroInitial,PINYIN_V),
    PinyinKey (PINYIN_ZeroInitial,PINYIN_A),  PinyinKey (PINYIN_ZeroInitial,PINYIN_O),  PinyinKey (PINYIN_ZeroInitial,PINYIN_E),  PinyinKey (PINYIN_ZeroInitial,PINYIN_Ea),
    PinyinKey (PINYIN_ZeroInitial,PINYIN_Ai), PinyinKey (PINYIN_ZeroInitial,PINYIN_Ei), PinyinKey (PINYIN_ZeroInitial,PINYIN_Ao), PinyinKey (PINYIN_ZeroInitial,PINYIN_Ou),
    PinyinKey (PINYIN_ZeroInitial,PINYIN_An), PinyinKey (PINYIN_ZeroInitial,PINYIN_En), PinyinKey (PINYIN_ZeroInitial,PINYIN_Ang),PinyinKey (PINYIN_ZeroInitial,PINYIN_Eng),
    PinyinKey (PINYIN_ZeroInitial,PINYIN_Er),
    PinyinKey (PINYIN_ZeroInitial,PINYIN_ZeroFinal,PINYIN_Fifth),
    PinyinKey (PINYIN_ZeroInitial,PINYIN_ZeroFinal,PINYIN_Second),
    PinyinKey (PINYIN_ZeroInitial,PINYIN_ZeroFinal,PINYIN_Third),
    PinyinKey (PINYIN_ZeroInitial,PINYIN_ZeroFinal,PINYIN_Fourth)
};

void print_map (int num)
{
    PinyinKey map[93][3];

    map[0][0].set_tone (PINYIN_First);

    const char *p = input_keys [num];

    for (size_t i=0; *p; ++i, ++p) {
        size_t idx = *p - 0x20;
        size_t n;
        for (n=0; n<3; ++n)
            if (map[idx][n].is_empty ()) break;

        map[idx][n] = pinyin_keys [i];
    }

    printf("static const PinyinKey %s [][3] = \n{\n", map_names[num]);

    char buf11[40];
    char buf12[40];
    char buf13[40];

    char buf21[40];
    char buf22[40];
    char buf23[40];

    for (size_t i=0; i<93; ++i) {
        snprintf (buf11, 40, "PinyinKey(%d)", map[i][0].get_value ());
        snprintf (buf12, 40, "PinyinKey(%d)", map[i][1].get_value ());
        snprintf (buf13, 40, "PinyinKey(%d)", map[i][2].get_value ());

        snprintf (buf21, 40, "/* %s */", map[i][0].get_key_string ());
        snprintf (buf22, 40, "/* %s */", map[i][1].get_key_string ());
        snprintf (buf23, 40, "/* %s */", map[i][2].get_key_string ());

        printf ("/* %c */{%-15s%9s, %-15s%9s, %-15s%9s},\n", i+0x20, buf11, buf21, buf12, buf22, buf13, buf23);
    }

    printf("};\n\n");
}

int main ()
{
    for (int i=0; input_keys[i]; ++i)
        print_map (i);
}

/*
vi:ts=4:nowrap:ai:expandtab
*/
