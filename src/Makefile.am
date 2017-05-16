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

AUTOMAKE_OPTIONS 	= gnu
SUBDIRS 		= include storage lookup

EXTRA_DIST		= libpinyin.ver

MAINTAINERCLEANFILES 	= Makefile.in 

CLEANFILES		= *.bak 

ACLOCAL			= aclocal -I $(ac_aux_dir)

INCLUDES                = -I$(top_srcdir)/src \
			  -I$(top_srcdir)/src/include \
			  -I$(top_srcdir)/src/storage \
			  -I$(top_srcdir)/src/lookup \
                          @GLIB2_CFLAGS@

libpinyinincludedir	= $(includedir)/libpinyin-@VERSION@

libpinyininclude_HEADERS = pinyin.h

if ENABLE_LIBZHUYIN
libpinyininclude_HEADERS += zhuyin.h
endif

noinst_HEADERS		= pinyin_internal.h

lib_LTLIBRARIES		= libpinyin.la

noinst_LTLIBRARIES	= libpinyin_internal.la

libpinyin_la_SOURCES	= pinyin.cpp

libpinyin_la_LIBADD	= storage/libstorage.la lookup/liblookup.la @GLIB2_LIBS@

libpinyin_la_LDFLAGS	= -Wl,--version-script=$(srcdir)/libpinyin.ver \
			  -version-info @LT_VERSION_INFO@

if ENABLE_LIBZHUYIN
lib_LTLIBRARIES     += libzhuyin.la

libzhuyin_la_SOURCES    = zhuyin.cpp

libzhuyin_la_LIBADD	= storage/libstorage.la lookup/liblookup.la @GLIB2_LIBS@

libzhuyin_la_LDFLAGS	= -Wl,--version-script=$(srcdir)/libzhuyin.ver \
			  -version-info @LT_VERSION_INFO@
endif

libpinyin_internal_la_SOURCES	= pinyin_internal.cpp

libpinyin_internal_la_LIBADD	= storage/libstorage.la lookup/liblookup.la

## Note:
## As libpinyin internal interface will change, only provides static library
##   to catch errors when compiling instead of running.
