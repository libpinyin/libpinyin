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
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.


import pinyin
from utils import expand_file

def gen_shengmu_table(scheme):
    entries = []
    #select shengmu mapping
    sheng = pinyin.SHUANGPIN_SCHEMAS[scheme][0]
    for c in "abcdefghijklmnopqrstuvwxyz;":
       sh = sheng.get(c, "NULL")
       if sh != "NULL":
           sh = '"{0}"'.format(sh)
       entry = '{{{0: <5}}} /* {1} */'.format(sh, c.upper())
       entries.append(entry)
    return ',\n'.join(entries)


def gen_yunmu_table(scheme):
    entries = []
    #select yunmu mapping
    yun = pinyin.SHUANGPIN_SCHEMAS[scheme][1]
    for c in "abcdefghijklmnopqrstuvwxyz;":
        y = yun.get(c, ("NULL", "NULL"))
        if len(y) == 1:
            y1 = y[0]
            y2 = "NULL"
        else:
            y1, y2 = y
        if y1 != "NULL":
            y1 = '"{0}"'.format(y1)
        if y2 != "NULL":
            y2 = '"{0}"'.format(y2)
        entry = '{{{{{0: <7}, {1: <7}}}}} /* {2} */'.format(y1, y2, c.upper())
        entries.append(entry)
    return ',\n'.join(entries)


def get_table_content(tablename):
    (scheme, part) = tablename.split('_', 1)
    if part == "SHENG":
        return gen_shengmu_table(scheme)
    if part == "YUN":
        return gen_yunmu_table(scheme)


### main function ###
if __name__ == "__main__":
    expand_file("double_pinyin_table.h.in", get_table_content)
