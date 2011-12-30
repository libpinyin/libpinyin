## Makefile.am -- Process this file with automake to produce Makefile.in
## Copyright (C) 2011 Peng Wu
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


textual_model_data	= interpolation.text \
				gb_char.table gbk_char.table

binary_model_data	= gb_char.bin gbk_char.bin \
				phrase_index.bin pinyin_index.bin \
				bigram.db


MAINTAINERCLEANFILES	= Makefile.in

EXTRA_DIST		= $(textual_model_data)

libpinyin_db_DATA 	= $(binary_model_data)

libpinyin_dbdir		= $(pkgdatadir)/data

CLEANFILES		= $(binary_model_data)

interpolation.text:
	wget https://github.com/downloads/libpinyin/libpinyin/model.text.tar.gz
	tar xvf model.text.tar.gz

gb_char.table gbk_char.table: interpolation.text

bigram.db: $(textual_model_data)
	$(RM) $(binary_model_data)
	../utils/storage/gen_binary_files --table-dir $(top_srcdir)/data
	../utils/storage/import_interpolation < $(top_srcdir)/data/interpolation.text
	../utils/training/gen_unigram

gb_char.bin gbk_char.bin phrase_index.bin pinyin_index.bin: bigram.db

rebuild:
	git reset --hard
	sed -i -e "s/#define LAMBDA_PARAMETER 0.330642/#define LAMBDA_PARAMETER $(LAMBDA_PARAMETER)/" ../src/include/novel_types.h
	make -C .. >/dev/null 2>/dev/null
