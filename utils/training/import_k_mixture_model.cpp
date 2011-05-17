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
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

#include <stdio.h>
#include "pinyin.h"
#include "tag_utility.h"
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

bool parse_unigram(FILE * input, PhraseLargeTable * phrases,
                   KMixtureModelBigram * bigram);

bool parse_bigram(FILE * input, PhraseLargeTable * phrases,
                  KMixtureModelBigram * bigram);

static ssize_t my_getline(FILE * input){
    ssize_t result = getline(&linebuf, &len, input);
    if ( result == -1 )
        return result;

    linebuf[strlen(linebuf) - 1] = '\0';
    return result;
}

bool parse_body(FILE * input, PhraseLargeTable * phrases,
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
            parse_unigram(input, phrases, bigram);
            goto retry;
        case GRAM_2_LINE:
            my_getline(input);
            parse_bigram(input, phrases, bigram);
            goto retry;
        default:
            assert(false);
        }
    } while (my_getline(input) != -1) ;

 end:
    taglib_pop_state();
    return true;
}

bool parse_unigram(FILE * input, PhraseLargeTable * phrases,
                   KMixtureModelBigram * bigram){
    taglib_push_state();

    assert(taglib_add_tag(GRAM_1_ITEM_LINE, "\\item", 1, "count", ""));

    do {
        assert(taglib_read(linebuf, line_type, values, required));
        switch (line_type) {
        case GRAM_1_ITEM_LINE:{
            /* handle \item in \1-gram */
            const char * string = (const char *) g_ptr_array_index(values, 0);
            phrase_token_t token = taglib_string_to_token(phrases, string);
            gpointer value = NULL;
            assert(g_hash_table_lookup_extended(required, "count",
                                                NULL, &value));
            glong count = atol((const char *)value);
            KMixtureModelArrayHeader array_header;
            memset(&array_header, 0, sizeof(KMixtureModelArrayHeader));
            array_header.m_WC = count;
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

bool parse_bigram(FILE * input, PhraseLargeTable * phrases,
                  KMixtureModelBigram * bigram){
    taglib_push_state();

    assert(taglib_add_tag(GRAM_2_ITEM_LINE, "\\item", 2,
                          "count:T:N_n_0:n_1:Mr", ""));

    phrase_token_t last_token = 0;
    KMixtureModelSingleGram * last_single_gram = NULL;
    do {
        assert(taglib_read(linebuf, line_type, values, required));
        switch (line_type) {
        case GRAM_2_ITEM_LINE:{
            /* handle \item in \2-gram */
            /* two tokens */
            const char * string = (const char *) g_ptr_array_index(values, 0);
            phrase_token_t token1 = taglib_string_to_token(phrases, string);
            string = (const char *) g_ptr_array_index(values, 1);
            phrase_token_t token2 = taglib_string_to_token(phrases, string);

            gpointer value = NULL;
            /* tag: count */
            assert(g_hash_table_lookup_extended(required, "count", NULL, &value));
            glong count = atol((char *)value);
            /* tag: T */
            assert(g_hash_table_lookup_extended(required, "T", NULL, &value));
            glong T = atol((char *)value);
            assert(count == T);
            /* tag: N_n_0 */
            assert(g_hash_table_lookup_extended(required, "N_n_0", NULL, &value));
            glong N_n_0 = atol((char *)value);
            /* tag: n_1 */
            assert(g_hash_table_lookup_extended(required, "n_1", NULL, &value));
            glong n_1 = atol((char *)value);
            /* tag: Mr */
            assert(g_hash_table_lookup_extended(required, "Mr", NULL, &value));
            glong Mr = atol((char *)value);

            KMixtureModelArrayItem array_item;
            memset(&array_item, 0, sizeof(KMixtureModelArrayItem));
            array_item.m_WC = count; array_item.m_N_n_0 = N_n_0;
            array_item.m_n_1 = n_1; array_item.m_Mr = Mr;

            if ( last_token != token1 ) {
                if ( last_token && last_single_gram ) {
                    bigram->store(last_token, last_single_gram);
                    delete last_single_gram;
                    //safe guard
                    last_token = 0;
                    last_single_gram = NULL;
                }
                KMixtureModelSingleGram * single_gram = NULL;
                bigram->load(token1, single_gram);

                //create the new single gram
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
        //safe guard
        last_token = 0;
        last_single_gram = NULL;
    }

    taglib_pop_state();
    return true;
}

int main(int argc, char * argv[]){
    return 0;
}
