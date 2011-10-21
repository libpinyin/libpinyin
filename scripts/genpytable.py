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
from fuzzy import *


def check_pinyin_chewing_map():
    for pinyin_key in pinyin.PINYIN_DICT.keys():
        if pinyin_key in bopomofo.PINYIN_BOPOMOFO_MAP.keys():
            pass
        else:
            print("pinyin %s has no chewing mapping", pinyin_key)

def get_chewing(pinyin_key):
    initial = 'CHEWING_ZERO_INITIAL'
    middle = 'CHEWING_ZERO_MIDDLE'
    final = 'CHEWING_ZERO_FINAL'
    assert pinyin_key != None
    assert pinyin_key in bopomofo.PINYIN_BOPOMOFO_MAP
    bopomofo_str = bopomofo.PINYIN_BOPOMOFO_MAP[pinyin_key]
    for char in bopomofo_str:
        if char in chewing.CHEWING_ASCII_INITIAL_MAP:
            initial = chewing.CHEWING_ASCII_INITIAL_MAP[char]
        if char in chewing.CHEWING_ASCII_MIDDLE_MAP:
            middle = chewing.CHEWING_ASCII_MIDDLE_MAP[char]
        if char in chewing.CHEWING_ASCII_FINAL_MAP:
            final = chewing.CHEWING_ASCII_FINAL_MAP[char]
    return initial, middle, final


### main function ###
if __name__ == "__main__":
    #pre-check here
    check_pinyin_chewing_map()
    #dump
    for pinyin_key in pinyin.PINYIN_DICT.keys():
        print (pinyin_key, get_chewing(pinyin_key))
