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


from operator import itemgetter
from utils import expand_file

bopomofo = [
    'ㄅ', 'ㄆ', 'ㄇ', 'ㄈ', 'ㄉ', 'ㄊ', 'ㄋ', 'ㄌ', 'ㄍ', 'ㄎ',
    'ㄏ', 'ㄐ', 'ㄑ', 'ㄒ', 'ㄓ', 'ㄔ', 'ㄕ', 'ㄖ', 'ㄗ', 'ㄘ', 'ㄙ',

    'ㄧ', 'ㄨ', 'ㄩ', 'ㄚ', 'ㄛ', 'ㄜ', 'ㄝ', 'ㄞ', 'ㄟ', 'ㄠ', 'ㄡ',
    'ㄢ', 'ㄣ', 'ㄤ', 'ㄥ', 'ㄦ',

    'ˉ', 'ˊ', 'ˇ', 'ˋ', '˙',
]

#陰平聲不標號, use space key
num_tones = -5

bopomofo_keyboards = {
    #標準注音鍵盤
    'STANDARD':
    (
    "1","q","a","z","2","w","s","x","e","d","c","r","f","v","5","t","g","b","y","h","n",
    "u","j","m","8","i","k",",","9","o","l",".","0","p",";","/","-",
    " ","6","3","4","7",
    ),
    #精業注音鍵盤
    'GINYIEH':
    (
    "2","w","s","x","3","e","d","c","r","f","v","t","g","b","6","y","h","n","u","j","m",
    "-","[","'","8","i","k",",","9","o","l",".","0","p",";","/","=",
    " ","q","a","z","1",
    ),
    #倚天注音鍵盤
    'ETEN':
    (
    "b","p","m","f","d","t","n","l","v","k","h","g","7","c",",",".","/","j",";","'","s",
    "e","x","u","a","o","r","w","i","q","z","y","8","9","0","-","=",
    " ","2","3","4","1",
    ),
    #IBM注音鍵盤
    'IBM':
    (
    "1","2","3","4","5","6","7","8","9","0","-","q","w","e","r","t","y","u","i","o","p",
    "a","s","d","f","g","h","j","k","l",";","z","x","c","v","b","n",
    " ","m",",",".","/",
    ),
}


def escape_char(ch):
    if ch == "'" or ch == "\\":
        ch = "\\" + ch;
    return "'{0}'".format(ch)


#generate shengmu and yunmu here
def gen_chewing_symbols(scheme):
    keyboard = bopomofo_keyboards[scheme]
    keyboard = keyboard[: num_tones]
    items = []
    for (i, key) in enumerate(keyboard):
        items.append((key, bopomofo[i]))
    items = sorted(items, key=itemgetter(0))
    entries = []
    for (key, string) in items:
        key = escape_char(key)
        string = '"{0}"'.format(string)
        entry = "{{{0: <5}, {1}}}".format(key, string)
        entries.append(entry)
    entries.append("{'\\0', NULL}")
    return ",\n".join(entries)


#generate tones here
def gen_chewing_tones(scheme):
    keyboard = bopomofo_keyboards[scheme]
    keyboard = keyboard[num_tones:]
    items = []
    for (i, key) in enumerate(keyboard, start=1):
        items.append((key, i));
    items = sorted(items, key=itemgetter(0))
    entries = []
    for (key, tone) in items:
        key = escape_char(key);
        entry = "{{{0: <5}, {1}}}".format(key, tone)
        entries.append(entry)
    entries.append("{'\\0', 0}")
    return ",\n".join(entries)


def get_table_content(tablename):
    (scheme, part) = tablename.split('_', 1)
    if part == "SYMBOLS":
        return gen_chewing_symbols(scheme);
    if part == "TONES":
        return gen_chewing_tones(scheme);


### main function ###
if __name__ == "__main__":
    expand_file("chewing_table.h.in", get_table_content)
