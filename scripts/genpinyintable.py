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

import operator
import bopomofo
from pinyintable import *
from chewingkey import gen_table_index


content_table = []
pinyin_index = []
bopomofo_index = []

#pinyin table
def filter_pinyin_list():
    for (correct, wrong, bopomofo, flags, chewing) in gen_pinyin_list():
        flags = '|'.join(flags)
        chewing = "ChewingKey({0})".format(', '.join(chewing))
        #correct = correct.replace("v", "ü")
        content_table.append((correct, bopomofo, chewing))
        if "IS_PINYIN" in flags:
            pinyin_index.append((wrong, flags, correct))
        if "IS_CHEWING" in flags:
            bopomofo_index.append((bopomofo, flags))


def sort_all():
    global content_table, pinyin_index, bopomofo_index
    #remove duplicates
    content_table = list(set(content_table))
    pinyin_index = list(set(pinyin_index))
    bopomofo_index = list(set(bopomofo_index))
    #define sort function
    sortfunc = operator.itemgetter(0)
    #begin sort
    content_table = sorted(content_table, key=sortfunc)
    #prepend zero item to reserve the invalid item
    content_table.insert(0, ("", "", "ChewingKey()"))
    #sort index
    pinyin_index = sorted(pinyin_index, key=sortfunc)
    bopomofo_index = sorted(bopomofo_index, key=sortfunc)

def get_sheng_yun(pinyin):
    if pinyin == None:
        return None, None
    if pinyin == "":
        return "", ""
    if pinyin == "ng":
        return "", "ng"
    for i in range(2, 0, -1):
        s = pinyin[:i]
        if s in shengmu_list:
            return s, pinyin[i:]
    return "", pinyin

def gen_content_table():
    entries = []
    for ((correct, bopomofo, chewing)) in content_table:
        (shengmu, yunmu) = get_sheng_yun(correct)
        entry = '{{"{0}", "{1}", "{2}", "{3}", {4}}}'.format(correct, shengmu, yunmu, bopomofo, chewing)
        entries.append(entry)
    return ',\n'.join(entries)


def gen_pinyin_index():
    entries = []
    for (wrong, flags, correct) in pinyin_index:
        index = [x[0] for x in content_table].index(correct)
        entry = '{{"{0}", {1}, {2}}}'.format(wrong, flags, index)
        entries.append(entry)
    return ',\n'.join(entries)


def gen_bopomofo_index():
    entries = []
    for (bopomofo_str, flags) in bopomofo_index:
        pinyin_str = bopomofo.BOPOMOFO_PINYIN_MAP[bopomofo_str]
        index = [x[0] for x in content_table].index(pinyin_str)
        entry = '{{"{0}", {1}, {2}}}'.format(bopomofo_str, flags, index)
        entries.append(entry)
    return ',\n'.join(entries)


def gen_chewing_key_table():
    return gen_table_index(content_table)


#init code
filter_pinyin_list()
sort_all()


### main function ###
if __name__ == "__main__":
    #s = gen_content_table() + gen_pinyin_index() + gen_bopomofo_index()
    s = gen_chewing_key_table()
    print(s)
