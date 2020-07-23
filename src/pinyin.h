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


#ifndef PINYIN_H
#define PINYIN_H


#include "novel_types.h"
#include "pinyin_custom2.h"


G_BEGIN_DECLS

typedef struct _ChewingKey ChewingKey;
typedef struct _ChewingKeyRest ChewingKeyRest;

typedef struct _pinyin_context_t pinyin_context_t;
typedef struct _pinyin_instance_t pinyin_instance_t;
typedef struct _lookup_candidate_t lookup_candidate_t;

typedef struct _import_iterator_t import_iterator_t;
typedef struct _export_iterator_t export_iterator_t;

typedef enum _lookup_candidate_type_t{
    NBEST_MATCH_CANDIDATE = 1,
    NORMAL_CANDIDATE,
    ZOMBIE_CANDIDATE,
    PREDICTED_CANDIDATE,
    ADDON_CANDIDATE,
} lookup_candidate_type_t;

typedef enum _sort_option_t{
    SORT_BY_PHRASE_LENGTH_AND_FREQUENCY = 1,
    SORT_BY_PHRASE_LENGTH_AND_PINYIN_LENGTH_AND_FREQUENCY,
} sort_option_t;

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
 * pinyin_load_phrase_library:
 * @context: the pinyin context.
 * @index: the phrase index to be loaded.
 * @returns: whether the load succeeded.
 *
 * Load the sub phrase library of the index.
 *
 */
bool pinyin_load_phrase_library(pinyin_context_t * context,
                                guint8 index);

/**
 * pinyin_unload_phrase_library:
 * @context: the pinyin context.
 * @index: the phrase index to be unloaded.
 * @returns: whether the unload succeeded.
 *
 * Unload the sub phrase library of the index.
 *
 */
bool pinyin_unload_phrase_library(pinyin_context_t * context,
                                  guint8 index);

/**
 * pinyin_load_addon_phrase_library:
 * @context: the pinyin context.
 * @index: the addon phrase index to be loaded.
 * @returns: whether the load succeeded.
 *
 * Load the addon sub phrase library of the index.
 *
 */
bool pinyin_load_addon_phrase_library(pinyin_context_t * context,
                                      guint8 index);

/**
 * pinyin_unload_addon_phrase_library:
 * @context: the pinyin context.
 * @index: the addon phrase index to be unloaded.
 * @returns: whether the unload succeeded.
 *
 * Unload the addon sub phrase library of the index.
 *
 */
bool pinyin_unload_addon_phrase_library(pinyin_context_t * context,
                                        guint8 index);

/**
 * pinyin_begin_add_phrases:
 * @context: the pinyin context.
 * @index: the phrase index to be imported.
 * @returns: the import iterator.
 *
 * Begin to add phrases.
 *
 */
import_iterator_t * pinyin_begin_add_phrases(pinyin_context_t * context,
                                             guint8 index);

/**
 * pinyin_iterator_add_phrase:
 * @iter: the import iterator.
 * @phrase: the phrase string.
 * @pinyin: the pinyin string.
 * @count: the count of the phrase/pinyin pair, -1 to use the default value.
 * @returns: whether the add operation succeeded.
 *
 * Add a pair of phrase and pinyin with count.
 *
 */
bool pinyin_iterator_add_phrase(import_iterator_t * iter,
                                const char * phrase,
                                const char * pinyin,
                                gint count);

/**
 * pinyin_end_add_phrases:
 * @iter: the import iterator.
 *
 * End adding phrases.
 *
 */
void pinyin_end_add_phrases(import_iterator_t * iter);

/**
 * pinyin_begin_get_phrases:
 * @context: the pinyin context.
 * @index: the phrase index to be exported.
 * @returns: the export iterator.
 *
 * Begin to get phrases.
 *
 */
export_iterator_t * pinyin_begin_get_phrases(pinyin_context_t * context,
                                             guint index);

/**
 * pinyin_iterator_has_next_phrase:
 * @iter: the export iterator.
 * @returns: whether the iterator has the next phrase.
 *
 * Check whether the iterator has the next phrase.
 *
 */
bool pinyin_iterator_has_next_phrase(export_iterator_t * iter);

/**
 * pinyin_iterator_get_next_phrase:
 * @iter: the export iterator.
 * @phrase: the phrase string.
 * @pinyin: the pinyin string.
 * @count: the count of the phrase/pinyin pair, -1 means the default value.
 * @returns: whether the get next phrase operation succeeded.
 *
 * Get a pair of phrase and pinyin with count.
 *
 */
bool pinyin_iterator_get_next_phrase(export_iterator_t * iter,
                                     gchar ** phrase,
                                     gchar ** pinyin,
                                     gint * count);

/**
 * pinyin_end_get_phrases:
 * @iter: the export iterator.
 *
 * End getting phrases.
 *
 */
void pinyin_end_get_phrases(export_iterator_t * iter);

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
 * pinyin_set_full_pinyin_scheme:
 * @context: the pinyin context.
 * @scheme: the full pinyin scheme.
 * @returns: whether the set full pinyin scheme succeeded.
 *
 * Change the full pinyin scheme of the pinyin context.
 *
 */
bool pinyin_set_full_pinyin_scheme(pinyin_context_t * context,
                                   FullPinyinScheme scheme);

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
 * pinyin_set_zhuyin_scheme:
 * @context: the pinyin context.
 * @scheme: the zhuyin scheme.
 * @returns: whether the set zhuyin scheme succeeded.
 *
 * Change the zhuyin scheme of the pinyin context.
 *
 */
bool pinyin_set_zhuyin_scheme(pinyin_context_t * context,
                              ZhuyinScheme scheme);

/**
 * pinyin_fini:
 * @context: the pinyin context.
 *
 * Finalize the pinyin context.
 *
 */
void pinyin_fini(pinyin_context_t * context);


/**
 * pinyin_mask_out:
 * @context: the pinyin context.
 * @mask: the mask.
 * @value: the value.
 * @returns: whether the mask out operation is successful.
 *
 * Mask out the matched phrase tokens.
 *
 */
bool pinyin_mask_out(pinyin_context_t * context,
                     phrase_token_t mask,
                     phrase_token_t value);


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
 * pinyin_get_context:
 * @instance: the pinyin instance.
 * @returns: the pinyin context.
 *
 * Get the pinyin context from the pinyin instance.
 *
 */
pinyin_context_t * pinyin_get_context (pinyin_instance_t * instance);

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
 * pinyin_guess_predicted_candidates:
 * @instance: the pinyin instance.
 * @prefix: the prefix before the predicted candidates.
 * @returns: whether the predicted candidates are guessed successfully.
 *
 * Guess the predicted candidates after the prefix word.
 *
 */
bool pinyin_guess_predicted_candidates(pinyin_instance_t * instance,
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
 * @index: the index of the nbest result.
 * @sentence: the saved sentence in the instance.
 * @returns: whether the sentence is already saved in the instance.
 *
 * Get the sentence from the instance.
 *
 * Note: the returned sentence should be freed by g_free().
 *
 */
bool pinyin_get_sentence(pinyin_instance_t * instance,
                         guint8 index,
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
 * pinyin_get_parsed_input_length:
 * @instance: the pinyin instance.
 * @returns: the parsed_length of the input.
 *
 * Get the parsed length of the input.
 *
 */
size_t pinyin_get_parsed_input_length(pinyin_instance_t * instance);


/**
 * pinyin_in_chewing_keyboard:
 * @instance: the pinyin instance.
 * @key: the input key.
 * @symbol: the zhuyin symbols must be freed by g_strfreev.
 * @returns: whether the key is in current chewing scheme.
 *
 * Check whether the input key is in current chewing scheme.
 *
 */
bool pinyin_in_chewing_keyboard(pinyin_instance_t * instance,
                                const char key, gchar *** symbols);

/**
 * pinyin_guess_candidates:
 * @instance: the pinyin instance.
 * @offset: the lookup offset.
 * @sort_option: the sort option.
 * @returns: whether a list of tokens are gotten.
 *
 * Guess the candidates at the offset.
 *
 */
bool pinyin_guess_candidates(pinyin_instance_t * instance,
                             size_t offset,
                             sort_option_t sort_option);

/**
 * pinyin_choose_candidate:
 * @instance: the pinyin instance.
 * @offset: the offset in the pinyin keys.
 * @candidate: the selected candidate.
 * @returns: the cursor after the chosen candidate.
 *
 * Choose a full pinyin candidate at the offset.
 *
 */
int pinyin_choose_candidate(pinyin_instance_t * instance,
                            size_t offset,
                            lookup_candidate_t * candidate);

/**
 * pinyin_choose_predicted_candidate:
 * @instance: the pinyin instance.
 * @candidate: the selected candidate.
 * @returns: whether the self-learning is successful.
 *
 * Choose a predicted candidate.
 *
 */
bool pinyin_choose_predicted_candidate(pinyin_instance_t * instance,
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
 * pinyin_lookup_tokens:
 * @instance: the pinyin instance.
 * @phrase: the phrase to be looked up.
 * @tokenarray: the returned GArray of tokens.
 * @returns: whether the lookup operation is successful.
 *
 * Lookup the tokens for the phrase utf8 string.
 *
 */
bool pinyin_lookup_tokens(pinyin_instance_t * instance,
                          const char * phrase, GArray * tokenarray);

/**
 * pinyin_train:
 * @instance: the pinyin instance.
 * @index: the index of the nbest result.
 * @returns: whether the sentence is trained.
 *
 * Train the current user input sentence.
 *
 */
bool pinyin_train(pinyin_instance_t * instance, guint8 index);

/**
 * pinyin_reset:
 * @instance: the pinyin instance.
 * @returns: whether the pinyin instance is resetted.
 *
 * Reset the pinyin instance.
 *
 */
bool pinyin_reset(pinyin_instance_t * instance);

/**
 * pinyin_get_zhuyin_string:
 * @instance: the pinyin instance.
 * @key: the chewing key.
 * @utf8_str: the zhuyin string.
 * @returns: whether the get operation is successful.
 *
 * Get the zhuyin string of the key.
 *
 */
bool pinyin_get_zhuyin_string(pinyin_instance_t * instance,
                              ChewingKey * key,
                              gchar ** utf8_str);

/**
 * pinyin_get_pinyin_string:
 * @instance: the pinyin instance.
 * @key: the pinyin key.
 * @utf8_str: the pinyin string.
 * @returns: whether the get operation is successful.
 *
 * Get the pinyin string of the key.
 *
 */
bool pinyin_get_pinyin_string(pinyin_instance_t * instance,
                              ChewingKey * key,
                              gchar ** utf8_str);

/**
 * pinyin_get_luoma_pinyin_string:
 * @instance: the pinyin instance.
 * @key: the pinyin key.
 * @utf8_str: the luoma pinyin string.
 * @returns: whether the get operation is successful.
 *
 * Get the luoma pinyin string of the key.
 *
 */
bool pinyin_get_luoma_pinyin_string(pinyin_instance_t * instance,
                                    ChewingKey * key,
                                    gchar ** utf8_str);

/**
 * pinyin_get_secondary_zhuyin_string:
 * @instance: the pinyin instance.
 * @key: the pinyin key.
 * @utf8_str: the secondary zhuyin string.
 * @returns: whether the get operation is successful.
 *
 * Get the secondary zhuyin string of the key.
 *
 */
bool pinyin_get_secondary_zhuyin_string(pinyin_instance_t * instance,
                                        ChewingKey * key,
                                        gchar ** utf8_str);

/**
 * pinyin_get_pinyin_strings:
 * @instance: the pinyin instance.
 * @key: the pinyin key.
 * @shengmu: the shengmu string.
 * @yunmu: the yunmu string.
 * @returns: whether the get operation is successful.
 *
 * Get the shengmu and yunmu strings of the key.
 *
 */
bool pinyin_get_pinyin_strings(pinyin_instance_t * instance,
                               ChewingKey * key,
                               gchar ** shengmu,
                               gchar ** yunmu);

/**
 * pinyin_get_pinyin_is_incomplete:
 * @instance: the pinyin instance.
 * @key: the pinyin key.
 * @returns: whether the pinyin key is incomplete pinyin.
 *
 * Check whether the pinyin key is incomplete pinyin.
 *
 */
bool pinyin_get_pinyin_is_incomplete(pinyin_instance_t * instance,
                                     ChewingKey * key);

/**
 * pinyin_token_get_phrase:
 * @instance: the pinyin instance.
 * @token: the phrase token.
 * @len: the phrase length.
 * @utf8_str: the phrase string.
 * @returns: whether the get operation is successful.
 *
 * Get the phrase length and utf8 string.
 *
 */
bool pinyin_token_get_phrase(pinyin_instance_t * instance,
                             phrase_token_t token,
                             guint * len,
                             gchar ** utf8_str);

/**
 * pinyin_token_get_n_pronunciation:
 * @instance: the pinyin instance.
 * @token: the phrase token.
 * @num: the number of pinyins.
 * @returns: whether the get operation is successful.
 *
 * Get the number of the pinyins.
 *
 */
bool pinyin_token_get_n_pronunciation(pinyin_instance_t * instance,
                                      phrase_token_t token,
                                      guint * num);

/**
 * pinyin_token_get_nth_pronunciation:
 * @instance: the pinyin instance.
 * @token: the phrase token.
 * @nth: the index of the pinyin.
 * @keys: the GArray of chewing key.
 * @returns: whether the get operation is successful.
 *
 * Get the nth pinyin from the phrase.
 *
 */
bool pinyin_token_get_nth_pronunciation(pinyin_instance_t * instance,
                                        phrase_token_t token,
                                        guint nth,
                                        ChewingKeyVector keys);

/**
 * pinyin_token_get_unigram_frequency:
 * @instance: the pinyin instance.
 * @token: the phrase token.
 * @freq: the unigram frequency of the phrase.
 * @returns: whether the get operation is successful.
 *
 * Get the unigram frequency of the phrase.
 *
 */
bool pinyin_token_get_unigram_frequency(pinyin_instance_t * instance,
                                        phrase_token_t token,
                                        guint * freq);

/**
 * pinyin_token_add_unigram_frequency:
 * @instance: the pinyin instance.
 * @token: the phrase token.
 * @delta: the delta of the unigram frequency.
 * @returns: whether the add operation is successful.
 *
 * Add delta to the unigram frequency of the phrase token.
 *
 */
bool pinyin_token_add_unigram_frequency(pinyin_instance_t * instance,
                                        phrase_token_t token,
                                        guint delta);

/**
 * pinyin_get_n_candidate:
 * @instance: the pinyin instance.
 * @num: the number of the candidates.
 * @returns: whether the get operation is successful.
 *
 * Get the number of the candidates.
 *
 */
bool pinyin_get_n_candidate(pinyin_instance_t * instance,
                            guint * num);

/**
 * pinyin_get_candidate:
 * @instance: the pinyin instance.
 * @index: the index of the candidate.
 * @candidate: the retrieved candidate.
 *
 * Get the candidate of the index from the candidates.
 *
 */
bool pinyin_get_candidate(pinyin_instance_t * instance,
                          guint index,
                          lookup_candidate_t ** candidate);

/**
 * pinyin_get_candidate_type:
 * @instance: the pinyin instance.
 * @candidate: the lookup candidate.
 * @type: the type of the candidate.
 * @returns: whether the get operation is successful.
 *
 * Get the type of the lookup candidate.
 *
 */
bool pinyin_get_candidate_type(pinyin_instance_t * instance,
                               lookup_candidate_t * candidate,
                               lookup_candidate_type_t * type);

/**
 * pinyin_get_candidate_string:
 * @instance: the pinyin instance.
 * @candidate: the lookup candidate.
 * @utf8_str: the string of the candidate.
 * @returns: whether the get operation is successful.
 *
 * Get the string of the candidate.
 *
 */
bool pinyin_get_candidate_string(pinyin_instance_t * instance,
                                 lookup_candidate_t * candidate,
                                 const gchar ** utf8_str);

/**
 * pinyin_get_candidate_nbest_index:
 * @instance: the pinyin instance.
 * @candidate: the lookup candidate.
 * @index: the index of the nbest result.
 * @returns: whether the get operation is successful.
 *
 * Get the nbest index of the candidate.
 *
 */
bool pinyin_get_candidate_nbest_index(pinyin_instance_t * instance,
                                      lookup_candidate_t * candidate,
                                      guint8 * index);

/**
 * pinyin_get_pinyin_key:
 * @instance: the pinyin instance.
 * @offset: the offset of the pinyin key.
 * @key: the retrieved pinyin key.
 * @returns: whether the get operation is successful.
 *
 * Get the pinyin key of the index from the pinyin keys.
 *
 */
bool pinyin_get_pinyin_key(pinyin_instance_t * instance,
                           size_t offset,
                           ChewingKey ** key);

/**
 * pinyin_get_pinyin_key_rest:
 * @instance: the pinyin index.
 * @offset: the offset of the pinyin key rest.
 * @key_rest: the retrieved pinyin key rest.
 * @returns: whether the get operation is successful.
 *
 * Get the pinyin key rest of the index from the pinyin key rests.
 *
 */
bool pinyin_get_pinyin_key_rest(pinyin_instance_t * instance,
                                size_t offset,
                                ChewingKeyRest ** key_rest);

/**
 * pinyin_get_pinyin_key_rest_positions:
 * @instance: the pinyin instance.
 * @key_rest: the pinyin key rest.
 * @begin: the begin position of the corresponding pinyin key.
 * @end: the end position of the corresponding pinyin key.
 * @returns: whether the get operation is successful.
 *
 * Get the positions of the pinyin key rest.
 *
 */
bool pinyin_get_pinyin_key_rest_positions(pinyin_instance_t * instance,
                                          ChewingKeyRest * key_rest,
                                          guint16 * begin, guint16 * end);

/**
 * pinyin_get_pinyin_key_rest_length:
 * @instance: the pinyin instance.
 * @key_rest: the pinyin key rest.
 * @length: the length of the corresponding pinyin key.
 * @returns: whether the get operation is successful.
 *
 * Get the length of the corresponding pinyin key.
 *
 */
bool pinyin_get_pinyin_key_rest_length(pinyin_instance_t * instance,
                                       ChewingKeyRest * key_rest,
                                       guint16 * length);

/**
 * pinyin_get_pinyin_offset:
 * @instance: the pinyin instance.
 * @cursor: the user cursor.
 * @offset: the lookup offset.
 * @returns: whether the get operation is successful.
 *
 * Get the lookup offset from the user cursor.
 *
 */
bool pinyin_get_pinyin_offset(pinyin_instance_t * instance,
                              size_t cursor,
                              size_t * offset);

/**
 * pinyin_get_left_pinyin_offset:
 * @instance: the pinyin instance.
 * @offset: the lookup offset.
 * @left: the left offset.
 * @returns: whether the get operation is successful.
 *
 * Get the left offset from the lookup offset.
 *
 */
bool pinyin_get_left_pinyin_offset(pinyin_instance_t * instance,
                                   size_t offset,
                                   size_t * left);

/**
 * pinyin_get_right_pinyin_offset:
 * @instance: the pinyin instance.
 * @offset: the lookup offset.
 * @right: the right offset.
 * @returns: whether the get operation is successful.
 *
 * Get the right offset from the lookup offset.
 *
 */
bool pinyin_get_right_pinyin_offset(pinyin_instance_t * instance,
                                    size_t offset,
                                    size_t * right);

/**
 * pinyin_get_character_offset:
 * @instance: the pinyin instance.
 * @phrase: the utf8 phrase.
 * @offset: the lookup offset.
 * @length: the character offset.
 * @returns: whether the get operation is successful.
 *
 * Get the character offset from the lookup offset.
 *
 */
bool pinyin_get_character_offset(pinyin_instance_t * instance,
                                 const char * phrase,
                                 size_t offset,
                                 size_t * length);

#if 0
/**
 * pinyin_get_raw_full_pinyin:
 * @instance: the pinyin instance.
 * @utf8_str: the modified raw full pinyin after choose candidate.
 * @returns: whether the get operation is successful.
 *
 * Get the modified raw full pinyin after choose candidate.
 *
 */
bool pinyin_get_raw_full_pinyin(pinyin_instance_t * instance,
                                const gchar ** utf8_str);
#endif

/**
 * pinyin_get_n_phrase:
 * @instance: the pinyin instance.
 * @num: the number of the phrase tokens.
 * @returns: whether the get operation is successful.
 *
 * Get the number of the phrase tokens.
 *
 */
bool pinyin_get_n_phrase(pinyin_instance_t * instance,
                         guint * num);

/**
 * pinyin_get_phrase_token:
 * @instance: the pinyin instance.
 * @index: the index of the phrase token.
 * @token: the retrieved phrase token.
 * @returns: whether the get operation is successful.
 *
 * Get the phrase token of the index from the phrase tokens.
 *
 */
bool pinyin_get_phrase_token(pinyin_instance_t * instance,
                             guint index,
                             phrase_token_t * token);

/**
 * pinyin_get_full_pinyin_auxiliary_text:
 * @instance: the pinyin instance.
 * @cursor: the current cursor.
 * @aux_text: the auxiliary text.
 * @returns: whether the get operation is successful.
 *
 * Get the auxiliary text for full pinyin.
 *
 */
bool pinyin_get_full_pinyin_auxiliary_text(pinyin_instance_t * instance,
                                           size_t cursor,
                                           gchar ** aux_text);

/**
 * pinyin_get_double_pinyin_auxiliary_text:
 * @instance: the pinyin instance.
 * @cursor: the current cursor.
 * @aux_text: the auxiliary text.
 * @returns: whether the get operation is successful.
 *
 * Get the auxiliary text for double pinyin.
 *
 */
bool pinyin_get_double_pinyin_auxiliary_text(pinyin_instance_t * instance,
                                             size_t cursor,
                                             gchar ** aux_text);

/**
 * pinyin_get_chewing_auxiliary_text:
 * @instance: the pinyin instance.
 * @cursor: the current cursor.
 * @aux_text: the auxiliary text.
 * @returns: whether the get operation is successful.
 *
 * Get the auxiliary text for chewing.
 *
 */
bool pinyin_get_chewing_auxiliary_text(pinyin_instance_t * instance,
                                       size_t cursor,
                                       gchar ** aux_text);

/**
 * pinyin_remember_user_input:
 * @instance: the pinyin instance.
 * @phrase: the utf8 phrase.
 * @count: the count of the phrase, -1 to use the default value.
 * @returns: whether the phrase is remembered.
 *
 * Remember the current phrase and pinyin pair.
 *
 */
bool pinyin_remember_user_input(pinyin_instance_t * instance,
                                const char * phrase,
                                gint count);

/**
 * pinyin_is_user_candidate:
 * @instance: the pinyin instance.
 * @candidate: the lookup candidate.
 * @returns: whether the candidate is user candidate.
 *
 * Check whether the candidate is user candidate.
 *
 */
bool pinyin_is_user_candidate(pinyin_instance_t * instance,
                              lookup_candidate_t * candidate);

/**
 * pinyin_remove_user_candidate:
 * @instance: the pinyin instance.
 * @candidate: the lookup candidate.
 * @returns: whether the remove operation is successful.
 *
 * Remove the user candidate from dictionary.
 *
 */
bool pinyin_remove_user_candidate(pinyin_instance_t * instance,
                                  lookup_candidate_t * candidate);



/* for compatibility. */
typedef ChewingKey PinyinKey;
typedef ChewingKeyRest PinyinKeyPos;


G_END_DECLS

#endif
