# -*- coding: utf-8 -*-
# vim:set et sts=4 sw=4:
#
# libpinyin - Library to deal with pinyin.
#
# Copyright (c) 2007-2008 Peng Huang <shawn.p.huang@gmail.com>
# Copyright (C) 2011 Peng Wu <alexepico@gmail.com>
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2, or (at your option)
# any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

auto_correct = [
    # "correct", "wrong", distance
    ("ng", "gn", 1),
    ("ng", "mg", 1),
    ("iu", "iou", 1),
    ("ui", "uei", 1),
    ("un", "uen", 1),
#    ("ue", "ve"),
    ("ve", "ue", 1),
    ("ong", "on", 1),
]

auto_correct_ext = [
    # "correct", "wrong", flag, distance
    ("ju", "jv", "PINYIN_CORRECT_V_U", 1),
    ("qu", "qv", "PINYIN_CORRECT_V_U", 1),
    ("xu", "xv", "PINYIN_CORRECT_V_U", 1),
    ("yu", "yv", "PINYIN_CORRECT_V_U", 1),

    ("jue", "jve", "PINYIN_CORRECT_V_U", 1),
    ("que", "qve", "PINYIN_CORRECT_V_U", 1),
    ("xue", "xve", "PINYIN_CORRECT_V_U", 1),
    ("yue", "yve", "PINYIN_CORRECT_V_U", 1),

    ("juan", "jvan", "PINYIN_CORRECT_V_U", 1),
    ("quan", "qvan", "PINYIN_CORRECT_V_U", 1),
    ("xuan", "xvan", "PINYIN_CORRECT_V_U", 1),
    ("yuan", "yvan", "PINYIN_CORRECT_V_U", 1),

    ("jun", "jvn", "PINYIN_CORRECT_V_U", 1),
    ("qun", "qvn", "PINYIN_CORRECT_V_U", 1),
    ("xun", "xvn", "PINYIN_CORRECT_V_U", 1),
    ("yun", "yvn", "PINYIN_CORRECT_V_U", 1),

#    ("juang", "jvang", "PINYIN_CORRECT_V_U", 1),
#    ("quang", "qvang", "PINYIN_CORRECT_V_U", 1),
#    ("xuang", "xvang", "PINYIN_CORRECT_V_U", 1),
#    ("yuang", "yvang", "PINYIN_CORRECT_V_U", 1),

#    ("jun", "jven", "PINYIN_CORRECT_UEN_UN | PINYIN_CORRECT_V_U", 1),
#    ("qun", "qven", "PINYIN_CORRECT_UEN_UN | PINYIN_CORRECT_V_U", 1),
#    ("xun", "xven", "PINYIN_CORRECT_UEN_UN | PINYIN_CORRECT_V_U", 1),
#    ("yun", "yven", "PINYIN_CORRECT_UEN_UN | PINYIN_CORRECT_V_U", 1),
]


fuzzy_shengmu = [
    ("c", "ch"),
    ("ch", "c"),
    ("z", "zh"),
    ("zh", "z"),
    ("s", "sh"),
    ("sh", "s"),
    ("l", "n"),
    ("n", "l"),
    ("f", "h"),
    ("h", "f"),
    ("l", "r"),
    ("r", "l"),
    ("k", "g"),
    ("g", "k"),
]

fuzzy_yunmu = [
    ("an", "ang"),
    ("ang", "an"),
    ("en", "eng"),
    ("eng", "en"),
    ("in", "ing"),
    ("ing", "in"),
]

fuzzy_corrects = [
    # "correct", "wrong", flag, distance
    ("shua", "sua", "PINYIN_AMB_S_SH", 1),
    ("zhua", "zua", "PINYIN_AMB_Z_ZH", 1),
]

# for HSU and ETEN26

hsu_correct = [
    # "correct", "wrong"
    ("ㄓ" , "ㄐ"),
    ("ㄔ" , "ㄑ"),
    ("ㄕ" , "ㄒ"),
    ("ㄛ" , "ㄏ"),
    ("ㄜ" , "ㄍ"),
    ("ㄢ" , "ㄇ"),
    ("ㄣ" , "ㄋ"),
    ("ㄤ" , "ㄎ"),
    ("ㄦ" , "ㄌ"),
    ("ㄐㄧ*" , "ㄍㄧ*"),
    ("ㄐㄩ*" , "ㄍㄩ*"),
    ("ㄓㄨ*" , "ㄐㄨ*"),
    ("ㄔㄨ*" , "ㄑㄨ*"),
    ("ㄕㄨ*" , "ㄒㄨ*"),
#    ("ㄐㄧ*" , "ㄍㄧ*"),
#    ("ㄐㄩ*" , "ㄍㄩ*"),
]


hsu_correct_special = [
# "correct", "wrong"
# ㄐㄑㄒ must follow ㄧㄩ
# m_middle == zero from libchewing code
    ("ㄓ*" , "ㄐ*"),
    ("ㄔ*" , "ㄑ*"),
    ("ㄕ*" , "ㄒ*"),
]


eten26_correct = [
    # "correct", "wrong"
    ("ㄓ" , "ㄐ"),
    ("ㄕ" , "ㄒ"),
    ("ㄡ" , "ㄆ"),
    ("ㄢ" , "ㄇ"),
    ("ㄣ" , "ㄋ"),
    ("ㄤ" , "ㄊ"),
    ("ㄥ" , "ㄌ"),
    ("ㄦ" , "ㄏ"),
    ("ㄓㄨ*" , "ㄐㄨ*"),
    ("ㄕㄨ*" , "ㄒㄨ*"),
    ("ㄑㄧ*" , "ㄍㄧ*"),
    ("ㄑㄩ*" , "ㄍㄩ*"),
]


eten26_correct_special = [
# "correct", "wrong"
# ㄐㄒ must follow ㄧㄩ
# m_middle == zero from libchewing code
    ("ㄓ*" , "ㄐ*"),
    ("ㄕ*" , "ㄒ*"),
]


dachen_cp26_switch = [
# switch key, from, to
    ('q', "ㄅ", "ㄆ"),
    ('q', "ㄆ", "ㄅ"),

    ('w', "ㄉ", "ㄊ"),
    ('w', "ㄊ", "ㄉ"),

    ('t', "ㄓ", "ㄔ"),
    ('t', "ㄔ", "ㄓ"),

    ('i', "ㄛ", "ㄞ"),
    ('i', "ㄞ", "ㄛ"),

    ('o', "ㄟ", "ㄢ"),
    ('o', "ㄢ", "ㄟ"),

    ('l', "ㄠ", "ㄤ"),
    ('l', "ㄤ", "ㄠ"),

    ('p', "ㄣ", "ㄦ"),
    ('p', "ㄦ", "ㄣ"),
]

dachen_cp26_switch_special = [
# m_initial != zero || m_middle != zero

    ('b', "ㄖ", "ㄝ"),

    ('n', "ㄙ", "ㄣ"),

# switching between "ㄧ", "ㄚ", and "ㄧㄚ"
# m_middle == 'ㄧ' and m_final != 'ㄚ'
    ('u', "ㄧ", "ㄚ"),
# m_middle != 'ㄧ' and m_final == 'ㄚ'
    ('u', "ㄚ", "ㄧㄚ"),
# m_middle == 'ㄧ' and m_final == "ㄚ"
    ('u', "ㄧㄚ", ""),
# m_middle != zero
    ('u', "*?", "*ㄚ"),

# switching between "ㄩ" and "ㄡ"
# m_final != 'ㄡ'
    ('m', "ㄩ", "ㄡ"),
# m_middle != 'ㄩ'
    ('m', "ㄡ", "ㄩ"),
# m_middle != zero
    ('m', "*?", "*ㄡ"),
]
