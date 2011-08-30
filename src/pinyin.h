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
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */


#ifndef PINYIN_H
#define PINYIN_H

#include <stdio.h>
#include "novel_types.h"
#include "memory_chunk.h"
#include "pinyin_base.h"
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

/* training module */
#include "flexible_ngram.h"

using namespace pinyin;

typedef struct _pinyin_context_t pinyin_context_t;

pinyin_context_t * pinyin_init(const char * systemdir, const char * userdir);
void pinyin_fini(pinyin_context_t * context);

bool pinyin_set_options(pinyin_context_t * context,
                        PinyinCustomSettings * custom);
bool pinyin_set_pinyin_keys(pinyin_context_t * context,
                            PinyinKeyVector pinyin_keys);

bool pinyin_get_guessed_sentence(pinyin_context_t * context,
                                 char ** sentence);
bool pinyin_parse_full(pinyin_context_t * context,
                       const char * onepinyin,
                       PinyinKey * onekey);
bool pinyin_parse_more_fulls(pinyin_context_t * context,
                             const char * pinyins,
                             PinyinKeyVector pinyin_keys);
bool pinyin_parse_double(pinyin_context_t * context,
                         const char * onepinyin,
                         PinyinKey * onekey);
bool pinyin_parse_more_doubles(pinyin_context_t * context,
                               const char * onepinyin,
                               PinyinKeyVector pinyin_keys);

bool pinyin_get_candidates(pinyin_context_t * context,
                           size_t offset, TokenVector candidates);
bool pinyin_choose_candidate(pinyin_context_t * context,
                             size_t offset, phrase_token_t token);

bool pinyin_clear_constraint(pinyin_context_t * context,
                             size_t offset);
bool pinyin_clear_constraints(pinyin_context_t * context);

bool pinyin_phrase_segment(pinyin_context_t * context,
                           const char * sentence,
                           TokenVector tokens);
bool pinyin_translate_token(pinyin_context_t * context,
                            phrase_token_t token, char ** word);

bool pinyin_train(pinyin_context_t * context);
bool pinyin_save(pinyin_context_t * context);
bool pinyin_reset(pinyin_context_t * context);

#endif
