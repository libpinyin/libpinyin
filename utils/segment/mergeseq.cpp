/* 
 *  libpinyin
 *  Library to deal with pinyin.
 *  
 *  Copyright (C) 2013 Peng Wu <alexepico@gmail.com>
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

#include "pinyin_internal.h"


void print_help(){
    printf("Usage: spseg [--generate-extra-enter] [-o outputfile] [inputfile]\n");
}


/* data structure definition. */
typedef struct{
    phrase_token_t m_token;
    gint m_token_len;
} TokenInfo;


/* GArray of ucs4 characters. */
typedef GArray * UnicodeCharVector;
/* GArray of TokenInfo. */
typedef GArray * TokenInfoVector;

gint calculate_sequence_length(TokenInfoVector * tokens) {
    gint len = 0;

    size_t i = 0;
    for (i = 0; i < tokens->len; ++i) {
        TokenInfo * token_info = &g_array_index(tokens, TokenInfo, i);
        len += token_info->len;
    }

    return len;
}

/* if merge sequence found, merge and output it,
 *   if not, just output the first token;
 * pop the first token or sequence.
 */
bool merge_sequence(PhraseLargeTable2 * phrase_table,
                    FacadePhraseIndex * phrase_index,
                    UnicodeCharVector * unichars,
                    TokenInfoVector * tokens) {
    assert(tokens->len > 0);

    bool found = false;
    TokenInfo * token_info = NULL;
    gint token_len = 0;
    phrase_token_t token = null_token;

    const gunichar * ucs4_str = (const gunichar *)unichars->data;

    PhraseTokens tokens;
    memset(tokens, 0, sizeof(PhraseTokens));
    phrase_index->prepare_tokens(tokens);

    /* search the merge sequence. */
    size_t index = tokens->len;
    gint seq_len = calculate_sequence_length(tokens);
    while (seq_len > 0) {
        /* do phrase table search. */
        phrase_index->clear_tokens(tokens);
        int retval = phrase_table->search(seq_len, ucs4_str, tokens);

        if (retval & SEARCH_OK) {
            int num = get_first_token(tokens, token);
            found = true;
            break;
        }

        --index;
        token_info = &g_array_index(tokens, TokenInfo, index);
        seq_len -= token_info->m_token_len;
    }

    phrase_index->destroy_tokens(tokens);

    /* push the merged sequence back. */
    if (found) {
        /* pop up the origin sequence. */
        g_array_remove_range(tokens, 0, index);

        TokenInfo info;
        info.m_token = token;
        info.m_token_len = seq_len;
        g_array_prepend_val(tokens, info);
    }

    return found;
}

bool pop_first_token(UnicodeCharVector * unichars,
                     TokenInfoVector * tokens,
                     FILE * output) {
    const gunichar * ucs4_str = (const gunichar *)unichars->data;

    /* pop it. */
    token_info = &g_array_index(tokens, TokenInfo, 0);
    token = token_info->m_token;
    token_len = token_info->m_token_len;

    glong read = 0;
    gchar * utf8_str = g_ucs4_to_utf8(ucs4_str, token_len, &read, NULL, NULL);
    assert(read == token_len);
    fprintf(output, "%d %s\n", token, utf8_str);
    g_free(utf8_str);

    g_array_remove_range(unichars, 0, token_len);
    g_array_remove_index(tokens, 0);

    return true;
}

bool feed_line(PhraseLargeTable2 * phrase_table,
               FacadePhraseIndex * phrase_index,
               UnicodeCharVector * unichars,
               TokenInfoVector * tokens,
               const char * line,
               FILE * output) {

    TAGLIB_PARSE_SEGMENTED_LINE(phrase_index, token, line);

    if (null_token == token) {
        /* empty the queue. */
        while (0 != tokens->len) {
            merge_sequence(phrase_table, phrase_index, unichars, tokens);
            pop_first_token(unichars, tokens, output);
        }

        assert(0 == unichars->len);
        assert(0 == tokens->len);
        return false;
    }

    PhraseItem item;
    phrase_index->get_phrase_item(token, item);
    guint8 len = item.get_phrase_length();

    TokenInfo info;
    info.m_token = token;
    info.m_token_len = len;
    g_array_append_val(tokens, info);

    ucs4_t buffer[MAX_PHRASE_LENGTH];
    item.get_phrase_string(buffer);
    g_array_append_vals(unichars, buffer, len);

    /* probe merge sequence. */
    gint len = calculate_sequence_length(tokens);
    while (len >= MAX_PHRASE_LENGTH) {
        merge_sequence(phrase_table, phrase_index, unichars, tokens);
        pop_first_token(unichars, tokens, output);
        len = calculate_sequence_length(tokens);
    }

    return true;
}
