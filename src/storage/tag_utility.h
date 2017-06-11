/* 
 *  libpinyin
 *  Library to deal with pinyin.
 *  
 *  Copyright (C) 2010 Peng Wu
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

#ifndef TAG_UTILITY_H
#define TAG_UTILITY_H

#include "novel_types.h"

/* Note: the optional tag has been removed from the first implementation.
 * Maybe the optional tag will be added back later.
 */

namespace pinyin{

/**
 * taglib_init:
 * @returns: whether the initialize operation is successful.
 *
 * Initialize the n-gram tag parse library.
 *
 */
bool taglib_init();

/**
 * taglib_add_tag:
 * @line_type: the line type.
 * @line_tag: the line tag.
 * @num_of_values: the number of values following the line tag.
 * @required_tags: the required tags of the line.
 * @ignored_tags: the ignored tags of the line.
 * @returns: whether the add operation is successful.
 *
 * Add one line tag to the tag parse library.
 *
 * Note: the required and ignored tags are separated by ',' or ':' .
 *
 */
bool taglib_add_tag(int line_type, const char * line_tag, int num_of_values, const char * required_tags, const char * ignored_tags);

/**
 * taglib_read:
 * @input_line: one input line.
 * @line_type: the line type.
 * @values: the values following the line tag.
 * @required: the required tags of the line type.
 * @returns: whether the line is parsed ok.
 *
 * Parse one input line into line_type, values and required tags.
 *
 * Note: most parameters are hash table of string (const char *).
 *
 */
bool taglib_read(const char * input_line, int & line_type,
                 GPtrArray * values, GHashTable * required);

/**
 * taglib_remove_tag:
 * @line_type: the type of the line tag.
 * @returns: whether the remove operation is successful.
 *
 * Remove one line tag.
 *
 */
bool taglib_remove_tag(int line_type);

/**
 * taglib_push_state:
 * @returns: whether the push operation is successful.
 *
 * Push the current state onto the stack.
 *
 * Note: the taglib_push/pop_state functions are used to save
 * the current known tag list in stack.
 * Used when the parsing context is changed.
 */
bool taglib_push_state();

/**
 * taglib_pop_state:
 * @returns: whether the pop operation is successful.
 *
 * Pop the current state off the stack.
 *
 */
bool taglib_pop_state();

/**
 * taglib_fini:
 * @returns: whether the finish operation is successful.
 *
 * Finish the n-gram tag parse library.
 *
 */
bool taglib_fini();


class FacadePhraseIndex;


/**
 * taglib_token_to_string:
 * @phrase_index: the phrase index for phrase string lookup.
 * @token: the phrase token.
 * @returns: the phrase string found in phrase index.
 *
 * Translate one token into the phrase string.
 *
 */
char * taglib_token_to_string(FacadePhraseIndex * phrase_index,
                              phrase_token_t token);

/**
 * taglib_validate_token_with_string:
 * @phrase_index: the phrase index.
 * @token: the phrase token.
 * @string: the phrase string.
 * @returns: whether the token is validated with the phrase string.
 *
 * Validate the token with the phrase string.
 *
 */
bool taglib_validate_token_with_string(FacadePhraseIndex * phrase_index,
                                       phrase_token_t token,
                                       const char * string);

/* Note: the following function is only available when the optional tag exists.
   bool taglib_report_status(int line_type); */

/* Note: taglib_write is omitted, as printf is more suitable for this. */

};

#endif
