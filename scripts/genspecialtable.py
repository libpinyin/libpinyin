# -*- coding: utf-8 -*-
# vim:set et sts=4 sw=4:
#
# libpinyin - Library to deal with pinyin.
#
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

import sys
import pinyin

pinyin_list = sorted(pinyin.PINYIN_LIST)
shengmu_list = sorted(pinyin.SHENGMU_LIST)
yunmu_list = sorted(pinyin.YUNMU_LIST)

def get_all_special():
    for p in pinyin_list:
        if p[-1] in ["n", "g", "r"]:
            for yun in yunmu_list:
                if yun not in pinyin_list:
                    continue
                new_pinyin = p[-1] + yun
                # if new_pinyin in pinyin_list:
                yield p, yun, p[:-1], new_pinyin
        elif p[-1] in ["e"]:
            yield p, "r", p[:-1], "er"

if __name__ == "__main__":
    for pinyins in get_all_special():
        print (pinyins)
