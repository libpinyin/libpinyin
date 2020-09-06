/* 
 *  libpinyin
 *  Library to deal with pinyin.
 *  
 *  Copyright (C) 2017 Peng Wu <alexepico@gmail.com>
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


#ifndef ZHUYIN_H
#define ZHUYIN_H


#include "novel_types.h"
#include "zhuyin_custom2.h"


G_BEGIN_DECLS

typedef struct _ChewingKey ChewingKey;
typedef struct _ChewingKeyRest ChewingKeyRest;

typedef struct _zhuyin_context_t zhuyin_context_t;
typedef struct _zhuyin_instance_t zhuyin_instance_t;
typedef struct _lookup_candidate_t lookup_candidate_t;

typedef struct _import_iterator_t import_iterator_t;

typedef enum _lookup_candidate_type_t{
    BEST_MATCH_CANDIDATE = 1,
    NORMAL_CANDIDATE_AFTER_CURSOR,
    NORMAL_CANDIDATE_BEFORE_CURSOR,
    ZOMBIE_CANDIDATE
} lookup_candidate_type_t;

/**
 * zhuyin_init:
 * @systemdir: the system wide language model data directory.
 * @userdir: the user's language model data directory.
 * @returns: the newly created pinyin context, NULL if failed.
 *
 * Create a new pinyin context.
 *
 */
zhuyin_context_t * zhuyin_init(const char * systemdir, const char * userdir);

/**
 * zhuyin_load_phrase_library:
 * @context: the zhuyin context.
 * @index: the phrase index to be loaded.
 * @returns: whether the load succeeded.
 *
 * Load the sub phrase library of the index.
 *
 */
bool zhuyin_load_phrase_library(zhuyin_context_t * context,
                                guint8 index);

/**
 * zhuyin_unload_phrase_library:
 * @context: the zhuyin context.
 * @index: the phrase index to be unloaded.
 * @returns: whether the unload succeeded.
 *
 * Unload the sub phrase library of the index.
 *
 */
bool zhuyin_unload_phrase_library(zhuyin_context_t * context,
                                  guint8 index);

/**
 * zhuyin_begin_add_phrases:
 * @context: the zhuyin context.
 * @index: the phrase index to be imported.
 * @returns: the import iterator.
 *
 * Begin to add phrases.
 *
 */
import_iterator_t * zhuyin_begin_add_phrases(zhuyin_context_t * context,
                                             guint8 index);

/**
 * zhuyin_iterator_add_phrase:
 * @iter: the import iterator.
 * @phrase: the phrase string.
 * @pinyin: the pinyin string.
 * @count: the count of the phrase/pinyin pair, -1 to use the default value.
 * @returns: whether the add operation succeeded.
 *
 * Add a pair of phrase and pinyin with count.
 *
 */
bool zhuyin_iterator_add_phrase(import_iterator_t * iter,
                                const char * phrase,
                                const char * pinyin,
                                gint count);

/**
 * zhuyin_end_add_phrases:
 * @iter: the import iterator.
 *
 * End adding phrases.
 *
 */
void zhuyin_end_add_phrases(import_iterator_t * iter);

/**
 * zhuyin_save:
 * @context: the zhuyin context to be saved into user directory.
 * @returns: whether the save succeeded.
 *
 * Save the user's self-learning information of the zhuyin context.
 *
 */
bool zhuyin_save(zhuyin_context_t * context);

/**
 * zhuyin_set_chewing_scheme:
 * @context: the zhuyin context.
 * @scheme: the chewing scheme.
 * @returns: whether the set chewing scheme succeeded.
 *
 * Change the chewing scheme of the zhuyin context.
 *
 */
bool zhuyin_set_chewing_scheme(zhuyin_context_t * context,
                               ZhuyinScheme scheme);

/**
 * zhuyin_set_full_pinyin_scheme:
 * @context: the zhuyin context.
 * @scheme: the full pinyin scheme.
 * @returns: whether the set full pinyin scheme succeeded.
 *
 * Change the full pinyin scheme of the zhuyin context.
 *
 */
bool zhuyin_set_full_pinyin_scheme(zhuyin_context_t * context,
                                   FullPinyinScheme scheme);

/**
 * zhuyin_fini:
 * @context: the zhuyin context.
 *
 * Finalize the zhuyin context.
 *
 */
void zhuyin_fini(zhuyin_context_t * context);


/**
 * zhuyin_mask_out:
 * @context: the zhuyin context.
 * @mask: the mask.
 * @value: the value.
 * @returns: whether the mask out operation is successful.
 *
 * Mask out the matched phrase tokens.
 *
 */
bool zhuyin_mask_out(zhuyin_context_t * context,
                     phrase_token_t mask,
                     phrase_token_t value);


/**
 * zhuyin_set_options:
 * @context: the zhuyin context.
 * @options: the pinyin options of the zhuyin context.
 * @returns: whether the set options scheme succeeded.
 *
 * Set the options of the zhuyin context.
 *
 */
bool zhuyin_set_options(zhuyin_context_t * context,
                        pinyin_option_t options);

/**
 * zhuyin_alloc_instance:
 * @context: the zhuyin context.
 * @returns: the newly allocated pinyin instance, NULL if failed.
 *
 * Allocate a new pinyin instance from the context.
 *
 */
zhuyin_instance_t * zhuyin_alloc_instance(zhuyin_context_t * context);

/**
 * zhuyin_free_instance:
 * @instance: the zhuyin instance.
 *
 * Free the zhuyin instance.
 *
 */
void zhuyin_free_instance(zhuyin_instance_t * instance);


/**
 * zhuyin_guess_sentence:
 * @instance: the zhuyin instance.
 * @returns: whether the sentence are guessed successfully.
 *
 * Guess a sentence from the saved pinyin keys in the instance.
 *
 */
bool zhuyin_guess_sentence(zhuyin_instance_t * instance);

/**
 * zhuyin_guess_sentence_with_prefix:
 * @instance: the zhuyin instance.
 * @prefix: the prefix before the sentence.
 * @returns: whether the sentence are guessed successfully.
 *
 * Guess a sentence from the saved pinyin keys with a prefix.
 *
 */
bool zhuyin_guess_sentence_with_prefix(zhuyin_instance_t * instance,
                                       const char * prefix);

/**
 * zhuyin_phrase_segment:
 * @instance: the zhuyin instance.
 * @sentence: the utf-8 sentence to be segmented.
 * @returns: whether the sentence are segmented successfully.
 *
 * Segment a sentence and saved the result in the instance.
 *
 */
bool zhuyin_phrase_segment(zhuyin_instance_t * instance,
                           const char * sentence);

/**
 * zhuyin_get_sentence:
 * @instance: the zhuyin instance.
 * @sentence: the saved sentence in the instance.
 * @returns: whether the sentence is already saved in the instance.
 *
 * Get the sentence from the instance.
 *
 * Note: the returned sentence should be freed by g_free().
 *
 */
bool zhuyin_get_sentence(zhuyin_instance_t * instance,
                         char ** sentence);

/**
 * zhuyin_parse_full_pinyin:
 * @instance: the zhuyin instance.
 * @onepinyin: a single full pinyin to be parsed.
 * @onekey: the parsed key.
 * @returns: whether the parse is successfully.
 *
 * Parse a single full pinyin.
 *
 */
bool zhuyin_parse_full_pinyin(zhuyin_instance_t * instance,
                              const char * onepinyin,
                              ChewingKey * onekey);

/**
 * zhuyin_parse_more_full_pinyins:
 * @instance: the zhuyin instance.
 * @pinyins: the full pinyins to be parsed.
 * @returns: the parsed length of the full pinyins.
 *
 * Parse multiple full pinyins and save it in the instance.
 *
 */
size_t zhuyin_parse_more_full_pinyins(zhuyin_instance_t * instance,
                                      const char * pinyins);

/**
 * zhuyin_parse_chewing:
 * @instance: the zhuyin instance.
 * @onechewing: the single chewing to be parsed.
 * @onekey: the parsed key.
 * @returns: whether the parse is successfully.
 *
 * Parse a single chewing.
 *
 */
bool zhuyin_parse_chewing(zhuyin_instance_t * instance,
                          const char * onechewing,
                          ChewingKey * onekey);

/**
 * zhuyin_parse_more_chewings:
 * @instance: the zhuyin instance.
 * @chewings: the chewings to be parsed.
 * @returns: the parsed length of the chewings.
 *
 * Parse multiple chewings and save it in the instance.
 *
 */
size_t zhuyin_parse_more_chewings(zhuyin_instance_t * instance,
                                  const char * chewings);

/**
 * zhuyin_get_parsed_input_length:
 * @instance: the zhuyin instance.
 * @returns: the parsed_length of the input.
 *
 * Get the parsed length of the input.
 *
 */
size_t zhuyin_get_parsed_input_length(zhuyin_instance_t * instance);


/**
 * zhuyin_in_chewing_keyboard:
 * @instance: the zhuyin instance.
 * @key: the input key.
 * @symbols: the chewing symbols must be freed by g_strfreev.
 * @returns: whether the key is in current chewing scheme.
 *
 * Check whether the input key is in current chewing scheme.
 *
 */
bool zhuyin_in_chewing_keyboard(zhuyin_instance_t * instance,
                                const char key, gchar *** symbols);
/**
 * zhuyin_guess_candidates_after_cursor:
 * @instance: the zhuyin instance.
 * @offset: the offset in the pinyin keys.
 * @returns: whether a list of tokens are gotten.
 *
 * Guess the candidates at the offset.
 *
 */
bool zhuyin_guess_candidates_after_cursor(zhuyin_instance_t * instance,
                                          size_t offset);

/**
 * zhuyin_guess_candidates_before_cursor:
 * @instance: the zhuyin instance.
 * @offset: the offset in the pinyin keys.
 * @returns: whether a list of tokens are gotten.
 *
 * Guess the candidates at the offset.
 *
 */
bool zhuyin_guess_candidates_before_cursor(zhuyin_instance_t * instance,
                                           size_t offset);

/**
 * zhuyin_choose_candidate:
 * @instance: the zhuyin instance.
 * @offset: the offset in the pinyin keys.
 * @candidate: the selected candidate.
 * @returns: the cursor after the chosen candidate.
 *
 * Choose a full pinyin candidate at the offset.
 *
 */
int zhuyin_choose_candidate(zhuyin_instance_t * instance,
                            size_t offset,
                            lookup_candidate_t * candidate);

/**
* zhuyin_clear_constraint:
* @instance: the zhuyin instance.
* @offset: the offset in the pinyin keys.
* @returns: whether the constraint is cleared.
*
* Clear the previous chosen candidate.
*
*/
bool zhuyin_clear_constraint(zhuyin_instance_t * instance,
                             size_t offset);

/**
 * zhuyin_lookup_tokens:
 * @instance: the zhuyin instance.
 * @phrase: the phrase to be looked up.
 * @tokenarray: the returned GArray of tokens.
 * @returns: whether the lookup operation is successful.
 *
 * Lookup the tokens for the phrase utf8 string.
 *
 */
bool zhuyin_lookup_tokens(zhuyin_instance_t * instance,
                          const char * phrase, GArray * tokenarray);

/**
 * zhuyin_train:
 * @instance: the zhuyin instance.
 * @returns: whether the sentence is trained.
 *
 * Train the current user input sentence.
 *
 */
bool zhuyin_train(zhuyin_instance_t * instance);

/**
 * zhuyin_reset:
 * @instance: the zhuyin instance.
 * @returns: whether the zhuyin instance is resetted.
 *
 * Reset the zhuyin instance.
 *
 */
bool zhuyin_reset(zhuyin_instance_t * instance);

/**
 * zhuyin_get_zhuyin_string:
 * @instance: the zhuyin instance.
 * @key: the chewing key.
 * @utf8_str: the zhuyin string.
 * @returns: whether the get operation is successful.
 *
 * Get the zhuyin string of the key.
 *
 */
bool zhuyin_get_zhuyin_string(zhuyin_instance_t * instance,
                              ChewingKey * key,
                              gchar ** utf8_str);

/**
 * zhuyin_get_pinyin_string:
 * @instance: the zhuyin instance.
 * @key: the pinyin key.
 * @utf8_str: the pinyin string.
 * @returns: whether the get operation is successful.
 *
 * Get the pinyin string of the key.
 *
 */
bool zhuyin_get_pinyin_string(zhuyin_instance_t * instance,
                              ChewingKey * key,
                              gchar ** utf8_str);

/**
 * zhuyin_token_get_phrase:
 * @instance: the zhuyin instance.
 * @token: the phrase token.
 * @len: the phrase length.
 * @utf8_str: the phrase string.
 * @returns: whether the get operation is successful.
 *
 * Get the phrase length and utf8 string.
 *
 */
bool zhuyin_token_get_phrase(zhuyin_instance_t * instance,
                             phrase_token_t token,
                             guint * len,
                             gchar ** utf8_str);

/**
 * zhuyin_token_get_n_pronunciation:
 * @instance: the zhuyin instance.
 * @token: the phrase token.
 * @num: the number of pinyins.
 * @returns: whether the get operation is successful.
 *
 * Get the number of the pinyins.
 *
 */
bool zhuyin_token_get_n_pronunciation(zhuyin_instance_t * instance,
                                      phrase_token_t token,
                                      guint * num);

/**
 * zhuyin_token_get_nth_pronunciation:
 * @instance: the zhuyin instance.
 * @token: the phrase token.
 * @nth: the index of the pinyin.
 * @keys: the GArray of chewing key.
 * @returns: whether the get operation is successful.
 *
 * Get the nth pinyin from the phrase.
 *
 */
bool zhuyin_token_get_nth_pronunciation(zhuyin_instance_t * instance,
                                        phrase_token_t token,
                                        guint nth,
                                        ChewingKeyVector keys);

/**
 * zhuyin_token_get_unigram_frequency:
 * @instance: the zhuyin instance.
 * @token: the phrase token.
 * @freq: the unigram frequency of the phrase.
 * @returns: whether the get operation is successful.
 *
 * Get the unigram frequency of the phrase.
 *
 */
bool zhuyin_token_get_unigram_frequency(zhuyin_instance_t * instance,
                                        phrase_token_t token,
                                        guint * freq);

/**
 * zhuyin_token_add_unigram_frequency:
 * @instance: the zhuyin instance.
 * @token: the phrase token.
 * @delta: the delta of the unigram frequency.
 * @returns: whether the add operation is successful.
 *
 * Add delta to the unigram frequency of the phrase token.
 *
 */
bool zhuyin_token_add_unigram_frequency(zhuyin_instance_t * instance,
                                        phrase_token_t token,
                                        guint delta);

/**
 * zhuyin_get_n_candidate:
 * @instance: the zhuyin instance.
 * @num: the number of the candidates.
 * @returns: whether the get operation is successful.
 *
 * Get the number of the candidates.
 *
 */
bool zhuyin_get_n_candidate(zhuyin_instance_t * instance,
                            guint * num);

/**
 * zhuyin_get_candidate:
 * @instance: the zhuyin instance.
 * @index: the index of the candidate.
 * @candidate: the retrieved candidate.
 *
 * Get the candidate of the index from the candidates.
 *
 */
bool zhuyin_get_candidate(zhuyin_instance_t * instance,
                          guint index,
                          lookup_candidate_t ** candidate);

/**
 * zhuyin_get_candidate_type:
 * @instance: the zhuyin instance.
 * @candidate: the lookup candidate.
 * @type: the type of the candidate.
 * @returns: whether the get operation is successful.
 *
 * Get the type of the lookup candidate.
 *
 */
bool zhuyin_get_candidate_type(zhuyin_instance_t * instance,
                               lookup_candidate_t * candidate,
                               lookup_candidate_type_t * type);

/**
 * zhuyin_get_candidate_string:
 * @instance: the zhuyin instance.
 * @candidate: the lookup candidate.
 * @utf8_str: the string of the candidate.
 * @returns: whether the get operation is successful.
 *
 * Get the string of the candidate.
 *
 */
bool zhuyin_get_candidate_string(zhuyin_instance_t * instance,
                                 lookup_candidate_t * candidate,
                                 const gchar ** utf8_str);

#if 0
/**
 * zhuyin_get_n_zhuyin:
 * @instance: the zhuyin instance.
 * @num: the number of the pinyins.
 * @returns: whether the get operation is successful.
 *
 * Get the number of the pinyins.
 *
 */
bool zhuyin_get_n_zhuyin(zhuyin_instance_t * instance,
                         guint * num);
#endif

/**
 * zhuyin_get_zhuyin_key:
 * @instance: the zhuyin instance.
 * @index: the index of the pinyin key.
 * @key: the retrieved pinyin key.
 * @returns: whether the get operation is successful.
 *
 * Get the pinyin key of the index from the pinyin keys.
 *
 */
bool zhuyin_get_zhuyin_key(zhuyin_instance_t * instance,
                           size_t index,
                           ChewingKey ** key);

/**
 * zhuyin_get_zhuyin_key_rest:
 * @instance: the pinyin index.
 * @index: the index of the pinyin key rest.
 * @key_rest: the retrieved pinyin key rest.
 * @returns: whether the get operation is successful.
 *
 * Get the pinyin key rest of the index from the pinyin key rests.
 *
 */
bool zhuyin_get_zhuyin_key_rest(zhuyin_instance_t * instance,
                                size_t index,
                                ChewingKeyRest ** key_rest);

/**
 * zhuyin_get_zhuyin_key_rest_positions:
 * @instance: the zhuyin instance.
 * @key_rest: the pinyin key rest.
 * @begin: the begin position of the corresponding pinyin key.
 * @end: the end position of the corresponding pinyin key.
 * @returns: whether the get operation is successful.
 *
 * Get the positions of the pinyin key rest.
 *
 */
bool zhuyin_get_zhuyin_key_rest_positions(zhuyin_instance_t * instance,
                                          ChewingKeyRest * key_rest,
                                          guint16 * begin, guint16 * end);

/**
 * zhuyin_get_zhuyin_key_rest_length:
 * @instance: the zhuyin instance.
 * @key_rest: the pinyin key rest.
 * @length: the length of the corresponding pinyin key.
 * @returns: whether the get operation is successful.
 *
 * Get the length of the corresponding zhuyin key.
 *
 */
bool zhuyin_get_zhuyin_key_rest_length(zhuyin_instance_t * instance,
                                       ChewingKeyRest * key_rest,
                                       guint16 * length);

/**
 * zhuyin_get_zhuyin_offset:
 * @instance: the zhuyin instance.
 * @cursor: the user cursor.
 * @offset: the lookup offset.
 * @returns: whether the get operation is successful.
 *
 * Get the lookup offset from the user cursor.
 *
 */
bool zhuyin_get_zhuyin_offset(zhuyin_instance_t * instance,
                              size_t cursor,
                              size_t * offset);

/**
 * zhuyin_get_left_zhuyin_offset:
 * @instance: the zhuyin instance.
 * @offset: the lookup offset.
 * @left: the left offset.
 * @returns: whether the get operation is successful.
 *
 * Get the left offset from the lookup offset.
 *
 */
bool zhuyin_get_left_zhuyin_offset(zhuyin_instance_t * instance,
                                   size_t offset,
                                   size_t * left);

/**
 * zhuyin_get_right_zhuyin_offset:
 * @instance: the zhuyin instance.
 * @offset: the lookup offset.
 * @right: the right offset.
 * @returns: whether the get operation is successful.
 *
 * Get the right offset from the lookup offset.
 *
 */
bool zhuyin_get_right_zhuyin_offset(zhuyin_instance_t * instance,
                                    size_t offset,
                                    size_t * right);

/**
 * zhuyin_get_character_offset:
 * @instance: the zhuyin instance.
 * @phrase: the utf8 phrase.
 * @offset: the lookup offset.
 * @length: the character offset.
 * @returns: whether the get operation is successful.
 *
 * Get the character offset from the lookup offset.
 *
 */
bool zhuyin_get_character_offset(zhuyin_instance_t * instance,
                                 const char * phrase,
                                 size_t offset,
                                 size_t * length);

#if 0
/**
 * zhuyin_get_raw_user_input:
 * @instance: the zhuyin instance.
 * @utf8_str: the modified raw full pinyin after choose candidate.
 * @returns: whether the get operation is successful.
 *
 * Get the modified raw full pinyin after choose candidate.
 *
 */
bool zhuyin_get_raw_user_input(zhuyin_instance_t * instance,
                               const gchar ** utf8_str);
#endif

/**
 * zhuyin_get_n_phrase:
 * @instance: the zhuyin instance.
 * @num: the number of the phrase tokens.
 * @returns: whether the get operation is successful.
 *
 * Get the number of the phrase tokens.
 *
 */
bool zhuyin_get_n_phrase(zhuyin_instance_t * instance,
                         guint * num);

/**
 * zhuyin_get_phrase_token:
 * @instance: the zhuyin instance.
 * @index: the index of the phrase token.
 * @token: the retrieved phrase token.
 * @returns: whether the get operation is successful.
 *
 * Get the phrase token of the index from the phrase tokens.
 *
 */
bool zhuyin_get_phrase_token(zhuyin_instance_t * instance,
                             guint index,
                             phrase_token_t * token);

/* hack here. */
typedef ChewingKey PinyinKey;
typedef ChewingKeyRest PinyinKeyPos;
typedef pinyin_option_t zhuyin_option_t;


G_END_DECLS

#endif
