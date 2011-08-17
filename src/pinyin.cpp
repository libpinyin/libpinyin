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

    char * m_system_dir;
    char * m_user_dir;
};

pinyin_context_t * pinyin_init(const char * systemdir, const char * userdir){
    pinyin_context_t * context = new pinyin_context_t;

    context->m_system_dir = g_strdup(systemdir);
    context->m_user_dir = g_strdup(userdir);

    context->m_pinyin_table = new PinyinLargeTable(&(context->m_custom));
    MemoryChunk * chunk = new MemoryChunk;
    gchar * filename = g_build_filename
        (context->m_system_dir, "pinyin_index.bin");
    chunk->load(filename);
    context->m_pinyin_table->load(chunk);

    context->m_validator.initialize(context->m_pinyin_table);
    context->m_default_parser = new PinyinDefaultParser;
    context->m_shuang_pin_parser = new PinyinShuangPinParser;

    context->m_phrase_table = new PhraseLargeTable;
    chunk = new MemoryChunk;
    filename = g_build_filename(context->m_system_dir, "phrase_index.bin");
    chunk->load(filename);
    context->m_phrase_table->load(chunk);

    context->m_phrase_index = new FacadePhraseIndex;
    MemoryChunk * log = new MemoryChunk; chunk = new MemoryChunk;
    filename = g_build_filename(context->m_system_dir, "gb_char.bin");
    chunk->load(filename);
    context->m_phrase_index->load(1, chunk);
    filename = g_build_filename(context->m_user_dir, "gb_char.dbin");
    log->load(filename);
    context->m_phrase_index->merge(1, log);

    log = new MemoryChunk; chunk = new MemoryChunk;
    filename = g_build_filename(context->m_system_dir, "gbk_char.bin");
    chunk->load(filename);
    context->m_phrase_index->load(2, chunk);
    filename = g_build_filename(context->m_user_dir, "gbk_char.dbin");
    log->load(filename);
    context->m_phrase_index->merge(2, log);

    context->m_system_bigram = new Bigram;
    filename = g_build_filename(context->m_system_dir, "system.db");
    context->m_system_bigram->attach(filename, ATTACH_READONLY);
    context->m_user_bigram = new Bigram;
    filename = g_build_filename(context->m_user_dir, "user.db");
    context->m_user_bigram->attach(filename, ATTACH_CREATE|ATTACH_READWRITE);

    context->m_pinyin_lookup = new PinyinLookup
        ( &(context->m_custom), context->m_pinyin_table,
          context->m_phrase_index, context->m_system_bigram,
          context->m_user_bigram);

    context->m_phrase_lookup = new PhraseLookup
        (context->m_phrase_table, context->m_phrase_index,
         context->m_system_bigram, context->m_user_bigram);

    context->m_pinyin_keys = g_array_new(FALSE, FALSE, sizeof(PinyinKey));
    context->m_match_results = g_array_new
        (FALSE, FALSE, sizeof(phrase_token_t));
    context->m_constraints = g_array_new
        (FALSE, FALSE, sizeof(lookup_constraint_t));

    return context;
}

void pinyin_fini(pinyin_context_t * context){
    delete context->m_default_parser;
    delete context->m_shuang_pin_parser;
    delete context->m_pinyin_table;
    delete context->m_phrase_table;
    delete context->m_phrase_index;
    delete context->m_system_bigram;
    delete context->m_user_bigram;
    delete context->m_pinyin_lookup;
    delete context->m_phrase_lookup;

    g_array_free(context->m_pinyin_keys, true);
    g_array_free(context->m_match_results, true);
    g_array_free(context->m_constraints, true);

    g_free(context->m_system_dir);
    g_free(context->m_user_dir);

    delete context;
}

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
    size_t key_len = context->m_pinyin_keys->len;
    g_array_set_size(context->m_pinyin_keys, 0);
    g_array_append_vals(context->m_pinyin_keys,
                        pinyin_keys->data, pinyin_keys->len);

    g_array_set_size(context->m_constraints, context->m_pinyin_keys->len);
    for (size_t i = key_len; i < context->m_pinyin_keys->len; ++i ) {
        lookup_constraint_t * constraint = 
            &g_array_index(context->m_constraints, lookup_constraint_t, i);
        constraint->m_type = NO_CONSTRAINT;
    }

    context->m_pinyin_lookup->validate_constraint
        (context->m_constraints, context->m_pinyin_keys);

    return true;
}


/* the returned sentence should be freed by g_free(). */
bool pinyin_get_guessed_sentence(pinyin_context_t * context,
                                 char ** sentence){
    bool retval = context->m_pinyin_lookup->get_best_match
        (context->m_pinyin_keys, context->m_constraints,
         context->m_match_results);

    retval = context->m_pinyin_lookup->convert_to_utf8
        (context->m_match_results, *sentence) && retval;

    return retval;
}

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
                             size_t offset, phrase_token_t token){
    bool retval = context->m_pinyin_lookup->add_constraint
        (context->m_constraints, offset, token);

    retval = context->m_pinyin_lookup->validate_constraint
        (context->m_constraints, context->m_pinyin_keys) && retval;

    return retval;
}

bool pinyin_clear_constraints(pinyin_context_t * context){
    bool retval = true;
    CandidateConstraints & constraints = context->m_constraints;

    for ( size_t i = 0; i < constraints->len; ++i ) {
        retval = context->m_pinyin_lookup->clear_constraint
            (constraints, i) && retval;
    }

    return retval;
}

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

bool pinyin_save(pinyin_context_t * context){
    MemoryChunk * oldchunk = new MemoryChunk;
    MemoryChunk * newlog = new MemoryChunk;

    gchar * filename = g_build_filename(context->m_system_dir, "gb_char.bin");
    oldchunk->load(filename);
    context->m_phrase_index->diff(1, oldchunk, newlog);
    filename = g_build_filename(context->m_user_dir, "gb_char.dbin");
    newlog->save(filename);
    delete newlog;

    oldchunk = new MemoryChunk; newlog = new MemoryChunk;
    filename = g_build_filename(context->m_system_dir, "gbk_char.bin");
    context->m_phrase_index->diff(2, oldchunk, newlog);
    filename = g_build_filename(context->m_user_dir, "gbk_char.dbin");
    newlog->save(filename);
    delete newlog;

    return true;
}

bool pinyin_reset(pinyin_context_t * context){
    g_array_set_size(context->m_pinyin_keys, 0);
    g_array_set_size(context->m_match_results, 0);
    g_array_set_size(context->m_constraints, 0);
    return true;
}

/** TODO: to be implemented.
 *  bool pinyin_get_guessed_sentence_with_prefix(...);
 *  bool pinyin_get_candidates_with_prefix(...);
 *  For context-dependent order of the candidates list.
 */
