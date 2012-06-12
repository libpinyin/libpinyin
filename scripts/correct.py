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
    # "correct", "wrong"
    ("ng", "gn"),
    ("ng", "mg"),
    ("iu", "iou"),
    ("ui", "uei"),
    ("un", "uen"),
#    ("ue", "ve"),
    ("ve", "ue"),
    ("ong", "on"),
]

auto_correct_ext = [
    # "correct", "wrong", flag
    ("ju", "jv", "PINYIN_CORRECT_V_U"),
    ("qu", "qv", "PINYIN_CORRECT_V_U"),
    ("xu", "xv", "PINYIN_CORRECT_V_U"),
    ("yu", "yv", "PINYIN_CORRECT_V_U"),

    ("jue", "jve", "PINYIN_CORRECT_V_U"),
    ("que", "qve", "PINYIN_CORRECT_V_U"),
    ("xue", "xve", "PINYIN_CORRECT_V_U"),
    ("yue", "yve", "PINYIN_CORRECT_V_U"),

    ("juan", "jvan", "PINYIN_CORRECT_V_U"),
    ("quan", "qvan", "PINYIN_CORRECT_V_U"),
    ("xuan", "xvan", "PINYIN_CORRECT_V_U"),
    ("yuan", "yvan", "PINYIN_CORRECT_V_U"),

    ("jun", "jvn", "PINYIN_CORRECT_V_U"),
    ("qun", "qvn", "PINYIN_CORRECT_V_U"),
    ("xun", "xvn", "PINYIN_CORRECT_V_U"),
    ("yun", "yvn", "PINYIN_CORRECT_V_U"),

#    ("juang", "jvang", "PINYIN_CORRECT_V_U"),
#    ("quang", "qvang", "PINYIN_CORRECT_V_U"),
#    ("xuang", "xvang", "PINYIN_CORRECT_V_U"),
#    ("yuang", "yvang", "PINYIN_CORRECT_V_U"),

#    ("jun", "jven", "PINYIN_CORRECT_UEN_UN | PINYIN_CORRECT_V_U"),
#    ("qun", "qven", "PINYIN_CORRECT_UEN_UN | PINYIN_CORRECT_V_U"),
#    ("xun", "xven", "PINYIN_CORRECT_UEN_UN | PINYIN_CORRECT_V_U"),
#    ("yun", "yven", "PINYIN_CORRECT_UEN_UN | PINYIN_CORRECT_V_U"),
]


'''
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
'''
