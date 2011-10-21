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

import pinyin
import bopomofo
import chewing
from correct import *


def check_pinyin_chewing_map():
    for pinyin_key in pinyin.PINYIN_DICT.keys():
        if pinyin_key in bopomofo.PINYIN_BOPOMOFO_MAP.keys():
            pass
        else:
            print("pinyin %s has no chewing mapping", pinyin_key)

def get_chewing(pinyin_key):
    initial, middle, final = \
        'CHEWING_ZERO_INITIAL', 'CHEWING_ZERO_MIDDLE', 'CHEWING_ZERO_FINAL'
    assert pinyin_key != None
    assert pinyin_key in bopomofo.PINYIN_BOPOMOFO_MAP

    #handle 'w' and 'y'
    if pinyin_key[0] == 'w':
        initial = 'PINYIN_W'
    if pinyin_key[0] == 'y':
        initial = 'PINYIN_Y'

    #get chewing string
    bopomofo_str = bopomofo.PINYIN_BOPOMOFO_MAP[pinyin_key]

    #handle 'ci', 'chi', 'si', 'shi', 'zi', 'zhi', 'ri'
    if pinyin_key in {'ci', 'chi', 'si', 'shi', 'zi', 'zhi', 'ri'}:
        middle = "CHEWING_I"
    #normal process
    for char in bopomofo_str:
        if char in chewing.CHEWING_ASCII_INITIAL_MAP:
            initial = chewing.CHEWING_ASCII_INITIAL_MAP[char]
        if char in chewing.CHEWING_ASCII_MIDDLE_MAP:
            middle = chewing.CHEWING_ASCII_MIDDLE_MAP[char]
        if char in chewing.CHEWING_ASCII_FINAL_MAP:
            final = chewing.CHEWING_ASCII_FINAL_MAP[char]
        if char == "ㄜ": #merge "ㄝ" and "ㄜ"
            final = "CHEWING_E"

    #handle "ueng"/"ong"
    if middle == "CHEWING_U" and final == "CHEWING_ENG":
        middle, final = "CHEWING_ZERO_MIDDLE", "PINYIN_ONG"

    #handle "ien"/"in"
    if middle == "CHEWING_I" and final == "CHEWING_EN":
        middle, final = "CHEWING_ZERO_MIDDLE", "PINYIN_IN"

    #handle "ieng"/"ing"
    if middle == "CHEWING_I" and final == "CHEWING_ENG":
        middle, final = "CHEWING_ZERO_MIDDLE", "PINYIN_ING"

    return initial, middle, final


### main function ###
if __name__ == "__main__":
    #pre-check here
    check_pinyin_chewing_map()
    #dump
    for pinyin_key in pinyin.PINYIN_DICT.keys():
        print (pinyin_key, get_chewing(pinyin_key))
