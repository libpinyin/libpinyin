#include "pinyin.h"

/* Place holder for combining static libraries in sub-directories. */

/* Note: will implement a glue layer for input method integration.
 * Refer to: fcitx-sunpinyin
 */

struct _pinyin_context_t{
    PinyinCustomSettings m_custom;

    BitmapPinyinValidator m_validator;
    PinyinDefaultParser * m_default_parser;
    PinyinShuangPinParser * m_shuang_pin_parser;

    PinyinLargeTable * m_pinyin_table;
    PhraseLargeTable * m_phrase_table;
    FacadePhraseIndex * m_phrase_index;
    Bigram * m_system_bigram;
    Bigram * m_user_bigram;

    PinyinLookup * m_pinyin_lookup;
    PhraseLookup * m_phrase_lookup;
    PinyinKeyVector m_pinyin_keys;
    MatchResults m_match_results;
    CandidateConstraints m_constraints;

    const char * m_system_dir;
    const char * m_user_dir;
};

pinyin_context_t * pinyin_init(const char * systemdir, const char * userdir);
void pinyin_fini(pinyin_context_t * context);

/* copy from custom to context->m_custom. */
bool pinyin_set_options(pinyin_context_t * context,
                        PinyinCustomSettings * custom){
    guint32 option = custom->to_value();
    context->m_custom.from_value(option);
    return true;
}

/* copy from pinyin_keys to m_pinyin_keys. */
bool pinyin_set_pinyin_keys(pinyin_context_t * context,
                            PinyinKeyVector pinyin_keys){
    g_array_set_size(context->m_pinyin_keys, 0);
    g_array_append_vals(context->m_pinyin_keys,
                        pinyin_keys->data, pinyin_keys->len);
    return true;
}


/* the returned sentence should be freed by g_free(). */
bool pinyin_get_guessed_sentence(pinyin_context_t * context,
                                char ** sentence);

bool pinyin_parse_full(pinyin_context_t * context,
                       const char * onepinyin,
                       PinyinKey * onekey){
    int pinyin_len = strlen(onepinyin);
    int parse_len = context->m_default_parser->parse_one_key
        ( context->m_validator, *onekey, onepinyin, pinyin_len);
    return pinyin_len == parse_len;
}

bool pinyin_parse_more_fulls(pinyin_context_t * context,
                             const char * pinyins,
                             PinyinKeyVector pinyin_keys){
    int pinyin_len = strlen(pinyins);
    PinyinKeyPosVector poses;
    poses = g_array_new(FALSE, FALSE, sizeof(PinyinKeyPos));

    int parse_len = context->m_default_parser->parse
        ( context->m_validator, pinyin_keys,
          poses, pinyins, pinyin_len);

    g_array_free(poses, TRUE);
    return pinyin_len == parse_len;
}

bool pinyin_parse_double(pinyin_context_t * context,
                         const char * onepinyin,
                         PinyinKey * onekey){
    int pinyin_len = strlen(onepinyin);
    int parse_len = context->m_shuang_pin_parser->parse_one_key
        ( context->m_validator, *onekey, onepinyin, pinyin_len);
    return pinyin_len == parse_len;
}

bool pinyin_parse_more_doubles(pinyin_context_t * context,
                               const char * pinyins,
                               PinyinKeyVector pinyin_keys){
    int pinyin_len = strlen(pinyins);
    PinyinKeyPosVector poses;
    poses = g_array_new(FALSE, FALSE, sizeof(PinyinKeyPos));

    int parse_len = context->m_shuang_pin_parser->parse
        ( context->m_validator, pinyin_keys,
          poses, pinyins, pinyin_len);

    g_array_free(poses, TRUE);
    return pinyin_len == parse_len;
}

bool pinyin_get_candidates(pinyin_context_t * context,
                           size_t offset, TokenVector tokens);
bool pinyin_choose_candidate(pinyin_context_t * context,
                             size_t offset, phrase_token_t token);
bool pinyin_clear_constraints(pinyin_context_t * context);

/* the returned word should be freed by g_free. */
bool pinyin_translate_token(pinyin_context_t * context,
                            phrase_token_t token, char ** word){
    PhraseItem item;
    utf16_t buffer[MAX_PHRASE_LENGTH];

    bool retval = context->m_phrase_index->get_phrase_item(token, item);
    item.get_phrase_string(buffer);
    guint8 length = item.get_phrase_length();
    *word = g_utf16_to_utf8(buffer, length, NULL, NULL, NULL);
    return retval;
}

bool pinyin_train(pinyin_context_t * context){
    bool retval = context->m_pinyin_lookup->train_result
        (context->m_pinyin_keys, context->m_constraints,
         context->m_match_results);
    return retval;
}

bool pinyin_save(pinyin_context_t * context);

bool pinyin_reset(pinyin_context_t * context);

/** TODO: to be implemented.
 *  bool pinyin_get_guessed_sentence_with_prefix(...);
 *  bool pinyin_get_candidates_with_prefix(...);
 *  For context-dependent order of the candidates list.
 */
