#include <string.h>
#include <stdio.h>
#include <sys/time.h>
#include "pinyin.h"

size_t bench_times = 1000;

guint32 record_time ()
{
    timeval tv;
    gettimeofday(&tv, NULL);
    return (guint32) tv.tv_sec * 1000000 + tv.tv_usec;
}

void print_time (guint32 old_time, guint32 times)
{
    timeval tv;
    gettimeofday (&tv, NULL);

    guint32 wasted = (guint32) tv.tv_sec * 1000000 + tv.tv_usec - old_time;

    printf("Spent %d us for %d operations, %f us/op, %f times/s.\n\n", wasted, times, ((double)wasted)/times, times * 1000000.0/wasted);
}


int main(int argc, char * argv[]){
    PhraseLargeTable largetable;

    FILE * gbfile = fopen("../../data/gb_char.table", "r");
    if ( gbfile == NULL ) {
        printf("open gb_char.table failed!\n");
        return 1;
    }

    largetable.load_text(gbfile);
    fclose(gbfile);

    FILE * gbkfile = fopen("../../data/gbk_char.table", "r");
    if (gbkfile == NULL ) {
        printf("open gbk_char.table failed!\n");
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
