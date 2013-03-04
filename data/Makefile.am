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
## Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.


tablefiles		= gb_char.table gbk_char.table \
				merged.table \
				art.table culture.table economy.table \
				geology.table history.table life.table \
				nature.table scitech.table \
				society.table sport.table

binfiles		= ${tablefiles:.table=.bin}


textual_model_data	= interpolation2.text \
				$(tablefiles)


binary_model_data	= phrase_index.bin pinyin_index.bin \
				bigram.db \
				$(binfiles)


MAINTAINERCLEANFILES	= Makefile.in

EXTRA_DIST		= $(textual_model_data)

libpinyin_db_DATA 	= $(binary_model_data)

libpinyin_dbdir		= $(libdir)/libpinyin/data

CLEANFILES		= $(binary_model_data)

interpolation2.text:
	wget http://downloads.sourceforge.net/libpinyin/models/model3.text.tar.gz
	tar xvf model3.text.tar.gz -C $(top_srcdir)/data


$(tablefiles): interpolation2.text

bigram.db: $(textual_model_data)
	$(RM) $(binary_model_data)
	../utils/storage/gen_binary_files --table-dir $(top_srcdir)/data
	../utils/storage/import_interpolation < $(top_srcdir)/data/interpolation2.text
	../utils/training/gen_unigram

phrase_index.bin pinyin_index.bin $(binfiles): bigram.db

rebuild:
	git reset --hard
	sed -i -e "s/#define LAMBDA_PARAMETER 0.351972/#define LAMBDA_PARAMETER $(LAMBDA_PARAMETER)/" ../src/include/novel_types.h
	make -C .. >/dev/null 2>/dev/null
