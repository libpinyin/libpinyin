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

#include <stdio.h>
#include "pinyin_internal.h"
#include "utils_helper.h"
#include "k_mixture_model.h"

enum LINE_TYPE{
    BEGIN_LINE = 1,
    END_LINE,
    GRAM_1_LINE,
    GRAM_2_LINE,
    GRAM_1_ITEM_LINE,
    GRAM_2_ITEM_LINE
};

static int line_type = 0;
static GPtrArray * values = NULL;
static GHashTable * required = NULL;
/* variables for line buffer. */
static char * linebuf = NULL;
static size_t len = 0;

bool parse_headline(KMixtureModelBigram * bigram);

bool parse_unigram(FILE * input, PhraseLargeTable2 * phrase_table,
                   FacadePhraseIndex * phrase_index,
                   KMixtureModelBigram * bigram);

bool parse_bigram(FILE * input, PhraseLargeTable2 * phrase_table,
                  FacadePhraseIndex * phrase_index,
                  KMixtureModelBigram * bigram);

void print_help(){
    printf("Usage: import_k_mixture_model [--k-mixture-model-file <FILENAME>]\n");
}

static ssize_t my_getline(FILE * input){
    ssize_t result = getline(&linebuf, &len, input);
    if ( result == -1 )
        return result;

    if ( '\n' == linebuf[strlen(linebuf) - 1] ) {
        linebuf[strlen(linebuf) - 1] = '\0';
    }
    return result;
}

bool parse_headline(KMixtureModelBigram * bigram){
    /* enter "\data" line */
    assert(taglib_add_tag(BEGIN_LINE, "\\data", 0, "model:count:N:total_freq", ""));

    /* read "\data" line */
    if ( !taglib_read(linebuf, line_type, values, required) ) {
        fprintf(stderr, "error: k mixture model expected.\n");
        return false;
    }

    assert(line_type == BEGIN_LINE);
    /* check header */
    TAGLIB_GET_TAGVALUE(const char *, model, (const char *));
    if ( !( strcmp("k mixture model", model) == 0 ) ) {
        fprintf(stderr, "error: k mixture model expected.\n");
        return false;
    }

    TAGLIB_GET_TAGVALUE(glong, count, atol);
    TAGLIB_GET_TAGVALUE(glong, N, atol);
    TAGLIB_GET_TAGVALUE(glong, total_freq, atol);

    KMixtureModelMagicHeader magic_header;
    memset(&magic_header, 0, sizeof(KMixtureModelMagicHeader));
    magic_header.m_WC =count; magic_header.m_N = N;
    magic_header.m_total_freq = total_freq;
    bigram->set_magic_header(magic_header);

    return true;
}

bool parse_body(FILE * input, PhraseLargeTable2 * phrase_table,
                FacadePhraseIndex * phrase_index,
                KMixtureModelBigram * bigram){
    taglib_push_state();

    assert(taglib_add_tag(END_LINE, "\\end", 0, "", ""));
    assert(taglib_add_tag(GRAM_1_LINE, "\\1-gram", 0, "", ""));
    assert(taglib_add_tag(GRAM_2_LINE, "\\2-gram", 0, "", ""));

    do {
    retry:
        assert(taglib_read(linebuf, line_type, values, required));
        switch(line_type) {
        case END_LINE:
            goto end;
        case GRAM_1_LINE:
            my_getline(input);
            parse_unigram(input, phrase_table, phrase_index, bigram);
            goto retry;
        case GRAM_2_LINE:
            my_getline(input);
            parse_bigram(input, phrase_table, phrase_index, bigram);
            goto retry;
        default:
            assert(false);
        }
    } while (my_getline(input) != -1) ;

 end:
    taglib_pop_state();
    return true;
}

bool parse_unigram(FILE * input, PhraseLargeTable2 * phrase_table,
                   FacadePhraseIndex * phrase_index,
                   KMixtureModelBigram * bigram){
    taglib_push_state();

    assert(taglib_add_tag(GRAM_1_ITEM_LINE, "\\item", 1, "count:freq", ""));

    do {
        assert(taglib_read(linebuf, line_type, values, required));
        switch (line_type) {
        case GRAM_1_ITEM_LINE:{
            /* handle \item in \1-gram */
            TAGLIB_GET_VALUE(token, 0);

            TAGLIB_GET_TAGVALUE(glong, count, atol);
            TAGLIB_GET_TAGVALUE(glong, freq, atol);

            KMixtureModelArrayHeader array_header;
            memset(&array_header, 0, sizeof(KMixtureModelArrayHeader));
            array_header.m_WC = count; array_header.m_freq = freq;
            bigram->set_array_header(token, array_header);
            break;
        }
        case END_LINE:
        case GRAM_1_LINE:
        case GRAM_2_LINE:
            goto end;
        default:
            assert(false);
        }
    } while (my_getline(input) != -1);

 end:
    taglib_pop_state();
    return true;
}

bool parse_bigram(FILE * input, PhraseLargeTable2 * phrase_table,
                  FacadePhraseIndex * phrase_index,
                  KMixtureModelBigram * bigram){
    taglib_push_state();

    assert(taglib_add_tag(GRAM_2_ITEM_LINE, "\\item", 2,
                          "count:T:N_n_0:n_1:Mr", ""));

    phrase_token_t last_token = null_token;
    KMixtureModelSingleGram * last_single_gram = NULL;
    do {
        assert(taglib_read(linebuf, line_type, values, required));
        switch (line_type) {
        case GRAM_2_ITEM_LINE:{
            /* handle \item in \2-gram */
            /* two tokens */
            TAGLIB_GET_VALUE(token1, 0);
            TAGLIB_GET_VALUE(token2, 1);

            TAGLIB_GET_TAGVALUE(glong, count, atol);
            TAGLIB_GET_TAGVALUE(glong, T, atol);
            assert(count == T);
            TAGLIB_GET_TAGVALUE(glong, N_n_0, atol);
            TAGLIB_GET_TAGVALUE(glong, n_1, atol);
            TAGLIB_GET_TAGVALUE(glong, Mr, atol);

            KMixtureModelArrayItem array_item;
            memset(&array_item, 0, sizeof(KMixtureModelArrayItem));
            array_item.m_WC = count; array_item.m_N_n_0 = N_n_0;
            array_item.m_n_1 = n_1; array_item.m_Mr = Mr;

            if ( last_token != token1 ) {
                if ( last_token && last_single_gram ) {
                    bigram->store(last_token, last_single_gram);
                    delete last_single_gram;
                    /* safe guard */
                    last_token = null_token;
                    last_single_gram = NULL;
                }
                KMixtureModelSingleGram * single_gram = NULL;
                bigram->load(token1, single_gram);

                /* create the new single gram */
                if ( single_gram == NULL )
                    single_gram = new KMixtureModelSingleGram;
                last_token = token1;
                last_single_gram = single_gram;
            }
            assert(last_single_gram->insert_array_item(token2, array_item));
            break;
        }
        case END_LINE:
        case GRAM_1_LINE:
        case GRAM_2_LINE:
            goto end;
        default:
            assert(false);
        }
    } while (my_getline(input) != -1);

 end:
    if ( last_token && last_single_gram ) {
        bigram->store(last_token, last_single_gram);
        delete last_single_gram;
        /* safe guard */
        last_token = null_token;
        last_single_gram = NULL;
    }

    taglib_pop_state();
    return true;
}

int main(int argc, char * argv[]){
    int i = 1;
    const char * k_mixture_model_filename = NULL;
    FILE * input = stdin;

    while ( i < argc ){
        if ( strcmp ("--help", argv[i]) == 0 ){
            print_help();
            exit(0);
        } else if ( strcmp ("--k-mixture-model-file", argv[i]) == 0 ){
            if ( ++i > argc ){
                print_help();
                exit(EINVAL);
            }
            k_mixture_model_filename = argv[i];
        } else {
            print_help();
            exit(EINVAL);
        }
        ++i;
    }

    PhraseLargeTable2 phrase_table;
    MemoryChunk * chunk = new MemoryChunk;
    chunk->load("phrase_index.bin");
    phrase_table.load(chunk);

    FacadePhraseIndex phrase_index;
    if (!load_phrase_index(&phrase_index))
        exit(ENOENT);

    KMixtureModelBigram bigram(K_MIXTURE_MODEL_MAGIC_NUMBER);
    bigram.attach(k_mixture_model_filename, ATTACH_READWRITE|ATTACH_CREATE);

    PhraseTokens tokens;
    memset(tokens, 0, sizeof(PhraseTokens));
    phrase_index.prepare_tokens(tokens);

    taglib_init();

    /* prepare to read n-gram model */
    values = g_ptr_array_new();
    required = g_hash_table_new(g_str_hash, g_str_equal);

    ssize_t result = my_getline(input);
    if ( result == -1 ) {
        fprintf(stderr, "empty file input.\n");
        exit(ENODATA);
    }

    if (!parse_headline(&bigram))
        exit(ENODATA);

    result = my_getline(input);
    if ( result != -1 )
        parse_body(input, &phrase_table, &phrase_index, &bigram);

    taglib_fini();

    phrase_index.destroy_tokens(tokens);

    return 0;
}
