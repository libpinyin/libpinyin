#include "timer.h"
#include <string.h>
#include "pinyin_internal.h"
#include "tests_helper.h"

size_t bench_times = 1000;

int main(int argc, char * argv[]){
    PhraseLargeTable2 largetable;
    FacadePhraseIndex phrase_index;

    if (!load_phrase_table(NULL, &largetable, &phrase_index))
        exit(ENOENT);

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
        phrase_token_t token = null_token;

        PhraseTokens tokens;
        memset(tokens, 0, sizeof(PhraseTokens));
        phrase_index.prepare_tokens(tokens);

        guint32 start = record_time();
        for ( size_t i = 0; i < bench_times; ++i){
            largetable.search(phrase_len, new_phrase, tokens);
        }
        print_time(start, bench_times);

        int retval = largetable.search(phrase_len, new_phrase, tokens);
        int num = get_first_token(tokens, token);

        phrase_index.destroy_tokens(tokens);

        if ( retval & SEARCH_OK )
            printf("%s: num:%d token:%d\n", linebuf, num, token);
        else
            printf("phrase %s not found.\n", linebuf);

        g_free(new_phrase);
    }

    if ( linebuf )
        free(linebuf);
    return 0;
}
