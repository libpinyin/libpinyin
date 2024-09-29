/* 
 *  libpinyin
 *  Library to deal with pinyin.
 *  
 *  Copyright (C) 2024 Peng Wu <alexepico@gmail.com>
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
#include "pinyin_internal.h"

void print_table(PunctTable & table) {
    GArray * items = g_array_new(TRUE, TRUE, sizeof(phrase_token_t));

    table.get_all_items(items);
    for (guint i = 0; i < items->len; ++i) {
        gchar ** puncts = NULL;
        phrase_token_t token = g_array_index(items, phrase_token_t, i);
        printf("token: %d\n", token);
        table.get_all_punctuations(token, puncts);

        if (puncts) {
            gchar * line = g_strjoinv(" ", puncts);
            printf("Punctuations: %s\n", line);
            g_free(line);
        }

        g_strfreev(puncts);
    }

    g_array_free(items, TRUE);
}

int main(int argc, char * argv[]){
    PunctTable table;
    check_result(table.attach("/tmp/punct.bin", ATTACH_CREATE|ATTACH_READWRITE));
    printf("created table.\n");
    print_table(table);

    table.append_punctuation(1, "……");
    table.append_punctuation(1, "…");
    table.append_punctuation(1, "？");
    printf("insert some punctuations.\n");
    print_table(table);

    table.remove_punctuation(1, "…");
    printf("remove some punctuations.\n");
    print_table(table);

    check_result(table.save_db("/tmp/snapshot.db"));
    check_result(table.load_db("/tmp/snapshot.db"));
    printf("after save and load table.\n");
    print_table(table);

    table.remove_punctuation(1, "……");
    printf("remove some punctuations.\n");
    print_table(table);

    table.remove_punctuation(1, "？");
    printf("remove some punctuations.\n");
    print_table(table);

    return 0;
}
