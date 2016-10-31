## Makefile.am -- Process this file with automake to produce Makefile.in
## Copyright (C) 2007 Peng Wu
##
## This program is free software: you can redistribute it and/or modify
## it under the terms of the GNU General Public License as published by
## the Free Software Foundation, either version 3 of the License, or
## (at your option) any later version.
##
## This program is distributed in the hope that it will be useful,
## but WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
## GNU General Public License for more details.
##
## You should have received a copy of the GNU General Public License
## along with this program.  If not, see <http://www.gnu.org/licenses/>.

INCLUDES                = -I$(top_srcdir)/src \
                          -I$(top_srcdir)/src/include \
                          -I$(top_srcdir)/src/storage \
                          -I$(top_srcdir)/src/lookup \
			  -I$(top_srcdir)/utils \
                          @GLIB2_CFLAGS@

LDADD			= ../../src/libpinyin_internal.la @GLIB2_LIBS@

bin_PROGRAMS		= gen_binary_files \
			  import_interpolation

noinst_PROGRAMS		= export_interpolation \
			  gen_pinyin_table

gen_binary_files_SOURCES    = gen_binary_files.cpp

import_interpolation_SOURCES = import_interpolation.cpp

export_interpolation_SOURCES = export_interpolation.cpp

gen_pinyin_table_SOURCES    = gen_pinyin_table.cpp
