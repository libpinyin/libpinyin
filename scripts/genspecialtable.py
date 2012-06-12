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

'''
def get_chewing_string(pinyin):
    #handle shengmu
    if pinyin not in pinyin_list:
        if pinyin in shengmu_list:
            chewing_key = get_shengmu_chewing(pinyin)
        else:
            assert False, "Un-expected pinyin string."
    else:
        chewing_key = get_chewing(pinyin)
    chewing_str = 'ChewingKey({0})'.format(', '.join(chewing_key))
    return chewing_str
'''

def gen_divided_table():
    entries = []
    for (pinyin_key, orig_freq, first_key, second_key, new_freq) \
            in divided_list:

        if orig_freq >= new_freq:
            assert orig_freq > 0, "Expected orig_freq > 0 here."

        entry = '{{"{0}", {1}, {{"{2}", "{3}"}}, {4}}}'.format \
            (pinyin_key, orig_freq, first_key, second_key, new_freq)
        entries.append(entry)
    return ',\n'.join(entries)


def gen_resplit_table():
    entries = []
    for (orig_first_key, orig_second_key, orig_freq, \
        new_first_key, new_second_key, new_freq) in resplit_list:

        if orig_freq >= new_freq:
            assert orig_freq > 0, "Expected orig_freq > 0 here."

        entry = '{{{{"{0}", "{1}"}}, {2}, {{"{3}", "{4}"}}, {5}}}'.format \
            (orig_first_key, orig_second_key, orig_freq,\
                 new_first_key, new_second_key, new_freq)
        entries.append(entry)
    return ',\n'.join(entries)


#init code, load lists
divided_list = filter_divided()
resplit_list = filter_resplit()
sort_all()


### main function ###
if __name__ == "__main__":
    s = gen_divided_table() + '\n' + gen_resplit_table()
    print(s)

