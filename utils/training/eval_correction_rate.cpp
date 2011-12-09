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


#include "pinyin_internal.h"


void print_help(){
    printf("Usage: eval_correction_rate\n");
}

bool get_possible_pinyin(FacadePhraseIndex * phrase_index,
                         TokenVector tokens, ChewingKeyVector keys){
    ChewingKey buffer[MAX_PHRASE_LENGTH];
    size_t key_index; guint32 max_freq;
    guint32 freq;
    g_array_set_size(keys, 0);

    for (size_t i = 0; i < tokens->len; ++i){
        phrase_token_t * token = &g_array_index(tokens, phrase_token_t, i);
        PhraseItem item;
        phrase_index->get_phrase_item(*token, item);
        key_index = 0; max_freq = 0;
        for ( size_t m = 0; m < item.get_n_pronunciation(); ++m ) {
            freq = 0;
            assert(item.get_nth_pronunciation(m, buffer, freq));
            if ( freq > max_freq ) {
                key_index = m;
                max_freq = freq;
            }
        }

        assert(item.get_nth_pronunciation(key_index, buffer, freq));
        assert(max_freq == freq);
        guint8 len = item.get_phrase_length();
        g_array_append_vals(keys, buffer, len);
    }
    return true;
}

bool get_best_match(PinyinLookup * pinyin_lookup,
                    ChewingKeyVector keys, TokenVector tokens){
    /* initialize constraints. */
    CandidateConstraints constraints = g_array_new
        (FALSE, FALSE, sizeof(lookup_constraint_t));
    g_array_set_size(constraints, keys->len);
    for ( size_t i = 0; i < constraints->len; ++i ) {
        lookup_constraint_t * constraint = &g_array_index
            (constraints, lookup_constraint_t, i);
        constraint->m_type = NO_CONSTRAINT;
    }

    return pinyin_lookup->get_best_match(keys, constraints, tokens);
}

bool do_one_test(PinyinLookup * pinyin_lookup,
                 FacadePhraseIndex * phrase_index,
                 TokenVector tokens){
    bool retval = false;

    ChewingKeyVector keys = g_array_new(FALSE, TRUE, sizeof(ChewingKey));
    TokenVector guessed_tokens = g_array_new
        (FALSE, TRUE, sizeof(phrase_token_t));

    get_possible_pinyin(phrase_index, tokens, keys);
    get_best_match(pinyin_lookup, keys, guessed_tokens);
    /* compare the results */
    char * sentence = NULL; char * guessed_sentence = NULL;
    pinyin_lookup->convert_to_utf8(tokens, sentence);
    pinyin_lookup->convert_to_utf8
        (guessed_tokens, guessed_sentence);

    if ( strcmp(sentence, guessed_sentence) != 0 ) {
        fprintf(stderr, "test sentence:%s\n", sentence);
        fprintf(stderr, "guessed sentence:%s\n", guessed_sentence);
        fprintf(stderr, "the result mis-matches.\n");
        retval = false;
    } else {
        retval = true;
    }

    g_free(sentence); g_free(guessed_sentence);
    g_array_free(keys, TRUE);
    g_array_free(guessed_tokens, TRUE);
    return retval;
}

int main(int argc, char * argv[]){
    const char * evals_text = "evals.text";

    pinyin_option_t options = USE_TONE;
    ChewingLargeTable largetable(options);

    MemoryChunk * new_chunk = new MemoryChunk;
    new_chunk->load("pinyin_index.bin");
    largetable.load(new_chunk);

    FacadePhraseIndex phrase_index;
    new_chunk = new MemoryChunk;
    new_chunk->load("gb_char.bin");
    phrase_index.load(1, new_chunk);
    new_chunk = new MemoryChunk;
    new_chunk->load("gbk_char.bin");
    phrase_index.load(2, new_chunk);

    PhraseLargeTable phrases;
    new_chunk = new MemoryChunk;
    new_chunk->load("phrase_index.bin");
    phrases.load(new_chunk);

    Bigram system_bigram;
    system_bigram.attach("bigram.db", ATTACH_READONLY);
    Bigram user_bigram;
    user_bigram.attach(NULL, ATTACH_CREATE|ATTACH_READWRITE);

    PinyinLookup pinyin_lookup(options, &largetable, &phrase_index,
                               &system_bigram, &user_bigram);

    /* open evals.text. */
    FILE * evals_file = fopen(evals_text, "r");
    if ( NULL == evals_file ) {
        fprintf(stderr, "Can't open file:%s\n", evals_text);
        exit(ENOENT);
    }

    /* Evaluates the correction rate of test text documents. */
    size_t tested_count = 0; size_t passed_count = 0;
    char* linebuf = NULL; size_t size = 0;
    TokenVector tokens = g_array_new(FALSE, TRUE, sizeof(phrase_token_t));

    phrase_token_t token;
    while( getline(&linebuf, &size, evals_file) ) {
        if ( feof(evals_file) )
            break;
        if ( '\n' == linebuf[strlen(linebuf)-1] )
            linebuf[strlen(linebuf)-1] = '\0';

        glong phrase_len = 0;
        utf16_t * phrase = g_utf8_to_utf16(linebuf, -1, NULL, &phrase_len, NULL);

        token = 0;
        if ( 0 != phrase_len ) {
            int result = phrases.search( phrase_len, phrase, token);
            if ( ! (result & SEARCH_OK) )
                token = 0;
            g_free(phrase);
            phrase = NULL;
        }

        if ( 0 == token ) {
            if ( tokens->len ) { /* one test. */
                if ( do_one_test(&pinyin_lookup, &phrase_index, tokens) ) {
                    tested_count ++; passed_count ++;
                } else {
                    tested_count ++;
                }
                g_array_set_size(tokens, 0);
            }
        } else {
            g_array_append_val(tokens, token);
        }
    }

    if ( tokens->len ) { /* one test. */
        if ( do_one_test(&pinyin_lookup, &phrase_index, tokens) ) {
            tested_count ++; passed_count ++;
        } else {
            tested_count ++;
        }
    }

    parameter_t rate = passed_count / (parameter_t) tested_count;
    printf("correction rate:%f\n", rate);

    g_array_free(tokens, TRUE);
    fclose(evals_file);
    free(linebuf);

    return 0;
}
