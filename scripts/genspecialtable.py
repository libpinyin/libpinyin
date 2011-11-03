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


import operator
import pinyin
from pinyintable import get_chewing, get_shengmu_chewing
from specialtable import *

pinyin_list = sorted(pinyin.PINYIN_LIST)
shengmu_list = sorted(pinyin.SHENGMU_LIST)

divided_list = []
resplit_list = []


def sort_all():
    global divided_list, resplit_list
    divided_list = sorted(divided_list, key=operator.itemgetter(0))
    resplit_list = sorted(resplit_list, key=operator.itemgetter(0, 1))

def get_chewing_string(pinyin):
    #handle shengmu
    if pinyin not in pinyin_list:
        if pinyin in shengmu_list:
            (initial, middle, final) = get_shengmu_chewing(pinyin)
        else:
            assert False, "Un-expected pinyin string."
    else:
        (initial, middle, final) = get_chewing(pinyin)
    chewing_str = 'ChewingKey({0}, {1}, {2})'.format(initial, middle, final)
    return chewing_str


def gen_divided_table():
    entries = []
    for (pinyin_key, first_key, second_key, freq) in divided_list:
        (pinyin_key, first_key, second_key) = map \
            (get_chewing_string, (pinyin_key, first_key, second_key))
        entry = '{{{0}, {1}, {2}, {3}}}'.format \
            (pinyin_key, first_key, second_key, freq)
        entries.append(entry)
    return ',\n'.join(entries)


def gen_resplit_table():
    entries = []
    for (orig_first_key, orig_second_key, orig_freq, \
        new_first_key, new_second_key, new_freq) in resplit_list:
        (orig_first_key, orig_second_key, new_first_key, new_second_key) = map\
            (get_chewing_string, (orig_first_key, orig_second_key, \
                                      new_first_key, new_second_key))
        entry = '{{{0}, {1}, {2}, {3}, {4}, {5}}}'.format \
            (orig_first_key, orig_second_key, orig_freq, \
                 new_first_key, new_second_key, new_freq)
        entries.append(entry)
    return ',\n'.join(entries)


### main function ###
if __name__ == "__main__":
    load_phrase("pinyin2.txt")

    #load lists
    divided_list = filter_divided()
    resplit_list = filter_resplit()
    sort_all()

    s = gen_divided_table() + '\n' + gen_resplit_table()
    print(s)

