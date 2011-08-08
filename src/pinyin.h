#include <stdio.h>
#include "novel_types.h"
#include "memory_chunk.h"
#include "pinyin_base.h"
#include "pinyin_phrase.h"
#include "pinyin_large_table.h"
#include "phrase_large_table.h"
#include "phrase_index.h"
#include "ngram.h"
#include "lookup.h"
#include "pinyin_lookup.h"
#include "phrase_lookup.h"
#include "tag_utility.h"

/* training module */
#include "flexible_ngram.h"

typedef MatchResults TokenVector;
typedef struct _pinyin_context_t pinyin_context_t;
