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

AUTOMAKE_OPTIONS = gnu subdir-objects
SUBDIRS = include storage lookup

EXTRA_DIST = libpinyin.ver libzhuyin.ver

MAINTAINERCLEANFILES = Makefile.in

CLEANFILES = *.bak

ACLOCAL = aclocal -I $(ac_aux_dir)

AM_CPPFLAGS = -I$(top_srcdir)/src \
                          -I$(top_srcdir)/src/include \
                          -I$(top_srcdir)/src/storage \
                          -I$(top_srcdir)/src/lookup \
                          @GLIB2_CFLAGS@

libpinyinincludedir	= $(includedir)/libpinyin-@VERSION@

libpinyininclude_HEADERS = pinyin.h

if ENABLE_LIBZHUYIN
libpinyininclude_HEADERS += zhuyin.h
endif

pinyin_SOURCES = \
               storage/phrase_index.cpp \
               storage/phrase_large_table2.cpp \
               storage/phrase_large_table3.cpp \
               storage/ngram.cpp \
               storage/tag_utility.cpp \
               storage/chewing_key.cpp \
               storage/pinyin_parser2.cpp \
               storage/zhuyin_parser2.cpp \
               storage/phonetic_key_matrix.cpp \
               storage/chewing_large_table.cpp \
               storage/chewing_large_table2.cpp \
               storage/table_info.cpp \
               lookup/pinyin_lookup2.cpp \
               lookup/phrase_lookup.cpp \
               lookup/lookup.cpp \
               lookup/phonetic_lookup.cpp \
               $(NULL)

if BERKELEYDB
pinyin_SOURCES += storage/ngram_bdb.cpp \
                  storage/phrase_large_table3_bdb.cpp \
                  storage/chewing_large_table2_bdb.cpp
endif

if KYOTOCABINET
pinyin_SOURCES += storage/ngram_kyotodb.cpp \
                  storage/phrase_large_table3_kyotodb.cpp \
                  storage/chewing_large_table2_kyotodb.cpp
endif


noinst_HEADERS		= pinyin_internal.h

lib_LTLIBRARIES = libpinyin.la

noinst_LIBRARIES = libpinyin_internal.a

libpinyin_la_SOURCES = $(pinyin_SOURCES) pinyin.cpp

libpinyin_la_LIBADD	= @GLIB2_LIBS@

if LLVMLD
## LLVM linker does not support --version-script,
##   use -exported_symbols_list instead
libpinyin_la_LDFLAGS = -Wl,-exported_symbols_list,$(srcdir)/libpinyin.exp \
              -version-info @LT_VERSION_INFO@
else
libpinyin_la_LDFLAGS = -Wl,--version-script=$(srcdir)/libpinyin.ver \
			  -version-info @LT_VERSION_INFO@
endif

if ENABLE_LIBZHUYIN
lib_LTLIBRARIES     += libzhuyin.la

libzhuyin_la_SOURCES = $(pinyin_SOURCES) zhuyin.cpp

libzhuyin_la_LIBADD	= @GLIB2_LIBS@

if LLVMLD
## LLVM linker does not support --version-script,
##   use -exported_symbols_list instead
libzhuyin_la_LDFLAGS = -Wl,-exported_symbols_list=$(srcdir)/libzhuyin.exp \
              -version-info @LT_VERSION_INFO@
else
libzhuyin_la_LDFLAGS = -Wl,--version-script=$(srcdir)/libzhuyin.ver \
			  -version-info @LT_VERSION_INFO@
endif
endif

libpinyin_internal_a_SOURCES = pinyin_internal.cpp

libpinyin_internal_a_LIBADD = storage/libstorage.a lookup/liblookup.a

## Note:
## As libpinyin internal interface will change, only provides static library
##   to catch errors when compiling instead of running.
