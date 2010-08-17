#include <stdio.h>
#include <assert.h>
#include <glib.h>
#include "memory_chunk.h"
#include "novel_types.h"
#include "phrase_index.h"
#include "ngram.h"

/* export interpolation model as textual format */

void gen_unigram(FILE * output, FacadePhraseIndex * phrase_index);
void gen_bigram(FILE * output, Bigram * bigram);
const char * token_to_string(phrase_token_t token);

void begin_data(FILE * file){
    fprintf(file, "\\data\n");
}

void end_data(FILE * file){
    fprintf(file, "\\end\n");
}

int main(int argc, char * argv[]){
    FILE * file = stdout;
    const char * bigram_filename = "../../data/bigram.db";

    FacadePhraseIndex phrase_index;

    //gb_char binary file
    MemoryChunk * chunk = new MemoryChunk;
    chunk->load("../../data/gb_char.bin");
    phrase_index.load(1, chunk);

    //gbk_char binary file
    chunk = new MemoryChunk;
    chunk->load("../../data/gbk_char.bin");
    phrase_index.load(2, chunk);

    Bigram bigram;
    bigram.attach(NULL, bigram_filename);

    begin_data(file);

    gen_unigram(stdout, &phrase_index);
    gen_bigram(stdout, &bigram);

    end_data(stdout);
    return 0;
}

void gen_unigram(FILE * output, FacadePhraseIndex * phrase_index) {
    fprintf(output, "\\1-gram\n");
    for ( size_t i = 0; i < PHRASE_INDEX_LIBRARY_COUNT; i++) {
        /* Generate each phrase index library */
        const phrase_token_t min = PHRASE_INDEX_MAKE_TOKEN(i, token_min);
        const phrase_token_t max = PHRASE_INDEX_MAKE_TOKEN(i, token_max);

        PhraseItem item;
        utf16_t buffer[MAX_PHRASE_LENGTH];
        for ( size_t j = min; j < max; j++) {
            int result = phrase_index->get_phrase_item(j, item);
            if ( result == ERROR_NO_SUB_PHRASE_INDEX ||
                 result == ERROR_OUT_OF_RANGE)
                break;
            if ( result == ERROR_NO_ITEM )
                continue;
            assert( result != ERROR_FILE_CORRUPTION );
            /* when get_phrase_item, the next error is impossible */
            assert( result != ERROR_INTEGER_OVERFLOW );
            assert( result == ERROR_OK);

            size_t freq = item.get_unigram_frequency();
            /* deal with the special phrase index, for "<start>..." */
            if ( i == 0 ) {
                const char * phrase = token_to_string(j);
                if ( NULL == phrase )
                    continue;
                fprintf(output, "\\item %s %d\n", phrase, freq);
                continue;
            }
            item.get_phrase_string(buffer);
            guint8 length = item.get_phrase_length();
            gchar * phrase = g_utf16_to_utf8(buffer, length, NULL, NULL, NULL);
            fprintf(output, "\\item %s %d\n", phrase, freq);
            g_free(phrase);
        }
    }
}

void gen_bigram(FILE * output, Bigram * bigram){

}

const char * token_to_string(phrase_token_t token){
    struct token_pair{
        phrase_token_t token;
        const char * string;
    };

    static const token_pair tokens [] = {
        {sentence_start, "<start>"},
        {0, NULL}
    };

    const token_pair * pair = tokens;
    while (pair->token) {
        if ( token == pair->token )
            return pair->string;
    }

    return NULL;
}
