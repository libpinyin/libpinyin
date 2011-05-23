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


#include "pinyin.h"
#include <glib.h>
#include <locale.h>
#include "k_mixture_model.h"

typedef GHashTable * HashofWordPair;
typedef GHashTable * HashofSecondWord;

/* Hash token of Hash token of word count. */
static HashofWordPair g_hash_of_document = NULL;
static PhraseLargeTable * g_phrases = NULL;
static KMixtureModelBigram * g_k_mixture_model = NULL;
static guint32 g_maximum_occurs = 20;
static parameter_t g_maximum_increase_rates = 3.;
static bool g_train_pi_gram = true;


void print_help(){
    printf("Usage: gen_k_mixture_model [--skip-pi-gram-training]\n");
    printf("                           [--maximum-ocurrs-allowed <INT>]\n");
    printf("                           [--maximum-increase-rates-allowed <FLOAT>]\n");
    printf("                           [--k-mixture-model-file <FILENAME>]\n");
    printf("                           {<FILENAME>}+\n");
}


bool read_document(FILE * document){
    char * linebuf = NULL;
    size_t size = 0;
    phrase_token_t last_token, cur_token = last_token = 0;

    while ( getline(&linebuf, &size, document) ){
        if ( feof(document) )
            break;
        /* Note: check '\n' here? */
        linebuf[strlen(linebuf) - 1] = '\0';

        glong phrase_len = 0;
        utf16_t * phrase = g_utf8_to_utf16(linebuf, -1, NULL, &phrase_len, NULL);

        if ( phrase_len == 0 )
            continue;

        phrase_token_t token = 0;
        int search_result = g_phrases->search( phrase_len, phrase, token );
        if ( ! (search_result & SEARCH_OK) )
            token = 0;

        g_free(phrase);
        phrase = NULL;

        last_token = cur_token;
        cur_token = token;

        /* skip null_token in second word. */
        if ( null_token == cur_token )
            continue;

        /* skip pi-gram training. */
        if ( null_token == last_token ){
            if ( !g_train_pi_gram )
                continue;
            last_token = sentence_start;
        }

        /* remember the (last_token, cur_token) word pair. */
        gpointer value = NULL;
        HashofSecondWord hash_of_second_word = NULL;
        gboolean lookup_result = g_hash_table_lookup_extended
            (g_hash_of_document, GUINT_TO_POINTER(last_token),
             NULL, &value);
        if ( !lookup_result ){
            hash_of_second_word = g_hash_table_new
                (g_direct_hash, g_direct_equal);
        } else {
            hash_of_second_word = (HashofSecondWord) value;
        }

        value = NULL;
        lookup_result = g_hash_table_lookup_extended
            (hash_of_second_word, GUINT_TO_POINTER(cur_token),
             NULL, &value);
        guint32 count = 0;
        if ( lookup_result ) {
            count = GPOINTER_TO_UINT(value);
        }
        count ++;
        g_hash_table_insert(hash_of_second_word,
                            GUINT_TO_POINTER(cur_token),
                            GUINT_TO_POINTER(count));
        g_hash_table_insert(g_hash_of_document,
                            GUINT_TO_POINTER(last_token),
                            hash_of_second_word);
    }

    free(linebuf);

    return true;
}

static void train_word_pair(gpointer key, gpointer value,
                            gpointer user_data){
    phrase_token_t token = GPOINTER_TO_UINT(key);
    guint32 count = GPOINTER_TO_UINT(value);
    KMixtureModelSingleGram * single_gram =
        (KMixtureModelSingleGram *)user_data;
    KMixtureModelArrayItem array_item;
    guint32 delta = 0;

    bool exists = single_gram->get_array_item(token, array_item);
    if ( exists ) {
        guint32 maximum_occurs_allowed = std_lite::max
            (g_maximum_occurs,
             (guint32)ceil(array_item.m_Mr * g_maximum_increase_rates));
        /* Exceeds the maximum occurs allowed of the word or phrase,
         * in a single document.
         */
        if ( count > maximum_occurs_allowed )
            return;
        array_item.m_WC += count;
        /* array_item.m_T += count; the same as m_WC. */
        array_item.m_N_n_0 ++;
        if ( 1 == count )
            array_item.m_n_1 ++;
        array_item.m_Mr = std_lite::max(array_item.m_Mr, count);
        delta = count;
        assert(single_gram->set_array_item(token, array_item));
    } else { /* item doesn't exist. */
        /* the same as above. */
        if ( count > g_maximum_occurs )
            return;
        memset(&array_item, 0, sizeof(KMixtureModelArrayItem));
        array_item.m_WC = count;
        /* array_item.m_T = count; the same as m_WC. */
        array_item.m_N_n_0 = 1;
        if ( 1 == count )
            array_item.m_n_1 = 1;
        array_item.m_Mr = count;
        delta = count;
        assert(single_gram->insert_array_item(token, array_item));
    }
    /* save delta in the array header. */
    KMixtureModelArrayHeader array_header;
    single_gram->get_array_header(array_header);
    array_header.m_WC += delta;
    single_gram->set_array_header(array_header);
}

bool train_single_gram(phrase_token_t token,
                       KMixtureModelSingleGram * single_gram,
                       guint32 & delta){
    assert(NULL != single_gram);
    delta = 0; /* delta in WC of single_gram. */
    KMixtureModelArrayHeader array_header;
    assert(single_gram->get_array_header(array_header));
    guint32 saved_array_header_WC = array_header.m_WC;

    HashofSecondWord hash_of_second_word = NULL;
    gpointer value = NULL;
    assert(g_hash_table_lookup_extended
           (g_hash_of_document, GUINT_TO_POINTER(token),
            NULL, &value));
    hash_of_second_word = (HashofSecondWord) value;
    assert(NULL != hash_of_second_word);

    g_hash_table_foreach(hash_of_second_word, train_word_pair, single_gram);

    assert(single_gram->get_array_header(array_header));
    delta = array_header.m_WC - saved_array_header_WC;
    return true;
}

static void hash_of_document_train_wrapper(gpointer key, gpointer value, gpointer user_data){
    phrase_token_t token = GPOINTER_TO_UINT(key);
    guint32 delta = 0;

    KMixtureModelSingleGram * single_gram = NULL;
    bool exists = g_k_mixture_model->load(token, single_gram);
    if ( !exists )
        single_gram = new KMixtureModelSingleGram;
    train_single_gram(token, single_gram, delta);

    KMixtureModelMagicHeader magic_header;
    if (!g_k_mixture_model->get_magic_header(magic_header)){
        /* the first time to access the new k mixture model file. */
        memset(&magic_header, 0, sizeof(KMixtureModelMagicHeader));
    }

    if ( magic_header.m_WC + delta < magic_header.m_WC ){
        fprintf(stderr, "the m_WC integer in magic header overflows.\n");
        return;
    }
    magic_header.m_WC += delta;
    magic_header.m_N ++;
    assert(g_k_mixture_model->set_magic_header(magic_header));

    /* save the single gram. */
    assert(g_k_mixture_model->store(token, single_gram));
    delete single_gram;
}

static gboolean hash_of_document_free_wrapper(gpointer key, gpointer value, gpointer user_data){
    phrase_token_t token = GPOINTER_TO_UINT(key);
    HashofSecondWord second_word = (HashofSecondWord) value;
    g_hash_table_unref(second_word);
    return TRUE;
}

int main(int argc, char * argv[]){
    int i = 1;
    const char * k_mixture_model_filename = NULL;

    setlocale(LC_ALL, "");
    while ( i < argc ){
        if ( strcmp("--help", argv[i]) == 0 ){
            print_help();
            exit(0);
        } else if ( strcmp("--skip-pi-gram-training", argv[i]) == 0 ){
            g_train_pi_gram = false;
        } else if ( strcmp("--maximum-ocurrs-allowed", argv[i]) == 0 ){
            if ( ++i >= argc ){
                print_help();
                exit(EINVAL);
            }
            g_maximum_occurs = atoi(argv[i]);
        } else if ( strcmp("--maximum-increase-rates-allowed", argv[i]) == 0 ){
            if ( ++i >= argc ){
                print_help();
                exit(EINVAL);
            }
            g_maximum_increase_rates = atof(argv[i]);
        } else if ( strcmp("--k-mixture-model-file", argv[i]) == 0 ){
            if ( ++i >= argc ){
                print_help();
                exit(EINVAL);
            }
            k_mixture_model_filename = argv[i];
        } else {
            break;
        }
        ++i;
    }

    g_phrases = new PhraseLargeTable;
    MemoryChunk * chunk = new MemoryChunk;
    chunk->load("../../data/phrase_index.bin");
    g_phrases->load(chunk);

    g_k_mixture_model = new KMixtureModelBigram(K_MIXTURE_MODEL_MAGIC_NUMBER);
    g_k_mixture_model->attach(k_mixture_model_filename, ATTACH_READWRITE|ATTACH_CREATE);

    while ( i < argc ){
        const char * filename = argv[i];
        FILE * document = fopen(filename, "r");
        if ( NULL == document ){
            int err_saved = errno;
            fprintf(stderr, "can't open file: %s.\n", filename);
            fprintf(stderr, "error:%s.\n", strerror(err_saved));
            exit(err_saved);
        }

        g_hash_of_document = g_hash_table_new
            (g_direct_hash, g_direct_equal);

        assert(read_document(document));
        fclose(document);

        /* train the document, and convert it to k mixture model. */
        g_hash_table_foreach(g_hash_of_document,
                             hash_of_document_train_wrapper, NULL);

        /* free resources of g_hash_of_document */
        g_hash_table_foreach_steal(g_hash_of_document,
                                   hash_of_document_free_wrapper, NULL);
        g_hash_table_unref(g_hash_of_document);
        g_hash_of_document = NULL;

        ++i;
    }

    delete g_phrases;
    delete g_k_mixture_model;

    return 0;
}
