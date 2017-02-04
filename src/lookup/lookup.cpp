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


#include "lookup.h"
#include "phrase_index.h"

namespace pinyin{

bool convert_to_utf8(FacadePhraseIndex * phrase_index,
                     MatchResult result,
                     /* in */ const char * delimiter,
                     /* in */ bool show_tokens,
                     /* out */ char * & result_string){
    //init variables
    if ( NULL == delimiter )
        delimiter = "";
    result_string = NULL;

    PhraseItem item;

    for ( size_t i = 0; i < result->len; ++i ){
        phrase_token_t token = g_array_index
            (result, phrase_token_t, i);
        if ( null_token == token )
            continue;

        phrase_index->get_phrase_item(token, item);
        ucs4_t buffer[MAX_PHRASE_LENGTH];
        item.get_phrase_string(buffer);

        guint8 length = item.get_phrase_length();
        gchar * phrase = NULL;
        char * tmp = NULL;

        if (show_tokens) {
            tmp = g_ucs4_to_utf8(buffer, length, NULL, NULL, NULL);
            phrase = g_strdup_printf("%d %s", token, tmp);
            g_free(tmp);
        } else {
            phrase = g_ucs4_to_utf8(buffer, length, NULL, NULL, NULL);
        }

        tmp = result_string;
        if ( NULL == result_string )
            result_string = g_strdup(phrase);
        else
            result_string = g_strconcat(result_string, delimiter, phrase, NULL);
        g_free(phrase);
        g_free(tmp);
    }
    return true;
}

};
