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

#include "timer.h"
#include <errno.h>
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "pinyin_internal.h"


static const gchar * parsername = "";
static gboolean incomplete = FALSE;
static const gchar * schemename = "";

static GOptionEntry entries[] =
{
    {"parser", 'p', 0, G_OPTION_ARG_STRING, &parsername, "parser", "fullpinyin doublepinyin zhuyin pinyindirect zhuyindirect"},
    {"incomplete", 'i', 0, G_OPTION_ARG_NONE, &incomplete, "incomplete pinyin", NULL},
    {"scheme", 's', 0, G_OPTION_ARG_STRING, &schemename, "scheme", "standard hsu dachen26"},
    {NULL}
};


size_t bench_times = 1000;

using namespace pinyin;


int main(int argc, char * argv[]) {
    GError * error = NULL;
    GOptionContext * context;

    context = g_option_context_new("- test pinyin parser");
    g_option_context_add_main_entries(context, entries, NULL);
    if (!g_option_context_parse(context, &argc, &argv, &error)) {
        g_print("option parsing failed:%s\n", error->message);
        exit(EINVAL);
    }

    pinyin_option_t options = PINYIN_CORRECT_ALL | USE_TONE | USE_RESPLIT_TABLE;
    if (incomplete)
        options |= PINYIN_INCOMPLETE | ZHUYIN_INCOMPLETE;

    PhoneticParser2 * parser = NULL;
    ChewingKeyVector keys = g_array_new(FALSE, FALSE, sizeof(ChewingKey));
    ChewingKeyRestVector key_rests =
        g_array_new(FALSE, FALSE, sizeof(ChewingKeyRest));

    /* create the parser */
    if (strcmp("fullpinyin", parsername) == 0) {
        parser = new FullPinyinParser2();
    } else if (strcmp("doublepinyin", parsername) == 0) {
        parser = new DoublePinyinParser2();
    } else if (strcmp("zhuyin", parsername) == 0) {
        if (strcmp("standard", schemename) == 0) {
            parser = new ZhuyinSimpleParser2();
        } else if (strcmp("hsu", schemename) == 0) {
            parser = new ZhuyinDiscreteParser2();
        } else if (strcmp("dachen26", schemename) == 0) {
            parser = new ZhuyinDaChenCP26Parser2();
        }
    } else if (strcmp("pinyindirect", parsername) == 0) {
        parser = new PinyinDirectParser2();
    } else if (strcmp("zhuyindirect", parsername) == 0) {
        parser = new ZhuyinDirectParser2();
    }


    if (!parser)
        parser = new FullPinyinParser2();

    char* linebuf = NULL; size_t size = 0; ssize_t read;
    while( (read = getline(&linebuf, &size, stdin)) != -1 ){
        if ( '\n' == linebuf[strlen(linebuf) - 1] ) {
            linebuf[strlen(linebuf) - 1] = '\0';
        }

        if ( strcmp ( linebuf, "quit" ) == 0)
            break;

#if 0
        ChewingKey key;
        bool success = parser->parse_one_key(options, key,
                                             linebuf, strlen(linebuf));
        if (success) {
            gchar * pinyins = key.get_pinyin_string();
            printf("pinyin:%s\n", pinyins);
            g_free(pinyins);
        }
#endif

#if 1
        int len = 0;
        guint32 start_time = record_time();
        for ( size_t i = 0; i < bench_times; ++i)
            len = parser->parse(options, keys, key_rests,
                                linebuf, strlen(linebuf));

        print_time(start_time, bench_times);

        printf("parsed %d chars, %d keys.\n", len, keys->len);

        assert(keys->len == key_rests->len);

        for (size_t i = 0; i < keys->len; ++i) {
            ChewingKey * key =
                &g_array_index(keys, ChewingKey, i);
            ChewingKeyRest * key_rest =
                &g_array_index(key_rests, ChewingKeyRest, i);

            gchar * pinyins = key->get_pinyin_string();
            printf("%s %d %d\t", pinyins,
                   key_rest->m_raw_begin, key_rest->m_raw_end);
            g_free(pinyins);
        }
        printf("\n");
#endif

    }

    if (linebuf)
        free(linebuf);

    delete parser;

    g_array_free(key_rests, TRUE);
    g_array_free(keys, TRUE);

    return 0;
}
