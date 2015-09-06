# -*- coding: utf-8 -*-
# vim:set et sts=4 sw=4:
#
# libzhuyin - Library to deal with zhuyin.
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
from bopomofo import BOPOMOFO_HANYU_PINYIN_MAP, BOPOMOFO_LUOMA_PINYIN_MAP, BOPOMOFO_SECONDARY_BOPOMOFO_MAP
from pinyintable import *
from correct import *
from chewingkey import gen_table_index
from utils import shuffle_all


content_table = []
hanyu_pinyin_index = []
luoma_pinyin_index = []
bopomofo_index = []
shuffle_bopomofo_index = []
secondary_bopomofo_index = []
hsu_bopomofo_index = []
eten26_bopomofo_index = []


#pinyin table
def filter_pinyin_list():
    for (pinyin, bopomofo, flags, chewing) in gen_pinyin_list():
        (luoma, second) = (None, None)

        if bopomofo in BOPOMOFO_LUOMA_PINYIN_MAP:
            luoma = BOPOMOFO_LUOMA_PINYIN_MAP[bopomofo]

        if bopomofo in BOPOMOFO_SECONDARY_BOPOMOFO_MAP:
            second = BOPOMOFO_SECONDARY_BOPOMOFO_MAP[bopomofo]

        flags = '|'.join(flags)
        chewing = "ChewingKey({0})".format(', '.join(chewing))
        #correct = correct.replace("v", "ü")

        content_table.append((pinyin, bopomofo, luoma, second, chewing))

        if "IS_PINYIN" in flags:
            hanyu_pinyin_index.append((pinyin, flags))
        if luoma:
            luoma_pinyin_index.append((luoma, "IS_PINYIN"))
        if "IS_BOPOMOFO" in flags:
            bopomofo_index.append((bopomofo, flags))
        if second:
            secondary_bopomofo_index.append((second, "IS_PINYIN"))


def populate_more_bopomofo_index():
    for (bopomofo, flags) in bopomofo_index:
        correct = bopomofo
        # populate hsu bopomofo index
        matches = itertools.chain(handle_rules(bopomofo, hsu_correct),
                                  handle_special_rules(bopomofo, hsu_correct_special))
        for wrong in matches:
            newflags = '|'.join((flags, 'HSU_CORRECT'))
            hsu_bopomofo_index.append((wrong, newflags, correct))

        # populate eten26 bopomofo index
        matches = itertools.chain(handle_rules(bopomofo, eten26_correct),
                                  handle_special_rules(bopomofo, eten26_correct_special))
        for wrong in matches:
            newflags = '|'.join((flags, 'ETEN26_CORRECT'))
            eten26_bopomofo_index.append((wrong, newflags, correct))

    for (bopomofo, flags) in bopomofo_index:
        correct = bopomofo
        # remove duplicate items
        if bopomofo not in [x[0] for x in hsu_bopomofo_index]:
            hsu_bopomofo_index.append((bopomofo, flags, correct))

        if bopomofo not in [x[0] for x in eten26_bopomofo_index]:
            eten26_bopomofo_index.append((bopomofo, flags, correct))

    # populate shuffled bopomofo index
    for (bopomofo, flags) in bopomofo_index:
        correct = bopomofo
        shuffle_bopomofo_index.append((bopomofo, flags, correct))
        newflags = '|'.join((flags, 'SHUFFLE_CORRECT'))
        for shuffle in shuffle_all(bopomofo):
            assert shuffle not in [x[0] for x in shuffle_bopomofo_index]
            shuffle_bopomofo_index.append((shuffle, newflags, correct))


def sort_all():
    global content_table, hanyu_pinyin_index, luoma_pinyin_index
    global bopomofo_index, shuffle_bopomofo_index, secondary_bopomofo_index
    global hsu_bopomofo_index, eten26_bopomofo_index

    #remove duplicates
    content_table = list(set(content_table))
    hanyu_pinyin_index = list(set(hanyu_pinyin_index))
    luoma_pinyin_index = list(set(luoma_pinyin_index))
    bopomofo_index = list(set(bopomofo_index))
    shuffle_bopomofo_index = list(set(shuffle_bopomofo_index))
    secondary_bopomofo_index = list(set(secondary_bopomofo_index))
    hsu_bopomofo_index = list(set(hsu_bopomofo_index))
    eten26_bopomofo_index = list(set(eten26_bopomofo_index))

    #define sort function
    sortfunc = operator.itemgetter(0)
    #begin sort
    content_table = sorted(content_table, key=sortfunc)
    #prepend zero item to reserve the invalid item
    content_table.insert(0, ("", "", "", "", "ChewingKey()"))
    #sort index
    hanyu_pinyin_index = sorted(hanyu_pinyin_index, key=sortfunc)
    luoma_pinyin_index = sorted(luoma_pinyin_index, key=sortfunc)
    bopomofo_index = sorted(bopomofo_index, key=sortfunc)
    shuffle_bopomofo_index = sorted(shuffle_bopomofo_index, key=sortfunc)
    secondary_bopomofo_index = sorted(secondary_bopomofo_index, key=sortfunc)
    hsu_bopomofo_index = sorted(hsu_bopomofo_index, key=sortfunc)
    eten26_bopomofo_index = sorted(eten26_bopomofo_index, key=sortfunc)

'''
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
'''

def gen_content_table():
    entries = []
    for ((pinyin, bopomofo, luoma, second, chewing)) in content_table:
        entry = '{{"{0}", "{1}", "{2}", "{3}" ,{4}}}'.format(pinyin, bopomofo, luoma, second, chewing)
        entries.append(entry)
    return ',\n'.join(entries)


def gen_hanyu_pinyin_index():
    entries = []
    for (pinyin, flags) in hanyu_pinyin_index:
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

def gen_bopomofo_index():
    entries = []
    for (shuffle, flags, correct) in shuffle_bopomofo_index:
        pinyin = BOPOMOFO_HANYU_PINYIN_MAP[correct]
        index = [x[0] for x in content_table].index(pinyin)
        entry = '{{"{0}", {1}, {2}}}'.format(shuffle, flags, index)
        entries.append(entry)
    return ',\n'.join(entries)

def gen_secondary_bopomofo_index():
    entries = []
    for (bopomofo, flags) in secondary_bopomofo_index:
        index = [x[3] for x in content_table].index(bopomofo)
        entry = '{{"{0}", {1}, {2}}}'.format(bopomofo, flags, index)
        entries.append(entry)
    return ',\n'.join(entries)

def gen_hsu_bopomofo_index():
    entries = []
    for (wrong, flags, correct) in hsu_bopomofo_index:
        pinyin = BOPOMOFO_HANYU_PINYIN_MAP[correct]
        index = [x[0] for x in content_table].index(pinyin)
        entry = '{{"{0}" /* "{1}" */, {2}, {3}}}'.format \
                (wrong, pinyin, flags, index)
        entries.append(entry)
    return ',\n'.join(entries)

def gen_eten26_bopomofo_index():
    entries = []
    for (wrong, flags, correct) in eten26_bopomofo_index:
        pinyin = BOPOMOFO_HANYU_PINYIN_MAP[correct]
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

def gen_chewing_key_table():
    return gen_table_index(content_table)


#init code
filter_pinyin_list()
check_rules(hsu_correct, hsu_correct_special)
check_rules(eten26_correct, eten26_correct_special)
populate_more_bopomofo_index()
sort_all()


### main function ###
if __name__ == "__main__":
    #s = gen_content_table() + gen_hanyu_pinyin_index() + gen_bopomofo_index()
    #s = gen_content_table() + gen_luoma_pinyin_index() + gen_secondary_bopomofo_index()
    s = gen_hsu_bopomofo_index() + gen_eten26_bopomofo_index()
    #s = gen_chewing_key_table()
    print(s)
