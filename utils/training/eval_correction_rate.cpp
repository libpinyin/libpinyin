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


#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "pinyin_internal.h"
#include "utils_helper.h"


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

bool get_best_match(FacadePhraseIndex * phrase_index,
                    PhoneticLookup<1, 1> * pinyin_lookup,
                    PhoneticKeyMatrix * matrix,
                    NBestMatchResults * results) {
    /* prepare the prefixes for get_nbest_match. */
    TokenVector prefixes = g_array_new
        (FALSE, FALSE, sizeof(phrase_token_t));
    g_array_append_val(prefixes, sentence_start);

    /* initialize constraints. */
    ForwardPhoneticConstraints constraints(phrase_index);
    constraints.validate_constraint(matrix);

    bool retval = pinyin_lookup->get_nbest_match(prefixes, matrix, &constraints, results);

    g_array_free(prefixes, TRUE);
    return retval;
}

bool do_one_test(PhoneticLookup<1, 1> * pinyin_lookup,
                 FacadePhraseIndex * phrase_index,
                 TokenVector tokens){
    bool retval = false;

    ChewingKeyVector keys = g_array_new(TRUE, TRUE, sizeof(ChewingKey));
    ChewingKeyRestVector key_rests = g_array_new
        (TRUE, TRUE, sizeof(ChewingKeyRest));
    TokenVector guessed_tokens = NULL;

    get_possible_pinyin(phrase_index, tokens, keys);

    /* create fake key_rests */
    g_array_set_size(key_rests, keys->len);
    for (size_t i = 0; i < key_rests->len; ++i) {
        ChewingKeyRest key_rest;
        key_rest.m_raw_begin = i; key_rest.m_raw_end = i + 1;
        g_array_index(key_rests, ChewingKeyRest, i) = key_rest;
    }

    PhoneticKeyMatrix matrix;
    NBestMatchResults results;
    fill_matrix(&matrix, keys, key_rests, keys->len);
    get_best_match(phrase_index, pinyin_lookup, &matrix, &results);

    assert(1 == results.size());
    assert(results.get_result(0, guessed_tokens));

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
    g_array_free(keys, TRUE); g_array_free(key_rests, TRUE);
    return retval;
}

int main(int argc, char * argv[]){
    const char * evals_text = "evals2.text";

    SystemTableInfo2 system_table_info;

    bool retval = system_table_info.load(SYSTEM_TABLE_INFO);
    if (!retval) {
        fprintf(stderr, "load table.conf failed.\n");
        exit(ENOENT);
    }

    FacadeChewingTable2 largetable;
    largetable.load(SYSTEM_PINYIN_INDEX, NULL);

    FacadePhraseIndex phrase_index;

    const pinyin_table_info_t * phrase_files =
        system_table_info.get_default_tables();

    if (!load_phrase_index(phrase_files, &phrase_index))
        exit(ENOENT);

    Bigram system_bigram;
    system_bigram.attach(SYSTEM_BIGRAM, ATTACH_READONLY);
    Bigram user_bigram;
    user_bigram.attach(NULL, ATTACH_CREATE|ATTACH_READWRITE);

    gfloat lambda = system_table_info.get_lambda();

    PhoneticLookup<1, 1> pinyin_lookup(lambda,
                                    &largetable, &phrase_index,
                                    &system_bigram, &user_bigram);

    /* open evals text. */
    FILE * evals_file = fopen(evals_text, "r");
    if ( NULL == evals_file ) {
        fprintf(stderr, "Can't open file:%s\n", evals_text);
        exit(ENOENT);
    }

    /* Evaluates the correction rate of test text documents. */
    size_t tested_count = 0; size_t passed_count = 0;
    char* linebuf = NULL; size_t size = 0;
    TokenVector tokens = g_array_new(FALSE, TRUE, sizeof(phrase_token_t));

    while( getline(&linebuf, &size, evals_file) ) {
        if ( feof(evals_file) )
            break;

        if ( '\n' == linebuf[strlen(linebuf) - 1] ) {
            linebuf[strlen(linebuf) - 1] = '\0';
        }

        TAGLIB_PARSE_SEGMENTED_LINE(&phrase_index, token, linebuf);

        if ( null_token == token ) {
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
