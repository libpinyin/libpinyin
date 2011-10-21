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
# Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

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
    ("ju", "jv", "PINYIN_CORRECT_V_TO_U"),
    ("qu", "qv", "PINYIN_CORRECT_V_TO_U"),
    ("xu", "xv", "PINYIN_CORRECT_V_TO_U"),
    ("yu", "yv", "PINYIN_CORRECT_V_TO_U"),

    ("jue", "jve", "PINYIN_CORRECT_V_TO_U"),
    ("que", "qve", "PINYIN_CORRECT_V_TO_U"),
    ("xue", "xve", "PINYIN_CORRECT_V_TO_U"),
    ("yue", "yve", "PINYIN_CORRECT_V_TO_U"),

    ("juan", "jvan", "PINYIN_CORRECT_V_TO_U"),
    ("quan", "qvan", "PINYIN_CORRECT_V_TO_U"),
    ("xuan", "xvan", "PINYIN_CORRECT_V_TO_U"),
    ("yuan", "yvan", "PINYIN_CORRECT_V_TO_U"),

    ("jun", "jvn", "PINYIN_CORRECT_V_TO_U"),
    ("qun", "qvn", "PINYIN_CORRECT_V_TO_U"),
    ("xun", "xvn", "PINYIN_CORRECT_V_TO_U"),
    ("yun", "yvn", "PINYIN_CORRECT_V_TO_U"),

    ("juang", "jvang", "PINYIN_FUZZY_UANG_UAN | PINYIN_CORRECT_V_TO_U"),
    ("quang", "qvang", "PINYIN_FUZZY_UANG_UAN | PINYIN_CORRECT_V_TO_U"),
    ("xuang", "xvang", "PINYIN_FUZZY_UANG_UAN | PINYIN_CORRECT_V_TO_U"),
    ("yuang", "yvang", "PINYIN_FUZZY_UANG_UAN | PINYIN_CORRECT_V_TO_U"),

    ("jun", "jven", "PINYIN_CORRECT_UEN_TO_UN | PINYIN_CORRECT_V_TO_U"),
    ("qun", "qven", "PINYIN_CORRECT_UEN_TO_UN | PINYIN_CORRECT_V_TO_U"),
    ("xun", "xven", "PINYIN_CORRECT_UEN_TO_UN | PINYIN_CORRECT_V_TO_U"),
    ("yun", "yven", "PINYIN_CORRECT_UEN_TO_UN | PINYIN_CORRECT_V_TO_U"),
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
    ("ian", "iang"),
    ("iang", "ian"),
    ("uan", "uang"),
    ("uang", "uan"),
]
