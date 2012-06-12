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
from genpinyintable import gen_content_table, \
    gen_pinyin_index, gen_bopomofo_index, \
    gen_chewing_key_table
from genspecialtable import gen_divided_table, gen_resplit_table

def get_table_content(tablename):
    if tablename == 'CONTENT_TABLE':
        return gen_content_table()
    if tablename == 'PINYIN_INDEX':
        return gen_pinyin_index()
    if tablename == 'BOPOMOFO_INDEX':
        return gen_bopomofo_index()
    if tablename == 'DIVIDED_TABLE':
        return gen_divided_table()
    if tablename == 'RESPLIT_TABLE':
        return gen_resplit_table()
    if tablename == 'TABLE_INDEX':
        return gen_chewing_key_table()


### main function ###
if __name__ == "__main__":
    expand_file("pinyin_parser_table.h.in", get_table_content)
