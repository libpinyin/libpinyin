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

MAINTAINERCLEANFILES = Makefile.in

AM_CPPFLAGS = -I$(top_srcdir)/src/include \
              -I$(top_srcdir)/src/storage \
              @GLIB2_CFLAGS@

noinst_HEADERS = lookup.h \
               pinyin_lookup2.h \
               phrase_lookup.h \
               phonetic_lookup.h \
               phonetic_lookup_linear.h \
               phonetic_lookup_heap.h


noinst_LIBRARIES = liblookup.a

liblookup_a_SOURCES = pinyin_lookup2.cpp \
                    phrase_lookup.cpp \
                    lookup.cpp \
                    phonetic_lookup.cpp
