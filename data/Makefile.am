## Makefile.am -- Process this file with automake to produce Makefile.in
## Copyright (C) 2011 Peng Wu
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


tablefiles		= gb_char.table gbk_char.table \
				opengram.table merged.table \
				art.table culture.table economy.table \
				geology.table history.table life.table \
				nature.table people.table science.table \
				society.table sport.table technology.table

binfiles		= ${tablefiles:.table=.bin}


textual_model_data	= interpolation2.text \
				$(tablefiles)


binary_model_data	= phrase_index.bin pinyin_index.bin \
				addon_phrase_index.bin addon_pinyin_index.bin \
				bigram.db \
				$(binfiles)


MAINTAINERCLEANFILES	= Makefile.in

EXTRA_DIST		= $(textual_model_data) \
                          table.conf.in

libpinyin_db_DATA 	= $(binary_model_data) \
                          table.conf

libpinyin_dbdir		= $(libdir)/libpinyin/data

CLEANFILES		= $(binary_model_data)

interpolation2.text:
	wget http://downloads.sourceforge.net/libpinyin/models/model19.text.tar.gz
	tar xvf model19.text.tar.gz -C $(top_srcdir)/data


$(tablefiles) table.conf: interpolation2.text

bigram.db: $(textual_model_data)
	$(RM) $(binary_model_data)
	../utils/storage/gen_binary_files --table-dir $(top_srcdir)/data
	../utils/storage/import_interpolation --table-dir $(top_srcdir)/data < $(top_srcdir)/data/interpolation2.text
	../utils/training/gen_unigram --table-dir $(top_srcdir)/data

addon_phrase_index.bin phrase_index.bin addon_pinyin_index.bin pinyin_index.bin $(binfiles): bigram.db

modify:
	git reset --hard
	sed -i -r -e "s'lambda parameter:0\\.[0-9]{3,6}'lambda parameter:$(LAMBDA_PARAMETER)'" table.conf
