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


import os
import sys
import math
import pinyin

pinyin_list = sorted(pinyin.PINYIN_LIST)
shengmu_list = sorted(pinyin.SHENGMU_LIST)
yunmu_list = sorted(pinyin.YUNMU_LIST)

phrase_dict = {}


def load_phrase(filename):
    phrasefile = open(filename, "r")
    for line in phrasefile.readlines():
        line = line.rstrip(os.linesep)
        (pinyin_str, freq) = line.split(None, 1)
        freq = int(freq)
        if 0 == freq:
            #print(pinyin_str)
            continue

        # no duplicate here
        if "'" in pinyin_str:
            (first_key, second_key) = pinyin_str.split("'")
            phrase_dict[(first_key, second_key)] = freq
        else:
            phrase_dict[pinyin_str] = freq
    phrasefile.close()


def gen_all_divided():
    for pinyin_key in pinyin_list:
        for first_key in pinyin_list:
            if len(pinyin_key) <= len(first_key):
                continue
            if not pinyin_key.startswith(first_key):
                continue
            second_key = pinyin_key[len(first_key):]
            if second_key in pinyin_list:
                yield pinyin_key, first_key, second_key


def filter_divided():
    for (pinyin_key, first_key, second_key) in gen_all_divided():
        if not (first_key, second_key) in phrase_dict:
            continue
        orig_freq = 0
        if pinyin_key in phrase_dict:
            orig_freq = phrase_dict[pinyin_key]
        new_freq = phrase_dict[(first_key, second_key)]
        yield pinyin_key, orig_freq, first_key, second_key, new_freq


def gen_all_resplit():
    for pinyin_key in pinyin_list:
        if pinyin_key[-1] in ["n", "g", "r"]:
            for yun in yunmu_list:
                if yun not in pinyin_list:
                    continue
                #check first new pinyin key
                if not pinyin_key[:-1] in pinyin_list:
                    continue
                #check second new pinyin key
                new_pinyin_key = pinyin_key[-1] + yun
                if new_pinyin_key in pinyin_list:
                    yield pinyin_key, yun, pinyin_key[:-1], new_pinyin_key
'''
        elif pinyin_key[-1] in ["e"]:
            #check first new pinyin key
            if pinyin_key[:-1] in pinyin_list:
                yield pinyin_key, "r", pinyin_key[:-1], "er"
'''


def filter_resplit():
    for (orig_first_key, orig_second_key, new_first_key, new_second_key) \
    in gen_all_resplit():
        #do the reverse here, as libpinyin pinyin parser is different with
        #ibus-pinyin's parser.
        (orig_first_key, orig_second_key, new_first_key, new_second_key) = \
            (new_first_key, new_second_key, orig_first_key, orig_second_key)
        if (new_first_key, new_second_key) not in phrase_dict:
            continue
        orig_freq = 0
        new_freq = phrase_dict[(new_first_key, new_second_key)]
        if (orig_first_key, orig_second_key) in phrase_dict:
            orig_freq = phrase_dict[(orig_first_key, orig_second_key)]
        yield orig_first_key, orig_second_key, orig_freq, \
        new_first_key, new_second_key, new_freq


#init code
load_phrase("pinyins.txt")
load_phrase("specials.txt")

if __name__ == "__main__":
    for p in filter_divided():
        print (p)
    for p in filter_resplit():
        print (p)
