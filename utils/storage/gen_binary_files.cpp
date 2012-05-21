/* 
 *  libpinyin
 *  Library to deal with pinyin.
 *  
 *  Copyright (C) 2010 Peng Wu
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
#include <locale.h>
#include "pinyin_internal.h"
#include "utils_helper.h"

void print_help(){
    printf("Usage: gen_binary_files --table-dir <DIRNAME>\n");
}

int main(int argc, char * argv[]){
    int i = 1;
    const char * table_dir = ".";

    setlocale(LC_ALL, "");
    while ( i < argc ){
        if ( strcmp("--help", argv[i]) == 0 ){
            print_help();
            exit(0);
        } else if ( strcmp("--table-dir", argv[i]) == 0){
            if ( ++i >= argc ){
                print_help();
                exit(EINVAL);
            }
            table_dir = argv[i];
        }
        ++i;
    }

    /* generate pinyin index*/
    pinyin_option_t options = USE_TONE;
    ChewingLargeTable chewinglargetable(options);
    PhraseLargeTable phraselargetable;

    /* generate phrase index */
    FacadePhraseIndex phrase_index;
    for (size_t i = 0; i < PHRASE_INDEX_LIBRARY_COUNT; ++i) {
        const char * tablename = pinyin_table_files[i];
        if (NULL == tablename)
            continue;

        gchar * filename = g_build_filename(table_dir, tablename, NULL);
        FILE * tablefile = fopen(filename, "r");

        if (NULL == tablefile) {
            fprintf(stderr, "open %s failed!\n", tablename);
            exit(ENOENT);
        }

        chewinglargetable.load_text(tablefile);
        fseek(tablefile, 0L, SEEK_SET);
        phraselargetable.load_text(tablefile);
        fseek(tablefile, 0L, SEEK_SET);
        phrase_index.load_text(i, tablefile);
        fclose(tablefile);
        g_free(filename);
    }

    MemoryChunk * new_chunk = new MemoryChunk;
    chewinglargetable.store(new_chunk);
    new_chunk->save("pinyin_index.bin");
    chewinglargetable.load(new_chunk);
    
    new_chunk = new MemoryChunk;
    phraselargetable.store(new_chunk);
    new_chunk->save("phrase_index.bin");
    phraselargetable.load(new_chunk);

    phrase_index.compact();

    if (!save_phrase_index(&phrase_index))
        exit(ENOENT);

    return 0;
}
