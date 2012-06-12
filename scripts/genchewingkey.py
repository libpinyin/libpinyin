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


from utils import expand_file
from chewingkey import gen_initials, gen_middles, gen_finals, gen_tones


def get_table_content(tablename):
    if tablename == 'CHEWING_INITIAL':
        return gen_initials()
    if tablename == 'CHEWING_MIDDLE':
        return gen_middles()
    if tablename == 'CHEWING_FINAL':
        return gen_finals()
    if tablename == 'CHEWING_TONE':
        return gen_tones()


### main function ###
if __name__ == "__main__":
    expand_file("chewing_enum.h.in", get_table_content)

