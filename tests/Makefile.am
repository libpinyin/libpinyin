## Makefile.am -- Process this file with automake to produce Makefile.in
## Copyright (C) 2007 Peng Wu
##
## This program is free software; you can redistribute it and/or modify
## it under the terms of the GNU General Public License as published by
## the Free Software Foundation; either version 2, or (at your option)
## any later version.
##
## This program is distributed in the hope that it will be useful,
## but WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
## GNU General Public License for more details.
##
## You should have received a copy of the GNU General Public License
## along with this program; if not, write to the Free Software
## Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

AUTOMAKE_OPTIONS 	= gnu
SUBDIRS 		= include storage lookup

MAINTAINERCLEANFILES 	= Makefile.in 

CLEANFILES		= *.bak 

ACLOCAL			= aclocal -I $(ac_aux_dir)

INCLUDES                = -I$(top_srcdir)/src \
                          -I$(top_srcdir)/src/include \
                          -I$(top_srcdir)/src/storage \
                          -I$(top_srcdir)/src/lookup \
                          @GLIB2_CFLAGS@

noinst_HEADERS          = timer.h \
			  tests_helper.h

noinst_PROGRAMS         = test_pinyin \
			  test_phrase \
			  test_chewing

test_pinyin_SOURCES	= test_pinyin.cpp

test_pinyin_LDADD	= ../src/libpinyin.la @GLIB2_LIBS@

test_phrase_SOURCES	= test_phrase.cpp

test_phrase_LDADD	= ../src/libpinyin.la @GLIB2_LIBS@

test_chewing_SOURCES	= test_chewing.cpp

test_chewing_LDADD	= ../src/libpinyin.la @GLIB2_LIBS@
