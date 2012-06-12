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
import bopomofo
import chewing
import itertools
from correct import *


pinyin_list = sorted(bopomofo.PINYIN_BOPOMOFO_MAP.keys())
shengmu_list = sorted(pinyin.SHENGMU_LIST)


def check_pinyin_chewing_map():
    for pinyin_key in pinyin.PINYIN_DICT.keys():
        if pinyin_key in pinyin_list:
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

    #handle bopomofo SPECIAL_INITIAL_SET
    if pinyin_key in bopomofo.SPECIAL_INITIAL_SET:
        middle = "CHEWING_I"
    #normal process
    for char in bopomofo_str:
        if char in chewing.CHEWING_ASCII_INITIAL_MAP:
            initial = chewing.CHEWING_ASCII_INITIAL_MAP[char]
        if char in chewing.CHEWING_ASCII_MIDDLE_MAP:
            middle = chewing.CHEWING_ASCII_MIDDLE_MAP[char]
        if char in chewing.CHEWING_ASCII_FINAL_MAP:
            final = chewing.CHEWING_ASCII_FINAL_MAP[char]
        if char == "ㄜ":  # merge "ㄝ" and "ㄜ"
            final = "CHEWING_E"

    post_process_rules = {
        #handle "ueng"/"ong"
        ("CHEWING_U", "CHEWING_ENG"): ("CHEWING_ZERO_MIDDLE", "PINYIN_ONG"),
        #handle "veng"/"iong"
        ("CHEWING_V", "CHEWING_ENG"): ("CHEWING_I", "PINYIN_ONG"),
        #handle "ien"/"in"
        ("CHEWING_I", "CHEWING_EN"): ("CHEWING_ZERO_MIDDLE", "PINYIN_IN"),
        #handle "ieng"/"ing"
        ("CHEWING_I", "CHEWING_ENG"): ("CHEWING_ZERO_MIDDLE", "PINYIN_ING"),
        }

    if (middle, final) in post_process_rules:
        (middle, final) = post_process_rules[(middle, final)]

    return initial, middle, final


def gen_pinyin_list():
    for p in itertools.chain(gen_pinyins(),
                             gen_shengmu(),
                             gen_corrects(),
                             gen_u_to_v(),
                             ):
        yield p


def gen_pinyins():
    #generate all pinyins in bopomofo
    for pinyin_key in pinyin_list:
        flags = []
        if pinyin_key in bopomofo.PINYIN_BOPOMOFO_MAP.keys():
            flags.append("IS_CHEWING")
        if pinyin_key in pinyin.PINYIN_LIST or \
                pinyin_key in pinyin.SHENGMU_LIST:
            flags.append("IS_PINYIN")
        if pinyin_key in shengmu_list:
            flags.append("PINYIN_INCOMPLETE")
        chewing_key = bopomofo.PINYIN_BOPOMOFO_MAP[pinyin_key]
        if chewing_key in chewing.CHEWING_ASCII_INITIAL_MAP and \
                pinyin_key not in bopomofo.SPECIAL_INITIAL_SET:
            flags.append("CHEWING_INCOMPLETE")
        yield pinyin_key, pinyin_key, chewing_key, \
            flags, get_chewing(pinyin_key)


def get_shengmu_chewing(shengmu):
    assert shengmu in shengmu_list, "Expected shengmu here."
    chewing_key = 'CHEWING_{0}'.format(shengmu.upper())
    if chewing_key in chewing.ASCII_CHEWING_INITIAL_MAP:
        initial = chewing_key
    else:
        initial = 'PINYIN_{0}'.format(shengmu.upper())
    return initial, "CHEWING_ZERO_MIDDLE", "CHEWING_ZERO_FINAL"

def gen_shengmu():
    #generate all shengmu
    for shengmu in shengmu_list:
        if shengmu in pinyin_list:
            continue
        flags = ["IS_PINYIN", "PINYIN_INCOMPLETE"]
        chewing_key = get_shengmu_chewing(shengmu)
        chewing_initial = chewing_key[0]
        if chewing_initial in chewing.ASCII_CHEWING_INITIAL_MAP:
            chewing_initial = chewing.ASCII_CHEWING_INITIAL_MAP[chewing_initial]
        yield shengmu, shengmu, chewing_initial, \
            flags, chewing_key


def gen_corrects():
    #generate corrections
    for correct, wrong in auto_correct:
        flags = ['IS_PINYIN', 'PINYIN_CORRECT_{0}_{1}'.format(wrong.upper(),
                                                              correct.upper())]
        for pinyin_key in pinyin_list:
            #fixes partial pinyin instead of the whole pinyin
            if pinyin_key.endswith(correct) and pinyin_key != correct:
                chewing_key = bopomofo.PINYIN_BOPOMOFO_MAP[pinyin_key]
                new_pinyin_key = pinyin_key.replace(correct, wrong)
                yield pinyin_key, new_pinyin_key, chewing_key,\
                    flags, get_chewing(pinyin_key)


def gen_u_to_v():
    #generate U to V
    for correct, wrong, flags in auto_correct_ext:
        #over-ride flags
        flags = ['IS_PINYIN', 'PINYIN_CORRECT_V_U']
        pinyin_key = correct
        chewing_key = bopomofo.PINYIN_BOPOMOFO_MAP[pinyin_key]
        yield correct, wrong, chewing_key, flags, get_chewing(pinyin_key)

### main function ###
if __name__ == "__main__":
    #pre-check here
    check_pinyin_chewing_map()

    #dump
    for p in gen_pinyin_list():
        print (p)
