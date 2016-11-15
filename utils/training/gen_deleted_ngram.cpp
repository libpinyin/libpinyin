/* 
 *  libpinyin
 *  Library to deal with pinyin.
 *  
 *  Copyright (C) 2006-2007, 2011 Peng Wu
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>
#include <glib.h>
#include "pinyin_internal.h"
#include "utils_helper.h"

static gboolean train_pi_gram = TRUE;
static const gchar * bigram_filename = DELETED_BIGRAM;

static GOptionEntry entries[] =
{
    {"skip-pi-gram-training", 0, G_OPTION_FLAG_REVERSE, G_OPTION_ARG_NONE, &train_pi_gram, "skip pi-gram training", NULL},
    {"deleted-bigram-file", 0, 0, G_OPTION_ARG_FILENAME, &bigram_filename, "deleted bi-gram file", NULL},
    {NULL}
};


int main(int argc, char * argv[]){
    setlocale(LC_ALL, "");

    GError * error = NULL;
    GOptionContext * context;

    context = g_option_context_new("- generate deleted n-gram");
    g_option_context_add_main_entries(context, entries, NULL);
    if (!g_option_context_parse(context, &argc, &argv, &error)) {
        g_print("option parsing failed:%s\n", error->message);
        exit(EINVAL);
    }

    SystemTableInfo2 system_table_info;

    bool retval = system_table_info.load(SYSTEM_TABLE_INFO);
    if (!retval) {
        fprintf(stderr, "load table.conf failed.\n");
        exit(ENOENT);
    }

    FacadePhraseIndex phrase_index;

    const pinyin_table_info_t * phrase_files =
        system_table_info.get_default_tables();

    if (!load_phrase_index(phrase_files, &phrase_index))
        exit(ENODATA);

    Bigram bigram;
    bigram.attach(bigram_filename, ATTACH_CREATE|ATTACH_READWRITE);

    char* linebuf = NULL; size_t size = 0;
    phrase_token_t last_token, cur_token = last_token = 0;
    while( getline(&linebuf, &size, stdin) ){
	if ( feof(stdin) )
	    break;

        if ( '\n' == linebuf[strlen(linebuf) - 1] ) {
            linebuf[strlen(linebuf) - 1] = '\0';
        }

        TAGLIB_PARSE_SEGMENTED_LINE(&phrase_index, token, linebuf);

	last_token = cur_token;
	cur_token = token;

        /* skip null_token in second word. */
        if ( null_token == cur_token )
            continue;

        /* skip pi-gram training. */
        if ( null_token == last_token ){
            if ( !train_pi_gram )
                continue;
            last_token = sentence_start;
        }

        /* train bi-gram */
        SingleGram * single_gram = NULL;
        bigram.load(last_token, single_gram);

        if ( NULL == single_gram ){
            single_gram = new SingleGram;
        }
        guint32 freq, total_freq;
        //increase freq
        if (single_gram->get_freq(cur_token, freq))
            assert(single_gram->set_freq(cur_token, freq + 1));
        else
            assert(single_gram->insert_freq(cur_token, 1));
        //increase total freq
        single_gram->get_total_freq(total_freq);
        single_gram->set_total_freq(total_freq + 1);
        
        bigram.store(last_token, single_gram);
        delete single_gram;
    }

    free(linebuf);
    return 0;
}
