# -*- coding: utf-8 -*-
# vim:set et sts=4 sw=4:
#
# libpinyin - Library to deal with pinyin.
#
# Copyright (c) 2010 BYVoid <byvoid1@gmail.com>
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
from operator import itemgetter
from bopomofo import *

def escape_char(ch):
    if ch == "'" or ch == "\\":
        ch = "\\" + ch;
    return "'{0}'".format(ch)


def gen_symbols(keys, symbols):
    items = []
    for (i, key) in enumerate(keys):
        items.append((key, symbols[i]))
    items = sorted(items, key=itemgetter(0))
    entries = []
    for (key, string) in items:
        key = escape_char(key)
        string = '"{0}"'.format(string)
        entry = "{{{0: <5}, {1}}}".format(key, string)
        entries.append(entry)
    entries.append("{'\\0', NULL}")
    return ",\n".join(entries)


#generate symbols here
def gen_chewing_symbols(scheme):
    (begin, end) = BOPOMOFO_SYMBOL_RANGE
    keys = BOPOMOFO_KEYBOARDS[scheme]
    keys = keys[begin:end]
    symbols = BOPOMOFO_SYMBOLS[begin:end]
    return gen_symbols(keys, symbols)


#generate initials here
def gen_chewing_initials(scheme):
    (begin, end) = BOPOMOFO_INITIAL_RANGE
    keys = BOPOMOFO_KEYBOARDS[scheme]
    keys = keys[begin:end]
    symbols = BOPOMOFO_SYMBOLS[begin:end]
    return gen_symbols(keys, symbols)


#generate middles here
def gen_chewing_middles(scheme):
    (begin, end) = BOPOMOFO_MIDDLE_RANGE
    keys = BOPOMOFO_KEYBOARDS[scheme]
    keys = keys[begin:end]
    symbols = BOPOMOFO_SYMBOLS[begin:end]
    return gen_symbols(keys, symbols)


#generate finals here
def gen_chewing_finals(scheme):
    (begin, end) = BOPOMOFO_FINAL_RANGE
    keys = BOPOMOFO_KEYBOARDS[scheme]
    keys = keys[begin:end]
    symbols = BOPOMOFO_SYMBOLS[begin:end]
    return gen_symbols(keys, symbols)


#generate tones here
def gen_chewing_tones(scheme):
    (begin, end) = BOPOMOFO_TONE_RANGE
    keys = BOPOMOFO_KEYBOARDS[scheme]
    keys = keys[begin:end]
    items = []
    for (i, key) in enumerate(keys, start=1):
        items.append((key, i));
    items = sorted(items, key=itemgetter(0))
    entries = []
    for (key, tone) in items:
        key = escape_char(key);
        entry = "{{{0: <5}, {1}}}".format(key, tone)
        entries.append(entry)
    entries.append("{'\\0', 0}")
    return ",\n".join(entries)


### main function ###
if __name__ == "__main__":
    print(gen_chewing_symbols("HSU"), os.linesep)

    print(gen_chewing_initials("HSU"), os.linesep)

    print(gen_chewing_middles("HSU"), os.linesep)

    print(gen_chewing_finals("HSU"), os.linesep)

    print(gen_chewing_tones("HSU"), os.linesep)
