#include "timer.h"
#include <string.h>
#include "pinyin_internal.h"

size_t bench_times = 1000;

int main(int argc, char * argv[]){
    PhraseLargeTable largetable;

    for (size_t i = 0; i < PHRASE_INDEX_LIBRARY_COUNT; ++i) {
        const char * tablename = pinyin_table_files[i];
        if (NULL == tablename)
            continue;

        gchar * filename = g_build_filename("..", "..", "data",
                                            tablename, NULL);
        FILE * tablefile = fopen(filename, "r");
        if ( tablefile == NULL ) {
            fprintf(stderr, "open %s failed!\n", tablename);
            return 1;
        }

        largetable.load_text(tablefile);
        fclose(tablefile);
        g_free(filename);
    }

    MemoryChunk * chunk = new MemoryChunk;
    largetable.store(chunk);
    largetable.load(chunk);

    char * linebuf = NULL;
    size_t size = 0;
    while( getline(&linebuf, &size, stdin) ){
        linebuf[strlen(linebuf) - 1] = '\0';
        if ( strcmp ( linebuf, "quit" ) == 0)
            break;

        glong phrase_len = g_utf8_strlen(linebuf, -1);
        ucs4_t * new_phrase = g_utf8_to_ucs4(linebuf, -1, NULL, NULL, NULL);
        phrase_token_t token;

        guint32 start = record_time();
        for ( size_t i = 0; i < bench_times; ++i){
            largetable.search(phrase_len, new_phrase, token);
        }
        print_time(start, bench_times);

        int retval = largetable.search(phrase_len, new_phrase, token);
        if ( retval & SEARCH_OK )
            printf("%s:\t%d\n", linebuf, token);
        else
            printf("phrase %s not found.\n", linebuf);

        g_free(new_phrase);
    }

    if ( linebuf )
        free(linebuf);
    return 0;
}
