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
## Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

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
                          @GLIB2_CPPFLAGS@

libpinyinincludedir	= $(includedir)/libpinyin-@LIBPINYIN_BINARY_VERSION@

libpinyininclude_HEADERS= pinyin.h

noinst_HEADERS		= pinyin_internal.h

lib_LTLIBRARIES		= libpinyin.la

noinst_LTLIBRARIES	= libpinyin_internal.la

libpinyin_la_SOURCES	= pinyin.cpp

libpinyin_la_LIBADD	= storage/libstorage.la lookup/liblookup.la

libpinyin_la_LDFLAGS	= -Wl,--version-script=$(srcdir)/libpinyin.ver \
			  -version-info "0:3:0"

libpinyin_internal_la_SOURCES	= pinyin_internal.cpp

libpinyin_internal_la_LIBADD	= storage/libstorage.la lookup/liblookup.la


## Note:
## As libpinyin internal interface will change, only provides static library
##   to catch errors when compiling instead of running.