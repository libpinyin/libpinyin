/* 
 *  libpinyin
 *  Library to deal with pinyin.
 *  
 *  Copyright (C) 2011 Peng Wu <alexepico@gmail.com>
 *  
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */


#ifndef PINYIN_INTERNAL_H
#define PINYIN_INTERNAL_H

#include <stdio.h>
#include "novel_types.h"
#include "memory_chunk.h"
#include "pinyin_custom2.h"
#include "chewing_key.h"
#include "pinyin_parser2.h"
#include "zhuyin_parser2.h"
#include "phonetic_key_matrix.h"
#include "pinyin_phrase3.h"
#include "chewing_large_table2.h"
#include "phrase_large_table3.h"
#include "facade_chewing_table2.h"
#include "facade_phrase_table3.h"
#include "phrase_index.h"
#include "phrase_index_logger.h"
#include "ngram.h"
#include "lookup.h"
#include "phonetic_lookup.h"
#include "phrase_lookup.h"
#include "tag_utility.h"
#include "table_info.h"


/* training module */
#include "flexible_ngram.h"


/* define filenames */
#define SYSTEM_TABLE_INFO "table.conf"
#define USER_TABLE_INFO "user.conf"
#define SYSTEM_BIGRAM "bigram.db"
#define USER_BIGRAM "user_bigram.db"
#define DELETED_BIGRAM "deleted_bigram.db"
#define SYSTEM_PINYIN_INDEX "pinyin_index.bin"
#define USER_PINYIN_INDEX "user_pinyin_index.bin"
#define SYSTEM_PHRASE_INDEX "phrase_index.bin"
#define USER_PHRASE_INDEX "user_phrase_index.bin"
#define ADDON_SYSTEM_PINYIN_INDEX "addon_pinyin_index.bin"
#define ADDON_SYSTEM_PHRASE_INDEX "addon_phrase_index.bin"


using namespace pinyin;


/* the following fixes build on Debian GNU/kFreeBSD */
#include <errno.h>
#ifndef ENODATA
#define ENODATA ENOENT
#endif


#endif
