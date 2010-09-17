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

bool taglib_init();

/* Note: most string array (const char *) are null pointer terminated. */
bool taglib_add_tag(int line_type, const char * line_tag, int num_of_values, const char * required_tags[], const char * optional_tags[], const char * ignored_tags[]);

/* most parameters are Array of string (const char *).
 * required tags are listed in the beginning of the array.
 */
bool taglib_read(const char * input_line, int & line_type, GPtrArray * values, GPtrArray * tagnames, GPtrArray * tagvalues);

/* Note: taglib_write is omited, as printf is more suitable for this. */

bool taglib_report_status(int line_type);

/* clear up the tag of type line_type. */
bool taglib_fini(int line_type);

#endif
