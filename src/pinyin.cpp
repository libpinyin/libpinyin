#include "pinyin.h"

/* Place holder for combining static libraries in sub-directories. */

/* Note: will implement a glue layer for input method integration.
 * Refer to: fcitx-sunpinyin
 */

struct _pinyin_context_t{
    PinyinDefaultParser * m_default_parser;
    PinyinShuangPinParser * m_shuang_pin_parser;
    PinyinCustomSettings * m_custom;

    PinyinLargeTable * m_pinyin_table;
    PhraseLargeTable * m_phrase_table;
    FacadePhraseIndex * m_phrase_index;
    Bigram * m_system_bigram;
    Bigram * m_user_bigram;

    PinyinLookup * m_pinyin_lookup;
    MatchResults m_match_results;
    CandidateConstraints m_constraints;

    BitmapPinyinValidator * m_validator;
    const char * m_system_dir;
    const char * m_user_dir;
};

pinyin_context_t * pinyin_init(const char * systemdir, const char * userdir);
void pinyin_fini(pinyin_context_t * context);

/* copy from custom to context->m_custom. */
bool pinyin_set_options(pinyin_context_t * context,
                        PinyinCustomSettings * custom);
bool pinyin_set_pinyin_keys(pinyin_context_t * context,
                            PinyinKeyVector pinyin_keys);


/* the returned sentence should be freed by g_free(). */
bool pinyin_get_guessed_sentence(pinyin_context_t * context,
                                char ** sentence);

bool pinyin_parse_one(pinyin_context_t * context,
                      const char * onepinyin,
                      PinyinKey * onekey);
bool pinyin_parse_more(pinyin_context_t * context,
                       const char * pinyins,
                       PinyinKeyVector pinyin_keys);

bool pinyin_get_candidates(pinyin_context_t * context,
                           size_t offset, TokenVector tokens);
bool pinyin_choose_candidate(pinyin_context_t * context,
                             size_t offset, phrase_token_t * token);

/* the returned word should be freed by g_free. */
bool pinyin_translate_token(pinyin_context_t * context,
                            phrase_token_t token, char ** word);

bool pinyin_train(pinyin_context_t * context);
bool pinyin_save(pinyin_context_t * context);

bool pinyin_reset(pinyin_context_t * context);

/** TODO: to be implemented.
 *  bool pinyin_get_guessed_sentence_with_prefix(...);
 *  bool pinyin_get_candidates_with_prefix(...);
 *  For context-dependent order of the candidates list.
 */
