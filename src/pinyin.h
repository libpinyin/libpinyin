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


#ifndef PINYIN_H
#define PINYIN_H

#include <stdio.h>
#include "novel_types.h"
#include "pinyin_custom2.h"
#include "chewing_key.h"
#include "pinyin_parser2.h"

using namespace pinyin;

extern "C" {

typedef struct _pinyin_context_t pinyin_context_t;
typedef struct _pinyin_instance_t pinyin_instance_t;
typedef struct _lookup_candidate_t lookup_candidate_t;

typedef GArray * CandidateVector; /* GArray of lookup_candidate_t */

enum lookup_candidate_type_t{
    NORMAL_CANDIDATE = 1,
    DIVIDED_CANDIDATE,
    RESPLIT_CANDIDATE
};

struct _lookup_candidate_t{
    enum lookup_candidate_type_t m_candidate_type;
    phrase_token_t m_token;
    ChewingKeyRest m_orig_rest;
    gchar * m_new_pinyins;
    guint32 m_freq; /* the amplifed gfloat numerical value. */
public:
    _lookup_candidate_t() {
        m_candidate_type = NORMAL_CANDIDATE;
        m_token = null_token;
        m_new_pinyins = NULL;
        m_freq = 0;
    }
};

struct _pinyin_instance_t{
    pinyin_context_t * m_context;
    gchar * m_raw_full_pinyin;
    TokenVector m_prefixes;
    ChewingKeyVector m_pinyin_keys;
    ChewingKeyRestVector m_pinyin_key_rests;
    CandidateConstraints m_constraints;
    MatchResults m_match_results;
};

/**
 * pinyin_init:
 * @systemdir: the system wide language model data directory.
 * @userdir: the user's language model data directory.
 * @returns: the newly created pinyin context, NULL if failed.
 *
 * Create a new pinyin context.
 *
 */
pinyin_context_t * pinyin_init(const char * systemdir, const char * userdir);

/**
 * pinyin_save:
 * @context: the pinyin context to be saved into user directory.
 * @returns: whether the save succeeded.
 *
 * Save the user's self-learning information of the pinyin context.
 *
 */
bool pinyin_save(pinyin_context_t * context);

/**
 * pinyin_set_double_pinyin_scheme:
 * @context: the pinyin context.
 * @scheme: the double pinyin scheme.
 * @returns: whether the set double pinyin scheme succeeded.
 *
 * Change the double pinyin scheme of the pinyin context.
 *
 */
bool pinyin_set_double_pinyin_scheme(pinyin_context_t * context,
                                     DoublePinyinScheme scheme);

/**
 * pinyin_set_chewing_scheme:
 * @context: the pinyin context.
 * @scheme: the chewing scheme.
 * @returns: whether the set chewing scheme succeeded.
 *
 * Change the chewing scheme of the pinyin context.
 *
 */
bool pinyin_set_chewing_scheme(pinyin_context_t * context,
                               ChewingScheme scheme);

/**
 * pinyin_fini:
 * @context: the pinyin context.
 *
 * Finalize the pinyin context.
 *
 */
void pinyin_fini(pinyin_context_t * context);


/**
 * pinyin_set_options:
 * @context: the pinyin context.
 * @options: the pinyin options of the pinyin context.
 * @returns: whether the set options scheme succeeded.
 *
 * Set the options of the pinyin context.
 *
 */
bool pinyin_set_options(pinyin_context_t * context,
                        pinyin_option_t options);

/**
 * pinyin_alloc_instance:
 * @context: the pinyin context.
 * @returns: the newly allocated pinyin instance, NULL if failed.
 *
 * Allocate a new pinyin instance from the context.
 *
 */
pinyin_instance_t * pinyin_alloc_instance(pinyin_context_t * context);

/**
 * pinyin_free_instance:
 * @instance: the pinyin instance.
 *
 * Free the pinyin instance.
 *
 */
void pinyin_free_instance(pinyin_instance_t * instance);


/**
 * pinyin_guess_sentence:
 * @instance: the pinyin instance.
 * @returns: whether the sentence are guessed successfully.
 *
 * Guess a sentence from the saved pinyin keys in the instance.
 *
 */
bool pinyin_guess_sentence(pinyin_instance_t * instance);

/**
 * pinyin_guess_sentence_with_prefix:
 * @instance: the pinyin instance.
 * @prefix: the prefix before the sentence.
 * @returns: whether the sentence are guessed successfully.
 *
 * Guess a sentence from the saved pinyin keys with a prefix.
 *
 */
bool pinyin_guess_sentence_with_prefix(pinyin_instance_t * instance,
                                       const char * prefix);

/**
 * pinyin_phrase_segment:
 * @instance: the pinyin instance.
 * @sentence: the utf-8 sentence to be segmented.
 * @returns: whether the sentence are segmented successfully.
 *
 * Segment a sentence and saved the result in the instance.
 *
 */
bool pinyin_phrase_segment(pinyin_instance_t * instance,
                           const char * sentence);

/**
 * pinyin_get_sentence:
 * @instance: the pinyin instance.
 * @sentence: the saved sentence in the instance.
 * @returns: whether the sentence is already saved in the instance.
 *
 * Get the sentence from the instance.
 *
 * Note: the returned sentence should be freed by g_free().
 *
 */
bool pinyin_get_sentence(pinyin_instance_t * instance,
                         char ** sentence);

/**
 * pinyin_parse_full_pinyin:
 * @instance: the pinyin instance.
 * @onepinyin: a single full pinyin to be parsed.
 * @onekey: the parsed key.
 * @returns: whether the parse is successfully.
 *
 * Parse a single full pinyin.
 *
 */
bool pinyin_parse_full_pinyin(pinyin_instance_t * instance,
                              const char * onepinyin,
                              ChewingKey * onekey);

/**
 * pinyin_parse_more_full_pinyins:
 * @instance: the pinyin instance.
 * @pinyins: the full pinyins to be parsed.
 * @returns: the parsed length of the full pinyins.
 *
 * Parse multiple full pinyins and save it in the instance.
 *
 */
size_t pinyin_parse_more_full_pinyins(pinyin_instance_t * instance,
                                      const char * pinyins);

/**
 * pinyin_parse_double_pinyin:
 * @instance: the pinyin instance.
 * @onepinyin: the single double pinyin to be parsed.
 * @onekey: the parsed key.
 * @returns: whether the parse is successfully.
 *
 * Parse a single double pinyin.
 *
 */
bool pinyin_parse_double_pinyin(pinyin_instance_t * instance,
                                const char * onepinyin,
                                ChewingKey * onekey);

/**
 * pinyin_parse_more_double_pinyins:
 * @instance: the pinyin instance.
 * @pinyins: the double pinyins to be parsed.
 * @returns: the parsed length of the double pinyins.
 *
 * Parse multiple double pinyins and save it in the instance.
 *
 */
size_t pinyin_parse_more_double_pinyins(pinyin_instance_t * instance,
                                        const char * pinyins);

/**
 * pinyin_parse_chewing:
 * @instance: the pinyin instance.
 * @onechewing: the single chewing to be parsed.
 * @onekey: the parsed key.
 * @returns: whether the parse is successfully.
 *
 * Parse a single chewing.
 *
 */
bool pinyin_parse_chewing(pinyin_instance_t * instance,
                          const char * onechewing,
                          ChewingKey * onekey);

/**
 * pinyin_parse_more_chewings:
 * @instance: the pinyin instance.
 * @chewings: the chewings to be parsed.
 * @returns: the parsed length of the chewings.
 *
 * Parse multiple chewings and save it in the instance.
 *
 */
size_t pinyin_parse_more_chewings(pinyin_instance_t * instance,
                                  const char * chewings);

/**
 * pinyin_in_chewing_keyboard:
 * @instance: the pinyin instance.
 * @key: the input key.
 * @symbol: the chewing symbol.
 * @returns: whether the key is in current chewing scheme.
 *
 * Check whether the input key is in current chewing scheme.
 *
 */
bool pinyin_in_chewing_keyboard(pinyin_instance_t * instance,
                                const char key, const char ** symbol);
/**
 * pinyin_get_candidates:
 * @instance: the pinyin instance.
 * @offset: the offset in the pinyin keys.
 * @candidates: The GArray of token candidates.
 * @returns: whether a list of tokens are gotten.
 *
 * Get the candidates at the offset.
 *
 */
bool pinyin_get_candidates(pinyin_instance_t * instance,
                           size_t offset,
                           TokenVector candidates);

/**
 * pinyin_get_full_pinyin_candidates:
 * @instance: the pinyin instance.
 * @offset: the offset in the pinyin keys.
 * @candidates: the GArray of lookup_candidate_t candidates.
 * @returns: whether a list of lookup_candidate_t candidates are gotten.
 *
 * Get the full pinyin candidates at the offset.
 *
 */
bool pinyin_get_full_pinyin_candidates(pinyin_instance_t * instance,
                                       size_t offset,
                                       CandidateVector candidates);

/**
 * pinyin_choose_candidate:
 * @instance: the pinyin instance.
 * @offset: the offset in the pinyin keys.
 * @token: the selected candidate.
 * @returns: the cursor after the chosen candidate.
 *
 * Choose an candidate at the offset.
 *
 */
int pinyin_choose_candidate(pinyin_instance_t * instance,
                            size_t offset,
                            phrase_token_t token);

/**
 * pinyin_choose_full_pinyin_candidate:
 * @instance: the pinyin instance.
 * @offset: the offset in the pinyin keys.
 * @candidate: the selected lookup_candidate_t candidate.
 * @returns: the cursor after the chosen candidate.
 *
 * Choose a full pinyin candidate at the offset.
 *
 */
int pinyin_choose_full_pinyin_candidate(pinyin_instance_t * instance,
                                        size_t offset,
                                        lookup_candidate_t * candidate);

/**
 * pinyin_clear_constraint:
 * @instance: the pinyin instance.
 * @offset: the offset in the pinyin keys.
 * @returns: whether the constraint is cleared.
 *
 * Clear the previous chosen candidate.
 *
 */
bool pinyin_clear_constraint(pinyin_instance_t * instance,
                             size_t offset);

/**
 * pinyin_clear_constraints:
 * @instance: the pinyin instance.
 * @returns: whether the constraints are cleared.
 *
 * Clear all constraints.
 *
 */
bool pinyin_clear_constraints(pinyin_instance_t * instance);

/**
 * pinyin_translate_token:
 * @instance: the pinyin instance.
 * @token: the phrase token.
 * @word: the phrase in utf-8.
 * @returns: whether the token is valid.
 *
 * Translate the token to utf-8 phrase.
 *
 * Note: the returned word should be freed by g_free().
 *
 */
bool pinyin_translate_token(pinyin_instance_t * instance,
                            phrase_token_t token, char ** word);

/**
 * pinyin_train:
 * @instance: the pinyin instance.
 * @returns: whether the sentence is trained.
 *
 * Train the current user input sentence.
 *
 */
bool pinyin_train(pinyin_instance_t * instance);

/**
 * pinyin_reset:
 * @instance: the pinyin instance.
 * @returns: whether the pinyin instance is resetted.
 *
 * Reset the pinyin instance.
 *
 */
bool pinyin_reset(pinyin_instance_t * instance);


/* hack here. */
typedef ChewingKey PinyinKey;
typedef ChewingKeyRest PinyinKeyPos;
typedef ChewingKeyVector PinyinKeyVector;
typedef ChewingKeyRestVector PinyinKeyPosVector;


#define LIBPINYIN_FORMAT_VERSION  "0.6.91"

};

#endif
