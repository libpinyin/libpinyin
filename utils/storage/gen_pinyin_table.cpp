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


#include <stdio.h>
#include <glib.h>
#include "pinyin_internal.h"


void print_help(){
    printf("Usage: gen_pinyin_table -t <PHRASE_INDEX> \n"
           "-o <OUTPUTFILE> <FILE1> <FILE2> .. <FILEn>\n"
           "<OUTPUTFILE> the result output file\n"
           "<FILEi> input pinyin files\n"
           "<PHRASE_INDEX> phrase index identifier\n");
}


static gint phrase_index = 0;
static const gchar * outputfile = "temp.out";

static GOptionEntry entries[] =
    {
        {"phraseindex", 't', 0, G_OPTION_ARG_INT, &phrase_index, "phrase index", NULL},
        {"outputfile", 'o', 0, G_OPTION_ARG_FILENAME, &outputfile, "output filename", NULL},
        {NULL}
    };


using namespace pinyin;

/* map from phrase_item to GArray of chewing_and_freq_item */
GTree  * g_chewing_tree;
/* Array of GArray of phrase_and_array_item */
GArray * g_item_array[MAX_PHRASE_LENGTH + 1];

struct phrase_item{
    size_t length;
    gunichar * uniphrase;
};

struct chewing_and_freq_item{
    ChewingKeyVector keys;
    ChewingKeyRestVector key_rests;
    guint32 freq;
};

struct phrase_and_array_item{
    phrase_item phrase;                    /* the key of g_chewing_tree */
    /* Array of chewing_and_freq_item */
    GArray * chewing_and_freq_array;       /* the value of g_chewing_tree */
};


void feed_file(const char * filename);

void feed_line(const char * phrase, const char * pinyin, const guint32 freq);

gboolean store_one_item(gpointer key, gpointer value, gpointer data);

int phrase_array_compare(gconstpointer lhs, gconstpointer rhs,
                         gpointer userdata);

void gen_phrase_file(const char * outputfile, int phrase_index);


gint phrase_item_compare(gconstpointer a, gconstpointer b){
    phrase_item * itema = (phrase_item *) a;
    phrase_item * itemb = (phrase_item *) b;
    if ( itema->length != itemb->length )
        return itema->length - itemb->length;
    else
        return memcmp(itema->uniphrase, itemb->uniphrase,
                      sizeof(gunichar) * itema->length);
}


int main(int argc, char * argv[]){
    int i;

    g_chewing_tree = g_tree_new(phrase_item_compare);

    GError * error = NULL;
    GOptionContext * context;

    context = g_option_context_new("- generate pinyin table");
    g_option_context_add_main_entries(context, entries, NULL);
    if (!g_option_context_parse(context, &argc, &argv, &error)) {
        g_print("option parsing failed:%s\n", error->message);
        exit(EINVAL);
    }

    for (i = 1; i < argc; ++i) {
        feed_file(argv[i]);
    }

    printf("nnodes: %d\n", g_tree_nnodes(g_chewing_tree));

    /* store in item array */
    g_item_array[0] = NULL;
    for (i = 1; i < MAX_PHRASE_LENGTH + 1; ++i){
        g_item_array[i] = g_array_new
            (FALSE, TRUE, sizeof(phrase_and_array_item));
    }
    g_tree_foreach(g_chewing_tree, store_one_item, NULL);

    /* sort item array */
    for ( int i = 1; i < MAX_PHRASE_LENGTH + 1; ++i){
        g_array_sort_with_data(g_item_array[i], phrase_array_compare , &i);
    }

    gen_phrase_file(outputfile, phrase_index);

    return 0;
}

void feed_file ( const char * filename){
    char phrase[1024], pinyin[1024];
    guint32 freq;

    FILE * infile = fopen(filename, "r");
    if ( NULL == infile ){
        fprintf(stderr, "Can't open file %s.\n", filename);
        exit(ENOENT);
    }

    while ( !feof(infile)){
        int num = fscanf(infile, "%1023s %1023s %u",
                         phrase, pinyin, &freq);

        if (3 != num)
            continue;

        if (feof(infile))
            break;

        feed_line(phrase, pinyin, freq);
    }

    fclose(infile);
}

void feed_line(const char * phrase, const char * pinyin, const guint32 freq) {
    phrase_item * item = new phrase_item;
    item->length = g_utf8_strlen(phrase, -1);

    /* FIXME: modify ">" to ">=" according to pinyin_large_table.cpp
     * where is the code which I don't want to touch. :-)
     */

    if (item->length >= MAX_PHRASE_LENGTH) {
        fprintf(stderr, "Too long phrase:%s\t%s\t%d\n", phrase, pinyin, freq);
        delete item;
        return;
    }

    item->uniphrase = g_utf8_to_ucs4(phrase, -1, NULL, NULL, NULL);

    FullPinyinParser2 parser;
    ChewingKeyVector keys = g_array_new(FALSE, FALSE, sizeof(ChewingKey));
    ChewingKeyRestVector key_rests = g_array_new
        (FALSE, FALSE, sizeof(ChewingKeyRest));

    pinyin_option_t options = PINYIN_CORRECT_ALL | USE_TONE;
    parser.parse(options, keys, key_rests, pinyin, strlen(pinyin));
    assert(keys->len == key_rests->len);

    if (keys->len != item->length) {
        fprintf(stderr, "Invalid pinyin:%s\t%s\t%d\n", phrase, pinyin, freq);
        delete item;
        return;
    }

    GArray * array = (GArray *)g_tree_lookup(g_chewing_tree, item);

    chewing_and_freq_item value_item;
    value_item.keys = keys; value_item.key_rests = key_rests;
    value_item.freq = freq;

    assert(item->length == value_item.keys->len);
    if (NULL == array) {
        array = g_array_new(FALSE, FALSE, sizeof(chewing_and_freq_item));
        g_array_append_val(array, value_item);
        g_tree_insert(g_chewing_tree, item, array);
        return;
    }

    bool found = false;
    for (size_t i = 0; i < array->len; ++i) {
        chewing_and_freq_item * cur_item =
            &g_array_index(array, chewing_and_freq_item, i);
        int result = pinyin_exact_compare2
            ((ChewingKey *) value_item.keys->data,
             (ChewingKey *) cur_item->keys->data,
             value_item.keys->len);

        if (0 == result) {
            fprintf(stderr, "Duplicate item: phrase:%s\tpinyin:%s\tfreq:%u\n",
                    phrase, pinyin, freq);
            cur_item->freq += freq;
            found = true;
        }
    }

    if (!found) {
        g_array_append_val(array, value_item);
        g_tree_insert(g_chewing_tree, item, array);
    } else {
        /* clean up */
        g_array_free(keys, TRUE);
        g_array_free(key_rests, TRUE);
    }

    delete item;
}


gboolean store_one_item(gpointer key, gpointer value, gpointer data) {
    phrase_and_array_item item;
    item.phrase = *((phrase_item *) key);
    item.chewing_and_freq_array = (GArray *) value;
    int len = item.phrase.length;
    g_array_append_val(g_item_array[len], item);
    return FALSE;
}


int phrase_array_compare(gconstpointer lhs, gconstpointer rhs,
                         gpointer userdata) {
    int phrase_length = *((int *) userdata);
    phrase_and_array_item * item_lhs = (phrase_and_array_item *) lhs;
    phrase_and_array_item * item_rhs = (phrase_and_array_item *) rhs;

    ChewingKeyVector keys_lhs = g_array_index
        (item_lhs->chewing_and_freq_array, chewing_and_freq_item, 0).keys;
    ChewingKeyVector keys_rhs = g_array_index
        (item_rhs->chewing_and_freq_array, chewing_and_freq_item, 0).keys;
    return pinyin_exact_compare2((ChewingKey *)keys_lhs->data,
                                 (ChewingKey *)keys_rhs->data, phrase_length);
}


void gen_phrase_file(const char * outputfile, int phrase_index){
    FILE * outfile = fopen(outputfile, "w");
    if (NULL == outfile ) {
        fprintf(stderr, "Can't write file %s.\n", outputfile);
        exit(ENOENT);
    }

    phrase_token_t token = 1;

    /* phrase length index */
    for (size_t i = 1; i < MAX_PHRASE_LENGTH + 1; ++i) {
        GArray * item_array = g_item_array[i];

        /* item array index */
        for (size_t m = 0; m < item_array->len; ++m) {
            phrase_and_array_item * item = &g_array_index
                (item_array, phrase_and_array_item, m);
            phrase_item phrase = item->phrase;
            GArray * chewing_and_freqs = item->chewing_and_freq_array;

            gchar * phrase_str = g_ucs4_to_utf8
                (phrase.uniphrase, phrase.length, NULL, NULL, NULL);

            /* iterate each pinyin */
            for (size_t n = 0; n < chewing_and_freqs->len; ++n) {
                chewing_and_freq_item * chewing_and_freq =
                    &g_array_index
                    (chewing_and_freqs, chewing_and_freq_item, n);

                ChewingKeyVector keys = chewing_and_freq->keys;

                GArray * pinyins = g_array_new(TRUE, FALSE, sizeof(gchar *));
                gchar * pinyin = NULL;

                size_t k;
                for (k = 0; k < keys->len; ++k) {
                    ChewingKey key = g_array_index(keys, ChewingKey, k);

                    //assert (CHEWING_ZERO_TONE != key.m_tone);
                    pinyin = key.get_pinyin_string();
                    g_array_append_val(pinyins, pinyin);
                }
                gchar * pinyin_str = g_strjoinv("'", (gchar **)pinyins->data);

                for (k = 0; k < pinyins->len; ++k) {
                    g_free(g_array_index(pinyins, gchar *, k));
                }
                g_array_free(pinyins, TRUE);

                guint32 freq = chewing_and_freq->freq;

                /* avoid zero freq. */
                if (freq < 3) freq = 3;

                fprintf(outfile, "%s\t%s\t%d\t%d\n",
                        pinyin_str, phrase_str,
                        PHRASE_INDEX_MAKE_TOKEN(phrase_index, token), freq);

                g_free(pinyin_str);
            }
            g_free(phrase_str);
            token++;
        }
    }

    fclose(outfile);
}
