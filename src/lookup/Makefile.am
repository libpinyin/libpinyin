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

MAINTAINERCLEANFILES    = Makefile.in

INCLUDES		= -I$(top_srcdir)/src/include \
			  -I$(top_srcdir)/src/storage \
			  @GLIB2_CPPFLAGS@

noinst_HEADERS		= lookup.h \
			  pinyin_lookup.h \
			  phrase_lookup.h \
			  winner_tree.h

noinst_LTLIBRARIES	= liblookup.la

liblookup_la_CXXFLAGS	= "-fPIC"

liblookup_la_LDFLAGS	= -static

liblookup_la_SOURCES	= pinyin_lookup.cpp \
			  winner_tree.cpp \
			  phrase_lookup.cpp \
			  lookup.cpp
