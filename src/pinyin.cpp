/* 
 *  libpinyin
 *  Library to deal with pinyin.
 *  
 *  Copyright (C) 2011 Peng Wu <alexepico@gmail.com>
 *  
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 * 
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *  
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */




#include "pinyin.h"
#include "pinyin_internal.h"

/* a glue layer for input method integration. */

struct _pinyin_context_t{
    PinyinCustomSettings m_custom;

    BitmapPinyinValidator m_validator;
    PinyinDefaultParser * m_default_parser;
    PinyinShuangPinParser * m_shuang_pin_parser;
    PinyinZhuYinParser * m_chewing_parser;

    PinyinLargeTable * m_pinyin_table;
    PhraseLargeTable * m_phrase_table;
    FacadePhraseIndex * m_phrase_index;
    Bigram * m_system_bigram;
    Bigram * m_user_bigram;

    PinyinLookup * m_pinyin_lookup;
    PhraseLookup * m_phrase_lookup;

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
        (context->m_system_dir, "pinyin_index.bin", NULL);
    if (!chunk->load(filename)) {
        fprintf(stderr, "open %s failed!\n", filename);
        return NULL;
    }
    context->m_pinyin_table->load(chunk);

    context->m_validator.initialize(context->m_pinyin_table);
    context->m_default_parser = new PinyinDefaultParser;
    context->m_shuang_pin_parser = new PinyinShuangPinParser;
    context->m_chewing_parser = new PinyinZhuYinParser;

    context->m_phrase_table = new PhraseLargeTable;
    chunk = new MemoryChunk;
    filename = g_build_filename(context->m_system_dir, "phrase_index.bin", NULL);
    if (!chunk->load(filename)) {
        fprintf(stderr, "open %s failed!\n", filename);
        return NULL;
    }
    context->m_phrase_table->load(chunk);

    context->m_phrase_index = new FacadePhraseIndex;
    MemoryChunk * log = new MemoryChunk; chunk = new MemoryChunk;
    filename = g_build_filename(context->m_system_dir, "gb_char.bin", NULL);
    if (!chunk->load(filename)) {
        fprintf(stderr, "open %s failed!\n", filename);
        return NULL;
    }
    context->m_phrase_index->load(1, chunk);
    filename = g_build_filename(context->m_user_dir, "gb_char.dbin", NULL);
    log->load(filename);
    context->m_phrase_index->merge(1, log);

    log = new MemoryChunk; chunk = new MemoryChunk;
    filename = g_build_filename(context->m_system_dir, "gbk_char.bin", NULL);
    if (!chunk->load(filename)) {
        fprintf(stderr, "open %s failed!\n", filename);
        return NULL;
    }
    context->m_phrase_index->load(2, chunk);
    filename = g_build_filename(context->m_user_dir, "gbk_char.dbin", NULL);
    log->load(filename);
    context->m_phrase_index->merge(2, log);

    context->m_system_bigram = new Bigram;
    filename = g_build_filename(context->m_system_dir, "bigram.db", NULL);
    context->m_system_bigram->attach(filename, ATTACH_READONLY);
    context->m_user_bigram = new Bigram;
    filename = g_build_filename(context->m_user_dir, "user.db", NULL);
    context->m_user_bigram->load_db(filename);

    context->m_pinyin_lookup = new PinyinLookup
        ( &(context->m_custom), context->m_pinyin_table,
          context->m_phrase_index, context->m_system_bigram,
          context->m_user_bigram);

    context->m_phrase_lookup = new PhraseLookup
        (context->m_phrase_table, context->m_phrase_index,
         context->m_system_bigram, context->m_user_bigram);

    return context;
}

bool pinyin_save(pinyin_context_t * context){
    if (!context->m_user_dir)
        return FALSE;

    MemoryChunk * oldchunk = new MemoryChunk;
    MemoryChunk * newlog = new MemoryChunk;

    gchar * filename = g_build_filename(context->m_system_dir,
                                        "gb_char.bin", NULL);
    oldchunk->load(filename);
    context->m_phrase_index->diff(1, oldchunk, newlog);
    filename = g_build_filename(context->m_user_dir,
                                "gb_char.dbin", NULL);
    newlog->save(filename);
    delete newlog;

    oldchunk = new MemoryChunk; newlog = new MemoryChunk;
    filename = g_build_filename(context->m_system_dir,
                                "gbk_char.bin", NULL);
    oldchunk->load(filename);
    context->m_phrase_index->diff(2, oldchunk, newlog);
    filename = g_build_filename(context->m_user_dir,
                                "gbk_char.dbin", NULL);
    newlog->save(filename);
    delete newlog;

    filename = g_build_filename(context->m_user_dir, "user.db", NULL);
    context->m_user_bigram->save_db(filename);

    return true;
}

bool pinyin_set_double_pinyin_scheme(pinyin_context_t * context,
                                     PinyinShuangPinScheme scheme){
    context->m_shuang_pin_parser->set_scheme(scheme);
    return true;
}

bool pinyin_set_chewing_scheme(pinyin_context_t * context,
                               PinyinZhuYinScheme scheme){
    context->m_chewing_parser->set_scheme(scheme);
    return true;
}


void pinyin_fini(pinyin_context_t * context){
    delete context->m_default_parser;
    delete context->m_shuang_pin_parser;
    delete context->m_chewing_parser;
    delete context->m_pinyin_table;
    delete context->m_phrase_table;
    delete context->m_phrase_index;
    delete context->m_system_bigram;
    delete context->m_user_bigram;
    delete context->m_pinyin_lookup;
    delete context->m_phrase_lookup;

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


pinyin_instance_t * pinyin_alloc_instance(pinyin_context_t * context){
    pinyin_instance_t * instance = new pinyin_instance_t;
    instance->m_context = context;

    instance->m_pinyin_keys = g_array_new(FALSE, FALSE, sizeof(PinyinKey));
    instance->m_pinyin_poses = g_array_new(FALSE, FALSE, sizeof(PinyinKeyPos));
    instance->m_constraints = g_array_new
        (FALSE, FALSE, sizeof(lookup_constraint_t));
    instance->m_match_results =
        g_array_new(FALSE, FALSE, sizeof(phrase_token_t));

    return instance;
}

void pinyin_free_instance(pinyin_instance_t * instance){
    g_array_free(instance->m_pinyin_keys, TRUE);
    g_array_free(instance->m_pinyin_poses, TRUE);
    g_array_free(instance->m_constraints, TRUE);
    g_array_free(instance->m_match_results, TRUE);

    delete instance;
}


static bool pinyin_update_constraints(pinyin_instance_t * instance){
    pinyin_context_t * & context = instance->m_context;
    PinyinKeyVector & pinyin_keys = instance->m_pinyin_keys;
    CandidateConstraints & constraints = instance->m_constraints;

    size_t key_len = constraints->len;
    g_array_set_size(constraints, pinyin_keys->len);
    for (size_t i = key_len; i < pinyin_keys->len; ++i ) {
        lookup_constraint_t * constraint =
            &g_array_index(constraints, lookup_constraint_t, i);
        constraint->m_type = NO_CONSTRAINT;
    }

    context->m_pinyin_lookup->validate_constraint
        (constraints, pinyin_keys);

    return true;
}


bool pinyin_guess_sentence(pinyin_instance_t * instance){
    pinyin_context_t * & context = instance->m_context;

    pinyin_update_constraints(instance);
    bool retval = context->m_pinyin_lookup->get_best_match
        (instance->m_pinyin_keys,
         instance->m_constraints,
         instance->m_match_results);

    return retval;
}

bool pinyin_phrase_segment(pinyin_instance_t * instance,
                           const char * sentence){
    pinyin_context_t * & context = instance->m_context;

    const glong num_of_chars = g_utf8_strlen(sentence, -1);
    glong utf16_len = 0;
    utf16_t * utf16 = g_utf8_to_utf16(sentence, -1, NULL, &utf16_len, NULL);

    g_return_val_if_fail(num_of_chars == utf16_len, FALSE);

    bool retval = context->m_phrase_lookup->get_best_match
        (utf16_len, utf16, instance->m_match_results);

    g_free(utf16);
    return retval;
}

/* the returned sentence should be freed by g_free(). */
bool pinyin_get_sentence(pinyin_instance_t * instance,
                         char ** sentence){
    pinyin_context_t * & context = instance->m_context;

    bool retval = pinyin::convert_to_utf8
        (context->m_phrase_index, instance->m_match_results,
         NULL, *sentence);

    return retval;
}

bool pinyin_parse_full_pinyin(pinyin_instance_t * instance,
                              const char * onepinyin,
                              PinyinKey * onekey){
    pinyin_context_t * & context = instance->m_context;

    int pinyin_len = strlen(onepinyin);
    int parse_len = context->m_default_parser->parse_one_key
        ( context->m_validator, *onekey, onepinyin, pinyin_len);
    return pinyin_len == parse_len;
}

size_t pinyin_parse_more_full_pinyins(pinyin_instance_t * instance,
                                      const char * pinyins){
    pinyin_context_t * & context = instance->m_context;
    int pinyin_len = strlen(pinyins);

    int parse_len = context->m_default_parser->parse
        ( context->m_validator, instance->m_pinyin_keys,
          instance->m_pinyin_poses, pinyins, pinyin_len);

    return parse_len;
}

bool pinyin_parse_double_pinyin(pinyin_instance_t * instance,
                                const char * onepinyin,
                                PinyinKey * onekey){
    pinyin_context_t * & context = instance->m_context;

    int pinyin_len = strlen(onepinyin);
    int parse_len = context->m_shuang_pin_parser->parse_one_key
        ( context->m_validator, *onekey, onepinyin, pinyin_len);
    return pinyin_len == parse_len;
}

size_t pinyin_parse_more_double_pinyins(pinyin_instance_t * instance,
                                        const char * pinyins){
    pinyin_context_t * & context = instance->m_context;
    int pinyin_len = strlen(pinyins);

    int parse_len = context->m_shuang_pin_parser->parse
        ( context->m_validator, instance->m_pinyin_keys,
          instance->m_pinyin_poses, pinyins, pinyin_len);

    return parse_len;
}

bool pinyin_parse_chewing(pinyin_instance_t * instance,
                          const char * onechewing,
                          PinyinKey * onekey){
    pinyin_context_t * & context = instance->m_context;

    int chewing_len = strlen(onechewing);
    int parse_len = context->m_chewing_parser->parse_one_key
        ( context->m_validator, *onekey, onechewing, chewing_len );
    return chewing_len == parse_len;
}

size_t pinyin_parse_more_chewings(pinyin_instance_t * instance,
                                  const char * chewings){
    pinyin_context_t * & context = instance->m_context;
    int chewing_len = strlen(chewings);

    int parse_len = context->m_chewing_parser->parse
        ( context->m_validator, instance->m_pinyin_keys,
          instance->m_pinyin_poses, chewings, chewing_len);

    return parse_len;
}

/* internal definition */
typedef struct {
    pinyin_context_t * m_context;
    PinyinKey * m_pinyin_keys;
} compare_context;

static gint compare_token( gconstpointer lhs, gconstpointer rhs){
    phrase_token_t token_lhs = *((phrase_token_t *)lhs);
    phrase_token_t token_rhs = *((phrase_token_t *)rhs);
    return token_lhs - token_rhs;
}

static gint compare_token_with_unigram_freq(gconstpointer lhs,
                                            gconstpointer rhs,
                                            gpointer user_data){
    phrase_token_t token_lhs = *((phrase_token_t *)lhs);
    phrase_token_t token_rhs = *((phrase_token_t *)rhs);
    compare_context * context = (compare_context *)user_data;
    FacadePhraseIndex * phrase_index = context->m_context->m_phrase_index;
    PinyinCustomSettings & custom = context->m_context->m_custom;
    PinyinKey * pinyin_keys = context->m_pinyin_keys;

    PhraseItem item;
    phrase_index->get_phrase_item(token_lhs, item);
    guint32 freq_lhs = item.get_unigram_frequency() *
        item.get_pinyin_possibility(custom, pinyin_keys) * 256;
    phrase_index->get_phrase_item(token_rhs, item);
    guint32 freq_rhs = item.get_unigram_frequency() *
        item.get_pinyin_possibility(custom, pinyin_keys) * 256;

    return -(freq_lhs - freq_rhs); /* in descendant order */
}

bool pinyin_get_candidates(pinyin_instance_t * instance,
                           size_t offset,
                           TokenVector candidates){
    pinyin_context_t * & context = instance->m_context;
    PinyinKeyVector & pinyin_keys = instance->m_pinyin_keys;
    g_array_set_size(candidates, 0);

    PinyinKey * keys = &g_array_index
        (pinyin_keys, PinyinKey, offset);
    size_t pinyin_len = pinyin_keys->len - offset;

    compare_context comp_context;
    comp_context.m_context = context;
    comp_context.m_pinyin_keys = keys;

    PhraseIndexRanges ranges;
    memset(ranges, 0, sizeof(ranges));

    guint8 min_index, max_index;
    assert( ERROR_OK == context->m_phrase_index->
            get_sub_phrase_range(min_index, max_index));

    for (size_t m = min_index; m <= max_index; ++m) {
        ranges[m] = g_array_new(FALSE, FALSE, sizeof(PhraseIndexRange));
    }

    GArray * tokens = g_array_new(FALSE, FALSE, sizeof(phrase_token_t));

    for (ssize_t i = pinyin_len; i >= 1; --i) {
        g_array_set_size(tokens, 0);

        /* clear ranges. */
        for ( size_t m = min_index; m <= max_index; ++m ) {
            g_array_set_size(ranges[m], 0);
        }

        /* do pinyin search. */
        int retval = context->m_pinyin_table->search
            (i, keys, ranges);

        if ( !(retval & SEARCH_OK) )
            continue;

        /* reduce to a single GArray. */
        for (size_t m = min_index; m <= max_index; ++m) {
            for (size_t n = 0; n < ranges[m]->len; ++n) {
                PhraseIndexRange * range =
                    &g_array_index(ranges[m], PhraseIndexRange, n);
                for (size_t k = range->m_range_begin;
                     k < range->m_range_end; ++k) {
                    g_array_append_val(tokens, k);
                }
            }
        }

        g_array_sort(tokens, compare_token);
        /* remove the duplicated items. */
        phrase_token_t last_token = null_token;
        for ( size_t n = 0; n < tokens->len; ++n) {
            phrase_token_t token = g_array_index(tokens, phrase_token_t, n);
            if ( last_token == token ){
                g_array_remove_index(tokens, n);
                n--;
            }
            last_token = token;
        }

        /* sort the candidates of the same length by uni-gram freqs. */
        g_array_sort_with_data(tokens, compare_token_with_unigram_freq,
                               &comp_context);

        /* copy out candidates. */
        g_array_append_vals(candidates, tokens->data, tokens->len);

        if ( !(retval & SEARCH_CONTINUED) )
            break;
    }

    g_array_free(tokens, TRUE);
    for (size_t m = min_index; m <= max_index; ++m) {
        g_array_free(ranges[m], TRUE);
    }

    return true;
}

guint8 pinyin_choose_candidate(pinyin_instance_t * instance,
                               size_t offset,
                               phrase_token_t token){
    pinyin_context_t * & context = instance->m_context;

    guint8 len = context->m_pinyin_lookup->add_constraint
        (instance->m_constraints, offset, token);

    bool retval = context->m_pinyin_lookup->validate_constraint
        (instance->m_constraints, instance->m_pinyin_keys) && len;

    return len;
}

bool pinyin_clear_constraint(pinyin_instance_t * instance,
                             size_t offset){
    pinyin_context_t * & context = instance->m_context;

    bool retval = context->m_pinyin_lookup->clear_constraint
        (instance->m_constraints, offset);

    return retval;
}

bool pinyin_clear_constraints(pinyin_instance_t * instance){
    pinyin_context_t * & context = instance->m_context;
    bool retval = true;

    for ( size_t i = 0; i < instance->m_constraints->len; ++i ) {
        retval = context->m_pinyin_lookup->clear_constraint
            (instance->m_constraints, i) && retval;
    }

    return retval;
}

/* the returned word should be freed by g_free. */
bool pinyin_translate_token(pinyin_instance_t * instance,
                            phrase_token_t token, char ** word){
    pinyin_context_t * & context = instance->m_context;
    PhraseItem item;
    utf16_t buffer[MAX_PHRASE_LENGTH];

    bool retval = context->m_phrase_index->get_phrase_item(token, item);
    item.get_phrase_string(buffer);
    guint8 length = item.get_phrase_length();
    *word = g_utf16_to_utf8(buffer, length, NULL, NULL, NULL);
    return retval;
}

bool pinyin_train(pinyin_instance_t * instance){
    if (!instance->m_context->m_user_dir)
        return FALSE;

    pinyin_context_t * & context = instance->m_context;

    bool retval = context->m_pinyin_lookup->train_result
        (instance->m_pinyin_keys, instance->m_constraints,
         instance->m_match_results);

    return retval;
}

bool pinyin_reset(pinyin_instance_t * instance){
    g_array_set_size(instance->m_pinyin_keys, 0);
    g_array_set_size(instance->m_pinyin_poses, 0);
    g_array_set_size(instance->m_constraints, 0);
    g_array_set_size(instance->m_match_results, 0);

    return true;
}

/**
 *  TODO: to be implemented.
 *    Note: prefix is the text before the pre-edit string.
 *  bool pinyin_get_guessed_sentence_with_prefix(...);
 *  bool pinyin_get_candidates_with_prefix(...);
 *  For context-dependent order of the candidates list.
 */
