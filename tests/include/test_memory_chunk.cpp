#include <stdio.h>
#include "pinyin_internal.h"

//Test Memory Chunk Functionality
int main(int argc, char * argv[]){
    MemoryChunk* chunk;
    chunk = new MemoryChunk();
    int i = 12;
    chunk->set_content(0, &i, sizeof(int));

    int * p = (int *)chunk->begin();
    assert(chunk->size() == sizeof(int));
    printf("%d\n", *p);
    printf("%ld\n", chunk->capacity());

    p = & i;
    chunk->set_chunk(p, sizeof(int), NULL);
    short t = 5;
    chunk->set_content(sizeof(int), &t, sizeof(short));
    assert( sizeof(int) + sizeof(short) == chunk->size());
    printf("%ld\n", chunk->capacity());

    p = (int *)chunk->begin();
    short * p2 =(short *)(((char *) (chunk->begin())) + sizeof(int));
    printf("%d\t%d\n", *p, *p2);

    chunk->set_content(sizeof(int) + sizeof(short), &t, sizeof(short));
  
    assert( sizeof(int) + (sizeof(short) << 1) == chunk->size());
    printf("%ld\n", chunk->capacity());
    p = (int *)chunk->begin();
    p2 =(short *)(((char *) (chunk->begin())) + sizeof(int));
    printf("%d\t%d\t%d\n", *p, *p2, *(p2 + 1));

    chunk->set_size(sizeof(int) + sizeof(short) *3);
    p = (int *)chunk->begin();
    p2 =(short *)(((char *) (chunk->begin())) + sizeof(int));

    chunk->set_content(0, &i, sizeof(int));

    *(p2+2) = 3;
    printf("%d\t%d\t%d\t%d\n", *p, *p2, *(p2 + 1), *(p2+2));

    int m = 10;
    chunk->set_chunk(&m, sizeof(int), NULL);
    int n = 12;
    chunk->insert_content(sizeof(int), &n, sizeof(int));
    n = 11;
    chunk->insert_content(sizeof(int), &n, sizeof(int));

    int * p3 = (int *)chunk->begin();
    printf("%d\t%d\t%d\n", *p3, *(p3+1), *(p3+2));
	
    chunk->remove_content(sizeof(int), sizeof(int));
    printf("%d\t%d\n", *p3, *(p3+1));

    int tmp;
    assert(chunk->get_content(sizeof(int), &tmp, sizeof(int)));
    printf("%d\n", tmp);

    //clean up chunk
    delete chunk;

    const char * filename =  "/tmp/version";
    const char * version = "0.2.0";

    //testing version check.
    chunk =  new MemoryChunk;
    bool retval = chunk->load(filename);
    if ( !retval ){
        fprintf(stderr, "can't find chunk\n");
    }else if ( memcmp(version, chunk->begin(), strlen(version) + 1) == 0){
        printf("match\n");
    }

    chunk->set_content(0, version, strlen(version) + 1);
    chunk->save(filename);

    retval = chunk->load(filename);
    if ( !retval ){
        fprintf(stderr, "can't find chunk\n");
    } else if ( memcmp(version, chunk->begin(), strlen(version) + 1) == 0){
        printf("match\n");
    }

    return 0;
}
