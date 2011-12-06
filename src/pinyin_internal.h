/* 
 *  libpinyin
 *  Library to deal with pinyin.
 *  
 *  Copyright (C) 2011 Peng Wu <alexepico@gmail.com>
 *  
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 * 
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *  
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */


#ifndef PINYIN_INTERNAL_H
#define PINYIN_INTERNAL_H

#include "pinyin.h"
#include "memory_chunk.h"

#include "pinyin_phrase.h"
#include "pinyin_large_table.h"
#include "phrase_large_table.h"
#include "phrase_index.h"
#include "phrase_index_logger.h"
#include "ngram.h"
#include "lookup.h"
#include "pinyin_lookup.h"
#include "phrase_lookup.h"
#include "tag_utility.h"
#include "pinyin_custom2.h"
#include "pinyin_parser2.h"
#include "chewing_large_table.h"
#include "facade_chewing_table.h"
#include "facade_phrase_table.h"


/* training module */
#include "flexible_ngram.h"

using namespace pinyin;


#endif
