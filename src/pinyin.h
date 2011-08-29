#include <stdio.h>
#include "novel_types.h"
#include "memory_chunk.h"
#include "pinyin_base.h"
#include "pinyin_phrase.h"
#include "pinyin_large_table.h"
#include "phrase_large_table.h"
#include "phrase_index.h"
#include "phrase_index_logger.h"
#include "ngram.h"
#include "lookup.h"
#include "pinyin_lookup.h"
#include "phrase_lookup.h"
#include "tag_utility.h"

/* training module */
#include "flexible_ngram.h"

using namespace pinyin;

typedef struct _pinyin_context_t pinyin_context_t;

pinyin_context_t * pinyin_init(const char * systemdir, const char * userdir);
void pinyin_fini(pinyin_context_t * context);

bool pinyin_set_options(pinyin_context_t * context,
                        PinyinCustomSettings * custom);
bool pinyin_set_pinyin_keys(pinyin_context_t * context,
                            PinyinKeyVector pinyin_keys);

bool pinyin_get_guessed_sentence(pinyin_context_t * context,
                                 char ** sentence);
bool pinyin_parse_full(pinyin_context_t * context,
                       const char * onepinyin,
                       PinyinKey * onekey);
bool pinyin_parse_more_fulls(pinyin_context_t * context,
                             const char * pinyins,
                             PinyinKeyVector pinyin_keys);
bool pinyin_parse_double(pinyin_context_t * context,
                         const char * onepinyin,
                         PinyinKey * onekey);
bool pinyin_parse_more_doubles(pinyin_context_t * context,
                               const char * onepinyin,
                               PinyinKeyVector pinyin_keys);

bool pinyin_get_candidates(pinyin_context_t * context,
                           size_t offset, TokenVector candidates);
bool pinyin_choose_candidate(pinyin_context_t * context,
                             size_t offset, phrase_token_t token);

bool pinyin_clear_constraint(pinyin_context_t * context,
                             size_t offset);
bool pinyin_clear_constraints(pinyin_context_t * context);

bool phrase_segment(pinyin_context_t * context, const char * sentence,
                    TokenVector tokens);
bool pinyin_translate_token(pinyin_context_t * context,
                            phrase_token_t token, char ** word);

bool pinyin_train(pinyin_context_t * context);
bool pinyin_save(pinyin_context_t * context);
bool pinyin_reset(pinyin_context_t * context);
