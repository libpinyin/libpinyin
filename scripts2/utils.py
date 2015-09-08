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
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
# MA  02110-1301, USA.


import os


def shuffle_all(instr):
    for output in shuffle_recur(instr):
        if output == instr:
            continue
        yield output


def shuffle_recur(instr):
    if len(instr) == 1:
        yield instr
    else:
        for i, ch in enumerate(instr):
            recur = instr[:i] + instr[i+1:]
            for s in shuffle_recur(recur):
                yield ch + s


if __name__ == "__main__":
    for s in shuffle_all("abc"):
        print(s)
