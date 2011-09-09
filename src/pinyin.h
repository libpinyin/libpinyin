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
#include "pinyin_custom.h"
#include "pinyin_base.h"

using namespace pinyin;

extern "C" {

typedef struct _pinyin_context_t pinyin_context_t;

typedef struct {
    pinyin_context_t * m_context;
    PinyinKeyVector m_pinyin_keys;
    PinyinKeyPosVector m_pinyin_poses;
    CandidateConstraints m_constraints;
    MatchResults m_match_results;
} pinyin_instance_t;

pinyin_context_t * pinyin_init(const char * systemdir, const char * userdir);
bool pinyin_save(pinyin_context_t * context);
void pinyin_fini(pinyin_context_t * context);

bool pinyin_set_options(pinyin_context_t * context,
                        PinyinCustomSettings * custom);

pinyin_instance_t * pinyin_alloc_instance(pinyin_context_t * context);
void pinyin_free_instance(pinyin_instance_t * instance);

bool pinyin_guess_sentence(pinyin_instance_t * instance);

bool pinyin_phrase_segment(pinyin_instance_t * instance,
                           const char * sentence);

bool pinyin_get_sentence(pinyin_instance_t * instance,
                         char ** sentence);

bool pinyin_parse_full_pinyin(pinyin_instance_t * instance,
                              const char * onepinyin,
                              PinyinKey * onekey);
bool pinyin_parse_more_full_pinyins(pinyin_instance_t * instance,
                             const char * pinyins);

bool pinyin_parse_double_pinyin(pinyin_instance_t * instance,
                                const char * onepinyin,
                                PinyinKey * onekey);
bool pinyin_parse_more_double_pinyins(pinyin_instance_t * instance,
                               const char * pinyins);

bool pinyin_parse_chewing(pinyin_instance_t * instance,
                          const char * onechewing,
                          PinyinKey * onekey);
bool pinyin_parse_more_chewings(pinyin_instance_t * instance,
                                const char * chewings);

bool pinyin_get_candidates(pinyin_instance_t * instance,
                           size_t offset,
                           TokenVector candidates);

bool pinyin_choose_candidate(pinyin_instance_t * instance,
                             size_t offset,
                             phrase_token_t token);

bool pinyin_clear_constraint(pinyin_instance_t * instance,
                             size_t offset);
bool pinyin_clear_constraints(pinyin_instance_t * instance);

bool pinyin_translate_token(pinyin_instance_t * instance,
                            phrase_token_t token, char ** word);

bool pinyin_train(pinyin_instance_t * instance);
bool pinyin_reset(pinyin_instance_t * instance);

};

#endif
