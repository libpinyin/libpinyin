#include <stdio.h>
#include <sys/time.h>
#include "novel_types.h"
#include "phrase_large_table.h"

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


}
