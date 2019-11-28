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

MAINTAINERCLEANFILES    = Makefile.in

AM_CPPFLAGS = -I$(top_srcdir)/src \
              -I$(top_srcdir)/src/include \
              -I$(top_srcdir)/src/storage \
              -I$(top_srcdir)/src/lookup \
              -I$(top_srcdir)/utils \
              @GLIB2_CFLAGS@

LDADD = ../../src/libpinyin_internal.a \
        ../../src/storage/libstorage.a \
        ../../src/lookup/liblookup.a \
        @GLIB2_LIBS@

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

gen_deleted_ngram_SOURCES = gen_deleted_ngram.cpp

gen_unigram_SOURCES     = gen_unigram.cpp

gen_k_mixture_model_SOURCES = gen_k_mixture_model.cpp

estimate_interpolation_SOURCES = estimate_interpolation.cpp

estimate_k_mixture_model_SOURCES = estimate_k_mixture_model.cpp

merge_k_mixture_model_SOURCES = merge_k_mixture_model.cpp

prune_k_mixture_model_SOURCES = prune_k_mixture_model.cpp

import_k_mixture_model_SOURCES = import_k_mixture_model.cpp

export_k_mixture_model_SOURCES = export_k_mixture_model.cpp

k_mixture_model_to_interpolation_SOURCES = k_mixture_model_to_interpolation.cpp

validate_k_mixture_model_SOURCES = validate_k_mixture_model.cpp

eval_correction_rate_SOURCES = eval_correction_rate.cpp
