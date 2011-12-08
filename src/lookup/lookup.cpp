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


#include "lookup.h"
#include "phrase_index.h"

namespace pinyin{

bool convert_to_utf8(FacadePhraseIndex * phrase_index,
                     MatchResults match_results,
                     /* in */ const char * delimiter,
                     /* out */ char * & result_string){
    //init variables
    if ( NULL == delimiter )
        delimiter = "";
    result_string = NULL;

    PhraseItem item;

    for ( size_t i = 0; i < match_results->len; ++i ){
        phrase_token_t * token = &g_array_index
            (match_results, phrase_token_t, i);
        if ( null_token == *token )
            continue;

        phrase_index->get_phrase_item(*token, item);
        utf16_t buffer[MAX_PHRASE_LENGTH];
        item.get_phrase_string(buffer);

        guint8 length = item.get_phrase_length();
        gchar * phrase = g_utf16_to_utf8(buffer, length, NULL, NULL, NULL);
        char * tmp = result_string;
        if ( NULL == result_string )
            result_string = g_strdup(phrase);
        else
            result_string = g_strconcat(result_string, delimiter, phrase, NULL);
        g_free(tmp); g_free(phrase);
    }
    return true;
}

};
