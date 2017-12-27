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
import operator
import itertools
import chewing
from pyzymap import ZHUYIN_PINYIN_MAP, ZHUYIN_LUOMA_PINYIN_MAP, ZHUYIN_SECONDARY_ZHUYIN_MAP
from pyzymap import PINYIN_ZHUYIN_MAP, ZHUYIN_SPECIAL_INITIAL_SET_IN_PINYIN_FORM
from fullpinyin import PINYIN_LIST, SHENGMU_LIST
from options import *
from utils import shuffle_all


#pinyins
pinyin_list = sorted(PINYIN_ZHUYIN_MAP.keys())
shengmu_list = sorted(SHENGMU_LIST)


def check_pinyin_zhuyin_map():
    for pinyin_key in PINYIN_LIST:
        if pinyin_key in pinyin_list:
            pass
        else:
            print("pinyin %s has no chewing mapping", pinyin_key)


def get_chewing(pinyin):
    initial, middle, final = \
        'CHEWING_ZERO_INITIAL', 'CHEWING_ZERO_MIDDLE', 'CHEWING_ZERO_FINAL'
    assert pinyin != None
    assert pinyin in PINYIN_ZHUYIN_MAP

    #handle 'w' and 'y'
    if pinyin[0] == 'w':
        initial = 'PINYIN_W'
    if pinyin[0] == 'y':
        initial = 'PINYIN_Y'

    #get zhuyin string
    zhuyin = PINYIN_ZHUYIN_MAP[pinyin]

    #handle zhuyin ZHUYIN_SPECIAL_INITIAL_SET_IN_PINYIN_FORM
    if pinyin in ZHUYIN_SPECIAL_INITIAL_SET_IN_PINYIN_FORM:
        middle = "CHEWING_I"
    #normal process
    for char in zhuyin:
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
    #generate all pinyins
    for pinyin in pinyin_list:
        flags = []
        if pinyin in PINYIN_ZHUYIN_MAP.keys():
            flags.append("IS_ZHUYIN")
        if pinyin in PINYIN_LIST or \
                pinyin in SHENGMU_LIST:
            flags.append("IS_PINYIN")
        if pinyin in shengmu_list:
            flags.append("PINYIN_INCOMPLETE")
        zhuyin = PINYIN_ZHUYIN_MAP[pinyin]
        if zhuyin in chewing.CHEWING_ASCII_INITIAL_MAP and \
                pinyin not in ZHUYIN_SPECIAL_INITIAL_SET_IN_PINYIN_FORM:
            flags.append("ZHUYIN_INCOMPLETE")
        yield pinyin, pinyin, zhuyin, flags, get_chewing(pinyin)


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
        yield shengmu, shengmu, chewing_initial, flags, chewing_key


def gen_corrects():
    #generate corrections
    for correct, wrong in auto_correct:
        flags = ['IS_PINYIN', 'PINYIN_CORRECT_{0}_{1}'.format(wrong.upper(),
                                                              correct.upper())]
        for pinyin in pinyin_list:
            #fixes partial pinyin instead of the whole pinyin
            if pinyin.endswith(correct) and pinyin != correct:
                zhuyin = PINYIN_ZHUYIN_MAP[pinyin]
                wrong_pinyin = pinyin.replace(correct, wrong)
                yield pinyin, wrong_pinyin, zhuyin,\
                    flags, get_chewing(pinyin)


def gen_u_to_v():
    #generate U to V
    for correct, wrong, flags in auto_correct_ext:
        #over-ride flags
        flags = ['IS_PINYIN', 'PINYIN_CORRECT_V_U']
        pinyin = correct
        zhuyin = PINYIN_ZHUYIN_MAP[pinyin]
        yield correct, wrong, zhuyin, flags, get_chewing(pinyin)


#pinyin table
content_table = []
pinyin_index = []
zhuyin_index = []
shuffle_zhuyin_index = []
luoma_pinyin_index = []
secondary_zhuyin_index = []
hsu_zhuyin_index = []
eten26_zhuyin_index = []


def filter_pinyin_list():
    for (correct, wrong, zhuyin, flags, chewing_key) in gen_pinyin_list():
        (luoma, secondary) = (None, None)

        if zhuyin in ZHUYIN_LUOMA_PINYIN_MAP:
            luoma = ZHUYIN_LUOMA_PINYIN_MAP[zhuyin]

        if zhuyin in ZHUYIN_SECONDARY_ZHUYIN_MAP:
            secondary = ZHUYIN_SECONDARY_ZHUYIN_MAP[zhuyin]

        flags = '|'.join(flags)
        chewing_key = "ChewingKey({0})".format(', '.join(chewing_key))
        #correct = correct.replace("v", "ü")

        content_table.append((correct, zhuyin, luoma, secondary, chewing_key))

        if "IS_PINYIN" in flags:
            pinyin_index.append((wrong, flags, correct))
        #skip pinyin correct options
        if correct != wrong:
            continue
        if luoma:
            luoma_pinyin_index.append((luoma, "IS_PINYIN"))
        if secondary:
            secondary_zhuyin_index.append((secondary, "IS_PINYIN"))
        if "IS_ZHUYIN" in flags:
            zhuyin_index.append((zhuyin, flags))


def populate_more_zhuyin_index():
    for (zhuyin, flags) in zhuyin_index:
        correct = zhuyin
        # populate hsu zhuyin index
        matches = itertools.chain(handle_rules(zhuyin, hsu_correct),
                                  handle_special_rules(zhuyin, hsu_correct_special))
        for wrong in matches:
            newflags = '|'.join((flags, 'ZHUYIN_CORRECT_HSU'))
            hsu_zhuyin_index.append((wrong, newflags, correct))

        # populate eten26 zhuyin index
        matches = itertools.chain(handle_rules(zhuyin, eten26_correct),
                                  handle_special_rules(zhuyin, eten26_correct_special))
        for wrong in matches:
            newflags = '|'.join((flags, 'ZHUYIN_CORRECT_ETEN26'))
            eten26_zhuyin_index.append((wrong, newflags, correct))

    for (zhuyin, flags) in zhuyin_index:
        correct = zhuyin
        # remove duplicate items
        if zhuyin not in [x[0] for x in hsu_zhuyin_index]:
            hsu_zhuyin_index.append((zhuyin, flags, correct))

        if zhuyin not in [x[0] for x in eten26_zhuyin_index]:
            eten26_zhuyin_index.append((zhuyin, flags, correct))

    # populate shuffled zhuyin index
    for (zhuyin, flags) in zhuyin_index:
        correct = zhuyin
        shuffle_zhuyin_index.append((zhuyin, flags, correct))
        newflags = '|'.join((flags, 'ZHUYIN_CORRECT_SHUFFLE'))
        for shuffle in shuffle_all(zhuyin):
            assert shuffle not in [x[0] for x in shuffle_zhuyin_index]
            shuffle_zhuyin_index.append((shuffle, newflags, correct))


def sort_all():
    global content_table, pinyin_index, luoma_pinyin_index
    global zhuyin_index, shuffle_zhuyin_index, secondary_zhuyin_index
    global hsu_zhuyin_index, eten26_zhuyin_index

    #remove duplicates
    content_table = list(set(content_table))
    pinyin_index = list(set(pinyin_index))
    zhuyin_index = list(set(zhuyin_index))
    shuffle_zhuyin_index = list(set(shuffle_zhuyin_index))
    luoma_pinyin_index = list(set(luoma_pinyin_index))
    secondary_zhuyin_index = list(set(secondary_zhuyin_index))
    hsu_zhuyin_index = list(set(hsu_zhuyin_index))
    eten26_zhuyin_index = list(set(eten26_zhuyin_index))

    #define sort function
    sortfunc = operator.itemgetter(0)
    #begin sort
    content_table = sorted(content_table, key=sortfunc)
    #prepend zero item to reserve the invalid item
    content_table.insert(0, ("", "", "", "", "ChewingKey()"))
    #sort index
    pinyin_index = sorted(pinyin_index, key=sortfunc)
    zhuyin_index = sorted(zhuyin_index, key=sortfunc)
    shuffle_zhuyin_index = sorted(shuffle_zhuyin_index, key=sortfunc)
    luoma_pinyin_index = sorted(luoma_pinyin_index, key=sortfunc)
    secondary_zhuyin_index = sorted(secondary_zhuyin_index, key=sortfunc)
    hsu_zhuyin_index = sorted(hsu_zhuyin_index, key=sortfunc)
    eten26_zhuyin_index = sorted(eten26_zhuyin_index, key=sortfunc)

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
    for ((pinyin, zhuyin, luoma, secondary, chewing_key)) in content_table:
        (shengmu, yunmu) = get_sheng_yun(pinyin)
        entry = '{{"{0}", "{1}", "{2}", "{3}", "{4}", "{5}", {6}}}'.format(pinyin, shengmu, yunmu, zhuyin, luoma, secondary, chewing_key)
        entries.append(entry)
    return ',\n'.join(entries)


def gen_pinyin_index():
    entries = []
    for (wrong, flags, correct) in pinyin_index:
        index = [x[0] for x in content_table].index(correct)
        entry = '{{"{0}", {1}, {2}}}'.format(wrong, flags, index)
        entries.append(entry)
    return ',\n'.join(entries)

def gen_luoma_pinyin_index():
    entries = []
    for (luoma, flags) in luoma_pinyin_index:
        index = [x[2] for x in content_table].index(luoma)
        entry = '{{"{0}", {1}, {2}}}'.format(luoma, flags, index)
        entries.append(entry)
    return ',\n'.join(entries)

def gen_secondary_zhuyin_index():
    entries = []
    for (secondary, flags) in secondary_zhuyin_index:
        index = [x[3] for x in content_table].index(secondary)
        entry = '{{"{0}", {1}, {2}}}'.format(secondary, flags, index)
        entries.append(entry)
    return ',\n'.join(entries)

def gen_zhuyin_index():
    entries = []
    for (shuffle, flags, correct) in shuffle_zhuyin_index:
        pinyin = ZHUYIN_PINYIN_MAP[correct]
        index = [x[0] for x in content_table].index(pinyin)
        entry = '{{"{0}", {1}, {2}}}'.format(shuffle, flags, index)
        entries.append(entry)
    return ',\n'.join(entries)

def gen_hsu_zhuyin_index():
    entries = []
    for (wrong, flags, correct) in hsu_zhuyin_index:
        pinyin = ZHUYIN_PINYIN_MAP[correct]
        index = [x[0] for x in content_table].index(pinyin)
        entry = '{{"{0}" /* "{1}" */, {2}, {3}}}'.format \
                (wrong, pinyin, flags, index)
        entries.append(entry)
    return ',\n'.join(entries)

def gen_eten26_zhuyin_index():
    entries = []
    for (wrong, flags, correct) in eten26_zhuyin_index:
        pinyin = ZHUYIN_PINYIN_MAP[correct]
        index = [x[0] for x in content_table].index(pinyin)
        entry = '{{"{0}" /* "{1}" */, {2}, {3}}}'.format \
                (wrong, pinyin, flags, index)
        entries.append(entry)
    return ',\n'.join(entries)

def check_rule(correct, wrong):
    if '*' not in correct:
        assert '*' not in wrong
    elif correct.endswith('*'):
        assert wrong.endswith('*')
    else:
        assert False, "unknown rule format"
    return True

def check_rules(rules, specials):
    for (correct, wrong) in rules:
        check_rule(correct, wrong)
    for (correct, wrong) in specials:
        assert '*' in correct
        check_rule(correct, wrong)

def handle_rules(zhuyin, corrects):
    matches = []
    for (correct, wrong) in corrects:
        if '*' not in correct:
            if correct == zhuyin:
                matches.append(wrong)
        elif correct.endswith('*'):
            starts = correct[0:-1]
            if zhuyin.startswith(starts):
                remained = zhuyin[len(starts):]
                newstr = wrong[0:-1] + remained
                matches.append(newstr)
    return matches

def handle_special_rules(zhuyin, corrects):
    # special rules require additional check m_middle == zero
    matches = []
    if 'ㄧ' in zhuyin:
        return matches
    if 'ㄨ' in zhuyin:
        return matches
    if 'ㄩ' in zhuyin:
        return matches
    # Note: special rules always contains '*'
    return handle_rules(zhuyin, corrects)


def gen_table_index_for_chewing_key():
    chewings = [x[4] for x in content_table]

    entries = []
    for initial in chewing.CHEWING_INITIAL_LIST:
        for middle in chewing.CHEWING_MIDDLE_LIST:
            for final in chewing.CHEWING_FINAL_LIST:
                chewingkey = 'ChewingKey({0}, {1}, {2})'.format(initial, middle, final)
                index = -1
                try:
                    index = chewings.index(chewingkey)
                except ValueError:
                    pass

                entry = '{0:<7} /* {1} */'.format(index, chewingkey)
                entries.append(entry)
    return ",\n".join(entries)


#init full pinyin table code
filter_pinyin_list()
check_rules(hsu_correct, hsu_correct_special)
check_rules(eten26_correct, eten26_correct_special)
populate_more_zhuyin_index()
sort_all()


### main function ###
if __name__ == "__main__":
    #pre-check here
    check_pinyin_zhuyin_map()

    #dump
    for p in gen_pinyin_list():
        print (p)

    s = gen_content_table() + gen_pinyin_index() + gen_zhuyin_index()
    s = gen_content_table() + gen_luoma_pinyin_index() + gen_secondary_zhuyin_index()
    s = gen_hsu_zhuyin_index() + gen_eten26_zhuyin_index()
    s = gen_table_index_for_chewing_key()
    print(s)
