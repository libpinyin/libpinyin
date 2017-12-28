# -*- coding: utf-8 -*-
# vim:set et sts=4 sw=4:
#
# libpinyin - Library to deal with pinyin.
#
# Copyright (C) 2017 Peng Wu <alexepico@gmail.com>
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
from chewing import *
from pyzymap import ZHUYIN_PINYIN_MAP, PINYIN_ZHUYIN_MAP
from fullpinyintable import get_chewing, content_table

# recursive maps for ChewingKey
zhuyin_maps = {}


# insert into zhuyin_maps
def add_valid_zhuyin(key):
    global zhuyin_maps
    (initial, middle, final, tone) = key

    maps = None

    # handle initial
    if initial not in zhuyin_maps:
        zhuyin_maps[initial] = {}
    maps = zhuyin_maps[initial]

    # handle middle
    if middle not in maps:
        maps[middle] = {}
    maps = maps[middle]

    # handle final
    if final not in maps:
        maps[final] = {}
    maps = maps[final]

    # handle tone
    if tone not in maps:
        maps[tone] = True


# compute zero tone from other tones
def compute_zero_tone():
    global zhuyin_maps

    for initial in zhuyin_maps.keys():
        middle_maps = zhuyin_maps[initial]
        for middle in middle_maps.keys():
            final_maps = middle_maps[middle]
            for final in final_maps.keys():
                tone_maps = final_maps[final]
                # assume all valid zero tones have sub tones
                if len(tone_maps) > 0:
                    tone_maps["CHEWING_ZERO_TONE"] = True


# check whether the zhuyin is valid
def is_valid_zhuyin(key):
    global zhuyin_maps
    (initial, middle, final, tone) = key

    maps = None

    # handle initial
    if initial not in zhuyin_maps:
        return False
    maps = zhuyin_maps[initial]

    # handle middle
    if middle not in maps:
        return False
    maps = maps[middle]

    # handle final
    if final not in maps:
        return False
    maps = maps[final]

    # handle tone
    if tone not in maps:
        return False

    return True


# generate valid_zhuyin_table
def gen_zhuyin_table():
    global zhuyin_maps
    chewings = [x[4] for x in content_table]

    entries = []
    for initial in CHEWING_INITIAL_LIST:
        for middle in CHEWING_MIDDLE_LIST:
            for final in CHEWING_FINAL_LIST:
                zhuyin = ""
                chewingkey = 'ChewingKey({0}, {1}, {2})'.format(initial, middle, final)
                try:
                    index = chewings.index(chewingkey)
                    zhuyin = content_table[index][1]
                except ValueError:
                    zhuyin = chewingkey

                for tone in CHEWING_TONE_LIST:
                    line = ""
                    if tone == "CHEWING_ZERO_TONE":
                        line = "/* {0} */".format(zhuyin) + os.linesep

                    key = (initial, middle, final, tone)
                    if is_valid_zhuyin(key):
                        line += "TRUE"
                    else:
                        line += "FALSE"
                    entries.append(line)
    return ",\n".join(entries)


# get zhuyin key
def get_zhuyin_key(zhuyin):
    # when bopomofo without tone, it means the first tone
    tone = "CHEWING_1"
    last_char = zhuyin[-1]
    if last_char in CHEWING_ASCII_TONE_MAP:
        tone = CHEWING_ASCII_TONE_MAP[last_char]
        zhuyin = zhuyin[:-1]
    pinyin = ZHUYIN_PINYIN_MAP[zhuyin]
    (initial, middle, final) = get_chewing(pinyin)
    return initial, middle, final, tone


def load_table(filename):
    table_file = open(filename, "r")
    for line in table_file.readlines():
        line = line.rstrip(os.linesep)
        (zhuyins, rest) = line.split(None, 1)
        assert " " not in zhuyins

        for zhuyin in zhuyins.split("'"):
            #print(zhuyin)
            key = get_zhuyin_key(zhuyin)
            #print(key)
            add_valid_zhuyin(key)

    table_file.close()

# load zhuyin table
load_table("tsi.table")
compute_zero_tone()
#print(zhuyin_maps)

### main function ###
if __name__ == "__main__":
    print(gen_zhuyin_table())
