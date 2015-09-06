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
import itertools
import chewing
from pyzymap import ZHUYIN_PINYIN_MAP, ZHUYIN_LUOMA_PINYIN_MAP, ZHUYIN_SECONDARY_ZHUYIN_MAP
from pyzymap import PINYIN_ZHUYIN_MAP, ZHUYIN_SPECIAL_INITIAL_SET_IN_PINYIN_FORM
from fullpinyin import PINYIN_LIST, SHENGMU_LIST
from options import *
from utils import shuffle_all


content_table = []
pinyin_index = []
luoma_pinyin_index = []
zhuyin_index = []
shuffle_zhuyin_index = []
secondary_zhuyin_index = []
hsu_zhuyin_index = []
eten26_zhuyin_index = []


pinyin_list = sorted(PINYIN_ZHUYIN_MAP.keys())
shengmu_list = sorted(SHENGMU_LIST)


def check_pinyin_zhuyin_map():
    for pinyin_key in PINYIN_LIST:
        if pinyin_key in pinyin_list:
            pass
        else:
            print("pinyin %s has no chewing mapping", pinyin_key)


def get_chewing(pinyin_key):
    initial, middle, final = \
        'CHEWING_ZERO_INITIAL', 'CHEWING_ZERO_MIDDLE', 'CHEWING_ZERO_FINAL'
    assert pinyin_key != None
    assert pinyin_key in PINYIN_ZHUYIN_MAP

    #handle 'w' and 'y'
    if pinyin_key[0] == 'w':
        initial = 'PINYIN_W'
    if pinyin_key[0] == 'y':
        initial = 'PINYIN_Y'

    #get chewing string
    bopomofo_str = PINYIN_ZHUYIN_MAP[pinyin_key]

    #handle bopomofo ZHUYIN_SPECIAL_INITIAL_SET_IN_PINYIN_FORM
    if pinyin_key in ZHUYIN_SPECIAL_INITIAL_SET_IN_PINYIN_FORM:
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
                             ):
        yield p


def gen_pinyins():
    #generate all pinyins in bopomofo
    for pinyin_key in pinyin_list:
        flags = []
        if pinyin_key in PINYIN_ZHUYIN_MAP.keys():
            flags.append("IS_BOPOMOFO")
        if pinyin_key in PINYIN_LIST or \
                pinyin_key in SHENGMU_LIST:
            flags.append("IS_PINYIN")
        if pinyin_key in shengmu_list:
            flags.append("PINYIN_INCOMPLETE")
        chewing_key = PINYIN_ZHUYIN_MAP[pinyin_key]
        if chewing_key in chewing.CHEWING_ASCII_INITIAL_MAP and \
                pinyin_key not in ZHUYIN_SPECIAL_INITIAL_SET_IN_PINYIN_FORM:
            flags.append("CHEWING_INCOMPLETE")
        yield pinyin_key, chewing_key, \
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
        yield shengmu, chewing_initial, \
            flags, chewing_key


def gen_corrects():
    #generate corrections
    for correct, wrong in auto_correct:
        flags = ['IS_PINYIN', 'PINYIN_CORRECT_{0}_{1}'.format(wrong.upper(),
                                                              correct.upper())]
        for pinyin_key in pinyin_list:
            #fixes partial pinyin instead of the whole pinyin
            if pinyin_key.endswith(correct) and pinyin_key != correct:
                chewing_key = PINYIN_ZHUYIN_MAP[pinyin_key]
                new_pinyin_key = pinyin_key.replace(correct, wrong)
                yield pinyin_key, new_pinyin_key, chewing_key,\
                    flags, get_chewing(pinyin_key)


def gen_u_to_v():
    #generate U to V
    for correct, wrong, flags in auto_correct_ext:
        #over-ride flags
        flags = ['IS_PINYIN', 'PINYIN_CORRECT_V_U']
        pinyin_key = correct
        chewing_key = PINYIN_ZHUYIN_MAP[pinyin_key]
        yield correct, wrong, chewing_key, flags, get_chewing(pinyin_key)


#pinyin table
def filter_pinyin_list():
    for (pinyin, bopomofo, flags, chewing) in gen_pinyin_list():
        (luoma, second) = (None, None)

        if bopomofo in ZHUYIN_LUOMA_PINYIN_MAP:
            luoma = ZHUYIN_LUOMA_PINYIN_MAP[bopomofo]

        if bopomofo in ZHUYIN_SECONDARY_ZHUYIN_MAP:
            second = ZHUYIN_SECONDARY_ZHUYIN_MAP[bopomofo]

        flags = '|'.join(flags)
        chewing = "ChewingKey({0})".format(', '.join(chewing))
        #correct = correct.replace("v", "ü")

        content_table.append((pinyin, bopomofo, luoma, second, chewing))

        if "IS_PINYIN" in flags:
            pinyin_index.append((pinyin, flags))
        if luoma:
            luoma_pinyin_index.append((luoma, "IS_PINYIN"))
        if "IS_BOPOMOFO" in flags:
            zhuyin_index.append((bopomofo, flags))
        if second:
            secondary_zhuyin_index.append((second, "IS_PINYIN"))


def populate_more_zhuyin_index():
    for (bopomofo, flags) in zhuyin_index:
        correct = bopomofo
        # populate hsu bopomofo index
        matches = itertools.chain(handle_rules(bopomofo, hsu_correct),
                                  handle_special_rules(bopomofo, hsu_correct_special))
        for wrong in matches:
            newflags = '|'.join((flags, 'HSU_CORRECT'))
            hsu_zhuyin_index.append((wrong, newflags, correct))

        # populate eten26 bopomofo index
        matches = itertools.chain(handle_rules(bopomofo, eten26_correct),
                                  handle_special_rules(bopomofo, eten26_correct_special))
        for wrong in matches:
            newflags = '|'.join((flags, 'ETEN26_CORRECT'))
            eten26_zhuyin_index.append((wrong, newflags, correct))

    for (bopomofo, flags) in zhuyin_index:
        correct = bopomofo
        # remove duplicate items
        if bopomofo not in [x[0] for x in hsu_zhuyin_index]:
            hsu_zhuyin_index.append((bopomofo, flags, correct))

        if bopomofo not in [x[0] for x in eten26_zhuyin_index]:
            eten26_zhuyin_index.append((bopomofo, flags, correct))

    # populate shuffled bopomofo index
    for (bopomofo, flags) in zhuyin_index:
        correct = bopomofo
        shuffle_zhuyin_index.append((bopomofo, flags, correct))
        newflags = '|'.join((flags, 'SHUFFLE_CORRECT'))
        for shuffle in shuffle_all(bopomofo):
            assert shuffle not in [x[0] for x in shuffle_zhuyin_index]
            shuffle_zhuyin_index.append((shuffle, newflags, correct))


def sort_all():
    global content_table, pinyin_index, luoma_pinyin_index
    global zhuyin_index, shuffle_zhuyin_index, secondary_zhuyin_index
    global hsu_zhuyin_index, eten26_zhuyin_index

    #remove duplicates
    content_table = list(set(content_table))
    pinyin_index = list(set(pinyin_index))
    luoma_pinyin_index = list(set(luoma_pinyin_index))
    zhuyin_index = list(set(zhuyin_index))
    shuffle_zhuyin_index = list(set(shuffle_zhuyin_index))
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
    luoma_pinyin_index = sorted(luoma_pinyin_index, key=sortfunc)
    zhuyin_index = sorted(zhuyin_index, key=sortfunc)
    shuffle_zhuyin_index = sorted(shuffle_zhuyin_index, key=sortfunc)
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
    for ((pinyin, bopomofo, luoma, second, chewing)) in content_table:
        (shengmu, yunmu) = get_sheng_yun(pinyin)
        entry = '{{"{0}", "{1}", "{2}", "{3}", "{4}", "{5}" ,{6}}}'.format(pinyin, shengmu, yunmu, bopomofo, luoma, second, chewing)
        entries.append(entry)
    return ',\n'.join(entries)


def gen_pinyin_index():
    entries = []
    for (pinyin, flags) in pinyin_index:
        index = [x[0] for x in content_table].index(pinyin)
        entry = '{{"{0}", {1}, {2}}}'.format(pinyin, flags, index)
        entries.append(entry)
    return ',\n'.join(entries)

def gen_luoma_pinyin_index():
    entries = []
    for (pinyin, flags) in luoma_pinyin_index:
        index = [x[2] for x in content_table].index(pinyin)
        entry = '{{"{0}", {1}, {2}}}'.format(pinyin, flags, index)
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

def gen_secondary_zhuyin_index():
    entries = []
    for (bopomofo, flags) in secondary_zhuyin_index:
        index = [x[3] for x in content_table].index(bopomofo)
        entry = '{{"{0}", {1}, {2}}}'.format(bopomofo, flags, index)
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

def handle_rules(bopomofo, corrects):
    matches = []
    for (correct, wrong) in corrects:
        if '*' not in correct:
            if correct == bopomofo:
                matches.append(wrong)
        elif correct.endswith('*'):
            starts = correct[0:-1]
            if bopomofo.startswith(starts):
                remained = bopomofo[len(starts):]
                newstr = wrong[0:-1] + remained
                matches.append(newstr)
    return matches

def handle_special_rules(bopomofo, corrects):
# special rules require additional check m_middle == zero
    matches = []
    if 'ㄧ' in bopomofo:
        return matches
    if 'ㄨ' in bopomofo:
        return matches
    if 'ㄩ' in bopomofo:
        return matches
# Note: special rules always contains '*'
    return handle_rules(bopomofo, corrects)


def gen_table_index_for_chewing_key(content_table):
    entries = []
    for i in range(0, len(chewing.CHEWING_INITIAL_LIST)):
        initial = chewing.CHEWING_INITIAL_LIST[i]
        for m in range(0, len(chewing.CHEWING_MIDDLE_LIST)):
            middle = chewing.CHEWING_MIDDLE_LIST[m]
            for f in range(0, len(chewing.CHEWING_FINAL_LIST)):
                final = chewing.CHEWING_FINAL_LIST[f]
                chewingkey = 'ChewingKey({0}, {1}, {2})'.format(initial, middle, final)
                index = -1
                try:
                    index = [x[4] for x in content_table].index(chewingkey)
                except ValueError:
                    pass

                entry = '{0:<7} /* {1} */'.format(index, chewingkey)
                entries.append(entry)
    return ",\n".join(entries)


#init code
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
    s = gen_table_index_for_chewing_key(content_table)
    print(s)
