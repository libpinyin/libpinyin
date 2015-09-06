# -*- coding: utf-8 -*-
# vim:set et sts=4 sw=4:
#
# libpinyin - Library to deal with pinyin.
#
# Copyright (C) 2013 Peng Wu <alexepico@gmail.com>
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

BOPOMOFO_SYMBOLS = [
    'ㄅ', 'ㄆ', 'ㄇ', 'ㄈ', 'ㄉ', 'ㄊ', 'ㄋ', 'ㄌ', 'ㄍ', 'ㄎ',
    'ㄏ', 'ㄐ', 'ㄑ', 'ㄒ', 'ㄓ', 'ㄔ', 'ㄕ', 'ㄖ', 'ㄗ', 'ㄘ', 'ㄙ',

    'ㄧ', 'ㄨ', 'ㄩ', 'ㄚ', 'ㄛ', 'ㄜ', 'ㄝ', 'ㄞ', 'ㄟ', 'ㄠ', 'ㄡ',
    'ㄢ', 'ㄣ', 'ㄤ', 'ㄥ', 'ㄦ',

    'ˉ', 'ˊ', 'ˇ', 'ˋ', '˙',
]

#陰平聲不標號, use space key

BOPOMOFO_SYMBOL_RANGE  = (0, -5)
BOPOMOFO_INITIAL_RANGE = (0, 21)
BOPOMOFO_MIDDLE_RANGE  = (21, 24)
BOPOMOFO_FINAL_RANGE   = (24, -5)
BOPOMOFO_TONE_RANGE    = (-5, NONE)

BOPOMOFO_KEYBOARDS = {
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
    "8","i","k",",","9","o","l",".","0","p",";","/","-","[","'","=",
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
    #許氏注音鍵盤
    'HSU':
    (
    "b","p","m","f","d","t","n","l","g","k","h","j","v","c","j","v","c","r","z","a","s",
    "e","x","u","y","h","g","e","i","a","w","o","m","n","k","l","l",
    " ","d","f","j","s",
    ),
    #倚天26鍵注音鍵盤
    'ETEN26':
    (
    "b","p","m","f","d","t","n","l","v","k","h","g","v","c","g","y","c","j","q","w","s",
    "e","x","u","a","o","r","w","i","q","z","p","m","n","t","l","h",
    " ","f","j","k","d",
    ),
    #標準(Dvorak)注音鍵盤
    'DVORAK-STANDARD':
    (
    "1","'","a",";","2",",","o","q",".","e","j","p","u","k","5","y","i","x","f","d","b",
    "g","h","m","8","c","t","w","9","r","n","v","0","l","s","z","[",
    " ","6","3","4","7",
    ),
    #許氏(Dvorak)注音鍵盤
    'DVORAK-HSU':
    (
    "b","p","m","f","d","t","n","l","g","k","h","j","v","c","j","v","c","r","z","a","s",
    "e","x","u","y","h","g","e","i","a","w","o","m","n","k","l","l",
    " ","d","f","j","s",
    ),
    #大千26鍵注音鍵盤
    'DACHEN-CP26':
    (
    "q","q","a","z","w","w","s","x","e","d","c","r","f","v","t","t","g","b","y","h","n",
    "u","j","m","u","i","k","b","i","o","l","m","o","p","l","n","p",
    " ","e","r","d","y",
    ),
}

