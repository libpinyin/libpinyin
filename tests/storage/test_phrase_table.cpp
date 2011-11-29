#include "timer.h"
#include <string.h>
#include "pinyin_internal.h"

size_t bench_times = 1000;

int main(int argc, char * argv[]){
    PhraseLargeTable largetable;

    FILE * gbfile = fopen("../../data/gb_char.table", "r");
    if ( gbfile == NULL ) {
        fprintf(stderr, "open gb_char.table failed!\n");
        return 1;
    }

    largetable.load_text(gbfile);
    fclose(gbfile);

    FILE * gbkfile = fopen("../../data/gbk_char.table", "r");
    if (gbkfile == NULL ) {
        fprintf(stderr, "open gbk_char.table failed!\n");
        return 1;
    }

    largetable.load_text(gbkfile);
    fclose(gbkfile);

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
        utf16_t * new_phrase = g_utf8_to_utf16(linebuf, -1, NULL, NULL, NULL);
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
