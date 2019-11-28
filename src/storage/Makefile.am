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

AM_CPPFLAGS = -I$(top_srcdir)/src/include \
			  -I$(top_srcdir)/src/storage \
			  @GLIB2_CFLAGS@

libpinyinincludedir	= $(includedir)/libpinyin-@VERSION@

libpinyininclude_HEADERS = pinyin_custom2.h

if ENABLE_LIBZHUYIN
libpinyininclude_HEADERS += zhuyin_custom2.h
endif

noinst_HEADERS = chewing_enum.h \
			  chewing_key.h \
			  pinyin_parser2.h \
			  zhuyin_parser2.h \
			  phonetic_key_matrix.h \
			  phrase_index.h \
			  phrase_index_logger.h \
			  phrase_large_table2.h \
			  phrase_large_table3.h \
			  phrase_large_table3_bdb.h \
			  phrase_large_table3_kyotodb.h \
			  ngram.h \
			  ngram_bdb.h \
			  ngram_kyotodb.h \
			  flexible_ngram.h \
			  flexible_single_gram.h \
			  flexible_ngram_bdb.h \
			  flexible_ngram_kyotodb.h \
			  tag_utility.h \
			  pinyin_parser_table.h \
			  special_table.h \
			  double_pinyin_table.h \
			  zhuyin_table.h \
			  pinyin_phrase2.h \
			  pinyin_phrase3.h \
			  chewing_large_table.h \
			  chewing_large_table2.h \
			  chewing_large_table2_bdb.h \
			  chewing_large_table2_kyotodb.h \
			  facade_chewing_table.h \
			  facade_chewing_table2.h \
			  facade_phrase_table2.h \
			  facade_phrase_table3.h \
			  table_info.h \
			  bdb_utils.h \
			  kyotodb_utils.h


noinst_LIBRARIES = libstorage.a

libstorage_a_SOURCES = phrase_index.cpp \
			   phrase_large_table2.cpp \
			   phrase_large_table3.cpp \
			   ngram.cpp \
			   tag_utility.cpp \
			   chewing_key.cpp \
			   pinyin_parser2.cpp \
			   zhuyin_parser2.cpp \
			   phonetic_key_matrix.cpp \
			   chewing_large_table.cpp \
			   chewing_large_table2.cpp \
			   table_info.cpp

if BERKELEYDB
libstorage_a_SOURCES += ngram_bdb.cpp \
			   phrase_large_table3_bdb.cpp \
			   chewing_large_table2_bdb.cpp
endif

if KYOTOCABINET
libstorage_a_SOURCES += ngram_kyotodb.cpp \
			   phrase_large_table3_kyotodb.cpp \
			   chewing_large_table2_kyotodb.cpp
endif
