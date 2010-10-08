/* 
 *  libpinyin
 *  Library to deal with pinyin.
 *  
 *  Copyright (C) 2010 Peng Wu
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

#ifndef TAG_UTILITY_H
#define TAG_UTILITY_H

/* Note: the optional tag has been removed from the first implementation.
 * Maybe the optional tag will be added back later.
 */

bool taglib_init();

/* Note: most string array (const char *) are null pointer terminated. */
bool taglib_add_tag(int line_type, const char * line_tag, int num_of_values, const char * required_tags[], const char * ignored_tags[]);

/* most parameters are hash table of string (const char *). */
bool taglib_read(const char * input_line, int & line_type, GPtrArray * values, GHashTable * required);

/* Note: taglib_write is omited, as printf is more suitable for this. */

/* Note the following function is only available when the optional tag exists.
 * bool taglib_report_status(int line_type);
 */

/* remove the tag of type line_type. */
bool taglib_remove_tag(int line_type);

/* the following functions are used to save current known tag list in stack.
 * Used when the parsing context is changed.
 */
bool taglib_push_state();
bool taglib_pop_state();

bool taglib_fini();

/* Useful macros to ease taglib_add_tag call,
 * or else need to use C++0x-features.
 */

#define TAGLIB_BEGIN_ADD_TAG(line_type, line_tag, num_of_values)        \
    {                                                                   \
        int line_type_saved = line_type;                          \
        const char * line_tag_saved = line_tag;                         \
        int num_of_values_saved = num_of_values;                  \
        ;

#define TAGLIB_REQUIRED_TAGS const char * required_tags_saved[]
/* #define TAGLIB_OPTIONAL_TAGS const char * optional_tags_saved[] */
#define TAGLIB_IGNORED_TAGS const char * ignored_tags_saved[]

#define TAGLIB_END_ADD_TAG                                              \
    assert(taglib_add_tag(line_type_saved, line_tag_saved,              \
                          num_of_values_saved,                          \
                          required_tags_saved, ignored_tags_saved));    \
    };

#endif
