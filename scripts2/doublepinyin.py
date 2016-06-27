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


MSPY_SHUANGPIN_SHENGMU_DICT = {
    "b" : "b", "c" : "c", "d" : "d", "f" : "f", "g" : "g",
    "h" : "h", "i" : "ch","j" : "j", "k" : "k", "l" : "l",
    "m" : "m", "n" : "n", "o" : "'", "p" : "p", "q" : "q",
    "r" : "r", "s" : "s", "t" : "t", "u" : "sh","v" : "zh",
    "w" : "w", "x" : "x", "y" : "y", "z" : "z"
}

MSPY_SHUANGPIN_YUNMU_DICT = {
    "a" : ("a",),
    "b" : ("ou",),
    "c" : ("iao",),
    "d" : ("uang", "iang"),
    "e" : ("e",),
    "f" : ("en",),
    "g" : ("eng", "ng"),
    "h" : ("ang",),
    "i" : ("i",),
    "j" : ("an",),
    "k" : ("ao",),
    "l" : ("ai",),
    "m" : ("ian",),
    "n" : ("in",),
    "o" : ("uo", "o"),
    "p" : ("un",),
    "q" : ("iu",),
    "r" : ("uan", "er"),
    "s" : ("ong", "iong"),
    "t" : ("ue",),
    "u" : ("u",),
    "v" : ("ui","ue"),
    "w" : ("ia","ua"),
    "x" : ("ie",),
    "y" : ("uai", "v"),
    "z" : ("ei",),
    ";" : ("ing",)
}

ZRM_SHUANGPIN_SHENGMU_DICT = {
    "b" : "b", "c" : "c", "d" : "d", "f" : "f", "g" : "g",
    "h" : "h", "i" : "ch","j" : "j", "k" : "k", "l" : "l",
    "m" : "m", "n" : "n", "o" : "'", "p" : "p", "q" : "q",
    "r" : "r", "s" : "s", "t" : "t", "u" : "sh","v" : "zh",
    "w" : "w", "x" : "x", "y" : "y", "z" : "z"
}

ZRM_SHUANGPIN_YUNMU_DICT = {
    "a" : ("a",),
    "b" : ("ou",),
    "c" : ("iao",),
    "d" : ("uang", "iang"),
    "e" : ("e",),
    "f" : ("en",),
    "g" : ("eng", "ng"),
    "h" : ("ang",),
    "i" : ("i",),
    "j" : ("an",),
    "k" : ("ao",),
    "l" : ("ai",),
    "m" : ("ian",),
    "n" : ("in",),
    "o" : ("uo", "o"),
    "p" : ("un",),
    "q" : ("iu",),
    "r" : ("uan", "er"),
    "s" : ("ong", "iong"),
    "t" : ("ue",),
    "u" : ("u",),
    "v" : ("ui","v"),
    "w" : ("ia","ua"),
    "x" : ("ie",),
    "y" : ("uai", "ing"),
    "z" : ("ei",),
}

ABC_SHUANGPIN_SHENGMU_DICT = {
    "a" : "zh", "b" : "b", "c" : "c", "d" : "d", "e":"ch", "f" : "f", "g" : "g",
    "h" : "h", "j" : "j", "k" : "k", "l" : "l",
    "m" : "m", "n" : "n", "o" : "'", "p" : "p", "q" : "q",
    "r" : "r", "s" : "s", "t" : "t", "v" : "sh",
    "w" : "w", "x" : "x", "y" : "y", "z" : "z"
}

ABC_SHUANGPIN_YUNMU_DICT = {
    "a" : ("a",),
    "b" : ("ou",),
    "c" : ("in","uai"),
    "d" : ("ia", "ua"),
    "e" : ("e",),
    "f" : ("en",),
    "g" : ("eng", "ng"),
    "h" : ("ang",),
    "i" : ("i",),
    "j" : ("an",),
    "k" : ("ao",),
    "l" : ("ai",),
    "m" : ("ue","ui"),
    "n" : ("un",),
    "o" : ("uo", "o"),
    "p" : ("uan",),
    "q" : ("ei",),
    "r" : ("er", "iu"),
    "s" : ("ong", "iong"),
    "t" : ("iang","uang"),
    "u" : ("u",),
    "v" : ("v","ue"),
    "w" : ("ian",),
    "x" : ("ie",),
    "y" : ("ing",),
    "z" : ("iao",),
}

ZGPY_SHUANGPIN_SHENGMU_DICT = {
    "a" : "ch", "b" : "b", "c" : "c", "d" : "d", "f" : "f", "g" : "g",
    "h" : "h", "i" : "sh","j" : "j", "k" : "k", "l" : "l",
    "m" : "m", "n" : "n", "o" : "'", "p" : "p", "q" : "q",
    "r" : "r", "s" : "s", "t" : "t", "u" : "zh",
    "w" : "w", "x" : "x", "y" : "y", "z" : "z"
}

ZGPY_SHUANGPIN_YUNMU_DICT = {
    "a" : ("a", ),
    "b" : ("iao", ),
    "d" : ("ie", ),
    "e" : ("e", ),
    "f" : ("ian", ),
    "g" : ("iang", "uang"),
    "h" : ("ong", "iong"),
    "i" : ("i", ),
    "j" : ("er", "iu"),
    "k" : ("ei", ),
    "l" : ("uan", ),
    "m" : ("un", ),
    "n" : ("ue", "ui"),
    "o" : ("uo", "o"),
    "p" : ("ai", ),
    "q" : ("ao", ),
    "r" : ("an", ),
    "s" : ("ang", ),
    "t" : ("eng", "ng"),
    "u" : ("u", ),
    "v" : ("v", ),
    "w" : ("en", ),
    "x" : ("ia", "ua"),
    "y" : ("in", "uai"),
    "z" : ("ou" ,),
    ";" : ("ing", )
}

PYJJ_SHUANGPIN_SHENGMU_DICT = {
    "a" : "'", "b" : "b", "c" : "c", "d" : "d", "f" : "f", "g" : "g",
    "h" : "h", "i" : "sh","j" : "j", "k" : "k", "l" : "l",
    "m" : "m", "n" : "n", "o" : "'", "p" : "p", "q" : "q",
    "r" : "r", "s" : "s", "t" : "t", "u" : "ch","v" : "zh",
    "w" : "w", "x" : "x", "y" : "y", "z" : "z"
}

PYJJ_SHUANGPIN_YUNMU_DICT = {
    "a" : ("a",),
    "b" : ("ia","ua"),
    "c" : ("uan",),
    "d" : ("ao", ),
    "e" : ("e",),
    "f" : ("an",),
    "g" : ("ang",),
    "h" : ("iang","uang"),
    "i" : ("i",),
    "j" : ("ian",),
    "k" : ("iao",),
    "l" : ("in",),
    "m" : ("ie",),
    "n" : ("iu",),
    "o" : ("uo", "o"),
    "p" : ("ou",),
    "q" : ("er","ing"),
    "r" : ("en", ),
    "s" : ("ai", ),
    "t" : ("eng", "ng"),
    "u" : ("u",),
    "v" : ("v","ui"),
    "w" : ("ei",),
    "x" : ("uai","ue"),
    "y" : ("ong","iong"),
    "z" : ("un",),
}

XHE_SHUANGPIN_SHENGMU_DICT = {
    "b" : "b", "c" : "c", "d" : "d", "f" : "f", "g" : "g",
    "h" : "h", "i" : "ch", "j" : "j", "k" : "k", "l" : "l",
    "m" : "m", "n" : "n", "o" : "'", "p" : "p", "q" : "q",
    "r" : "r", "s" : "s", "t" : "t", "u" : "sh", "v" : "zh",
    "w" : "w", "x" : "x", "y" : "y", "z" : "z",
}

XHE_SHUANGPIN_YUNMU_DICT = {
    "a" : ("a",),
    "b" : ("in",),
    "c" : ("ao",),
    "d" : ("ai",),
    "e" : ("e",),
    "f" : ("en",),
    "g" : ("eng", "ng"),
    "h" : ("ang",),
    "i" : ("i",),
    "j" : ("an",),
    "k" : ("uai", "ing"),
    "l" : ("iang", "uang"),
    "m" : ("ian",),
    "n" : ("iao",),
    "o" : ("uo", "o"),
    "p" : ("ie",),
    "q" : ("iu",),
    "r" : ("uan", "er"),
    "s" : ("ong", "iong"),
    "t" : ("ue",),
    "u" : ("u",),
    "v" : ("v", "ui"),
    "w" : ("ei",),
    "x" : ("ia", "ua"),
    "y" : ("un",),
    "z" : ("ou",),
}

SHUANGPIN_SCHEMAS = {
    "MSPY" : (MSPY_SHUANGPIN_SHENGMU_DICT, MSPY_SHUANGPIN_YUNMU_DICT),
    "ZRM"  : (ZRM_SHUANGPIN_SHENGMU_DICT,  ZRM_SHUANGPIN_YUNMU_DICT),
    "ABC"  : (ABC_SHUANGPIN_SHENGMU_DICT,  ABC_SHUANGPIN_YUNMU_DICT),
    "ZGPY" : (ZGPY_SHUANGPIN_SHENGMU_DICT, ZGPY_SHUANGPIN_YUNMU_DICT),
    "PYJJ" : (PYJJ_SHUANGPIN_SHENGMU_DICT, PYJJ_SHUANGPIN_YUNMU_DICT),
    "XHE"  : (XHE_SHUANGPIN_SHENGMU_DICT,  XHE_SHUANGPIN_YUNMU_DICT),
}

