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

INCLUDES                = -I$(top_srcdir)/src/include \
			  -I$(top_srcdir)/src/storage \
			  @GLIB2_CFLAGS@

libpinyinincludedir	= $(includedir)/libpinyin-@VERSION@

libpinyininclude_HEADERS= pinyin_custom2.h


noinst_HEADERS		= chewing_enum.h \
			  chewing_key.h \
			  pinyin_parser2.h \
			  phrase_index.h \
			  phrase_index_logger.h \
			  phrase_large_table2.h \
			  ngram.h \
			  flexible_ngram.h \
			  tag_utility.h \
			  pinyin_parser_table.h \
			  double_pinyin_table.h \
			  chewing_table.h \
			  pinyin_phrase2.h \
			  chewing_large_table.h \
			  facade_chewing_table.h \
			  facade_phrase_table2.h \
			  table_info.h


noinst_LTLIBRARIES      = libstorage.la

libstorage_la_CXXFLAGS	= "-fPIC"

libstorage_la_LDFLAGS	= -static

libstorage_la_SOURCES    = phrase_index.cpp \
			   phrase_large_table2.cpp \
			   ngram.cpp \
			   tag_utility.cpp \
			   pinyin_parser2.cpp \
			   chewing_large_table.cpp \
			   table_info.cpp

