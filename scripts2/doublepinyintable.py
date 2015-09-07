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


from doublepinyin import SHUANGPIN_SCHEMAS
from fullpinyin import YUNMU_LIST


def gen_shengmu_table(scheme):
    entries = []
    #select shengmu mapping
    sheng = SHUANGPIN_SCHEMAS[scheme][0]
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
    yun = SHUANGPIN_SCHEMAS[scheme][1]
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


#https://zh.wikipedia.org/zh-hans/双拼
def gen_fallback_table2(scheme):
    entries = []
    #select yunmu mapping
    yun = SHUANGPIN_SCHEMAS[scheme][1]
    for yunmu in sorted(YUNMU_LIST):
        char1 = yunmu[0]
        char2 = None
        for k, v in yun.items():
            if len(v) == 1:
                if yunmu == v[0]:
                    char2 = k
            if len(v) == 2:
                if yunmu == v[0] or yunmu == v[1]:
                    char2 = k

        if char2 == None:
            continue
        index = char1 + char2
        entry = '{{"{0}", "{1}"}}'.format(index, yunmu)
        entries.append(entry)
    return ',\n'.join(entries)


def gen_fallback_table3(scheme):
    entries = []
    #select yunmu mapping
    yun = SHUANGPIN_SCHEMAS[scheme][1]
    for yunmu in sorted(YUNMU_LIST):
        #special case for double character yunmu
        if len(yunmu) == 2:
            entry = '{{"{0}", "{1}"}}'.format(yunmu, yunmu)
            entries.append(entry)
            continue

        #same as gen_fallback_table2
        char1 = yunmu[0]
        char2 = None
        for k, v in yun.items():
            if len(v) == 1:
                if yunmu == v[0]:
                    char2 = k
            if len(v) == 2:
                if yunmu == v[0] or yunmu == v[1]:
                    char2 = k

        if char2 == None:
            continue
        index = char1 + char2
        entry = '{{"{0}", "{1}"}}'.format(index, yunmu)
        entries.append(entry)
    return ',\n'.join(entries)



'''
def get_table_content(tablename):
    (scheme, part) = tablename.split('_', 1)
    if part == "SHENG":
        return gen_shengmu_table(scheme)
    if part == "YUN":
        return gen_yunmu_table(scheme)
'''


### main function ###
if __name__ == "__main__":
    print(gen_fallback_table2("PYJJ"))
    print(gen_fallback_table3("ZRM"))
    print(gen_fallback_table3("XHE"))
