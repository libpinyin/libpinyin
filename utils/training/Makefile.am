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

INCLUDES		= -I$(top_srcdir)/src \
			  -I$(top_srcdir)/src/include \
			  -I$(top_srcdir)/src/storage \
			  -I$(top_srcdir)/src/lookup \
			  @GLIB2_CPPFLAGS@

noinst_HEADERS		= k_mixture_model.h

bin_PROGRAMS		= gen_unigram

noinst_PROGRAMS		= gen_ngram \
			  gen_deleted_ngram \
			  gen_k_mixture_model \
			  estimate_interpolation \
			  estimate_k_mixture_model \
			  merge_k_mixture_model \
			  prune_k_mixture_model \
			  import_k_mixture_model \
			  export_k_mixture_model \
			  k_mixture_model_to_interpolation \
			  validate_k_mixture_model \
			  eval_correction_rate

gen_ngram_SOURCES	= gen_ngram.cpp

gen_ngram_LDADD		= ../../src/libpinyin_internal.la @GLIB2_LDFLAGS@

gen_deleted_ngram_SOURCES = gen_deleted_ngram.cpp

gen_deleted_ngram_LDADD = ../../src/libpinyin_internal.la @GLIB2_LDFLAGS@

gen_unigram_SOURCES     = gen_unigram.cpp

gen_unigram_LDADD       = ../../src/libpinyin_internal.la @GLIB2_LDFLAGS@

gen_k_mixture_model_SOURCES = gen_k_mixture_model.cpp

gen_k_mixture_model_LDADD = ../../src/libpinyin_internal.la @GLIB2_LDFLAGS@

estimate_interpolation_SOURCES = estimate_interpolation.cpp

estimate_interpolation_LDADD = ../../src/libpinyin_internal.la @GLIB2_LDFLAGS@

estimate_k_mixture_model_SOURCES = estimate_k_mixture_model.cpp

estimate_k_mixture_model_LDADD = ../../src/libpinyin_internal.la \
					@GLIB2_LDFLAGS@

merge_k_mixture_model_SOURCES = merge_k_mixture_model.cpp

merge_k_mixture_model_LDADD = ../../src/libpinyin_internal.la @GLIB2_LDFLAGS@

prune_k_mixture_model_SOURCES = prune_k_mixture_model.cpp

prune_k_mixture_model_LDADD = ../../src/libpinyin_internal.la @GLIB2_LDFLAGS@

import_k_mixture_model_SOURCES = import_k_mixture_model.cpp

import_k_mixture_model_LDADD = ../../src/libpinyin_internal.la @GLIB2_LDFLAGS@

export_k_mixture_model_SOURCES = export_k_mixture_model.cpp

export_k_mixture_model_LDADD = ../../src/libpinyin_internal.la @GLIB2_LDFLAGS@

k_mixture_model_to_interpolation_SOURCES = k_mixture_model_to_interpolation.cpp

k_mixture_model_to_interpolation_LDADD = ../../src/libpinyin_internal.la \
						@GLIB2_LDFLAGS@

validate_k_mixture_model_SOURCES = validate_k_mixture_model.cpp

validate_k_mixture_model_LDADD = ../../src/libpinyin_internal.la \
					@GLIB2_LDFLAGS@

eval_correction_rate_SOURCES = eval_correction_rate.cpp

eval_correction_rate_LDADD = ../../src/libpinyin_internal.la @GLIB2_LDFLAGS@
