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

AM_CPPFLAGS             = -I$(top_srcdir)/src \
                          -I$(top_srcdir)/src/include \
                          -I$(top_srcdir)/src/storage \
                          -I$(top_srcdir)/src/lookup \
                          -I$(top_srcdir)/tests \
                          @GLIB2_CFLAGS@

LDADD			= \
				../../src/libpinyin_internal.a \
				../../src/storage/libstorage.a \
				../../src/lookup/liblookup.a \
				@GLIB2_LIBS@ \
				$(NULL)

TESTS			= test_phrase_index_logger \
			  test_ngram \
			  test_flexible_ngram

noinst_PROGRAMS		= test_phrase_index \
			  test_phrase_index_logger \
			  test_phrase_table \
			  test_ngram \
			  test_flexible_ngram \
			  test_parser2 \
			  test_matrix \
			  test_chewing_table \
			  test_table_info


test_phrase_index_SOURCES = test_phrase_index.cpp

test_phrase_index_logger_SOURCES = test_phrase_index_logger.cpp

test_phrase_table_SOURCES = test_phrase_table.cpp

test_ngram_SOURCES = test_ngram.cpp

test_flexible_ngram_SOURCES = test_flexible_ngram.cpp

test_parser2_SOURCES = test_parser2.cpp

test_matrix_SOURCES = test_matrix.cpp

test_chewing_table_SOURCES    = test_chewing_table.cpp

test_table_info_SOURCES    = test_table_info.cpp
