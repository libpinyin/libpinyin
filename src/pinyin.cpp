/* 
 *  libpinyin
 *  Library to deal with pinyin.
 *  
 *  Copyright (C) 2011 Peng Wu <alexepico@gmail.com>
 *  
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */


#include "pinyin.h"
#include <stdio.h>
#include <float.h>
#include <unistd.h>
#include <glib/gstdio.h>
#include "pinyin_internal.h"


using namespace pinyin;

/* reduce bigram frequency affects on candidates sorting */
#define BIGRAM_FREQUENCY_DISCOUNT 0.1f

/* a glue layer for input method integration. */

typedef GArray * CandidateVector; /* GArray of lookup_candidate_t */

struct _pinyin_context_t{
    pinyin_option_t m_options;

    /* input parsers. */
    FullPinyinParser2 * m_full_pinyin_parser;
    DoublePinyinParser2 * m_double_pinyin_parser;
    ZhuyinParser2 * m_chewing_parser;

    /* default tables. */
    FacadeChewingTable2 * m_pinyin_table;
    FacadePhraseTable3 * m_phrase_table;
    FacadePhraseIndex * m_phrase_index;
    Bigram * m_system_bigram;
    Bigram * m_user_bigram;

    /* lookups. */
    PhoneticLookup<2, 3> * m_pinyin_lookup;
    PhraseLookup * m_phrase_lookup;

    /* addon tables. */
    FacadeChewingTable2 * m_addon_pinyin_table;
    FacadePhraseTable3 * m_addon_phrase_table;
    FacadePhraseIndex * m_addon_phrase_index;

    char * m_system_dir;
    char * m_user_dir;
    bool m_modified;

    SystemTableInfo2 m_system_table_info;
};

struct _pinyin_instance_t{
    /* pointer of pinyin_context_t. */
    pinyin_context_t * m_context;

    /* the tokens of phrases before the user input. */
    TokenVector m_prefixes;

    /* cached parsed pinyin keys. */
    PhoneticKeyMatrix m_matrix;
    size_t m_parsed_len;

    /* cached pinyin lookup variables. */
    ForwardPhoneticConstraints * m_constraints;
    NBestMatchResults m_nbest_results;
    TokenVector m_phrase_result;
    CandidateVector m_candidates;
};

struct _lookup_candidate_t{
    lookup_candidate_type_t m_candidate_type;
    gchar * m_phrase_string;
    phrase_token_t m_token;
    guint8 m_phrase_length;
    gint8 m_nbest_index; /* only for NBEST_MATCH_CANDIDATE. */
    guint16 m_begin; /* must contain the preceding "'" character. */
    guint16 m_end; /* must not contain the following "'" character. */
    guint32 m_freq; /* the amplifed gfloat numerical value. */

public:
    _lookup_candidate_t() {
        m_candidate_type = NORMAL_CANDIDATE;
        m_phrase_string = NULL;
        m_token = null_token;
        m_phrase_length = 0;
        m_nbest_index = -1;
        m_begin = 0; m_end = 0;
        m_freq = 0;
    }
};

struct _import_iterator_t{
    pinyin_context_t * m_context;
    guint8 m_phrase_index;
};

struct _export_iterator_t{
    pinyin_context_t * m_context;
    guint8 m_phrase_index;
    /* null token means no next item. */
    phrase_token_t m_next_token;
    guint8 m_next_pronunciation;
};

static bool _clean_user_files(const char * user_dir,
                              const pinyin_table_info_t * phrase_files){
    /* clean up files, if version mis-matches. */
    for (size_t i = 1; i < PHRASE_INDEX_LIBRARY_COUNT; ++i) {
        const pinyin_table_info_t * table_info = phrase_files + i;

        if (NOT_USED == table_info->m_file_type)
            continue;

        if (NULL == table_info->m_user_filename)
            continue;

        const char * userfilename = table_info->m_user_filename;

        /* remove dbin file. */
        gchar * filename = g_build_filename(user_dir, userfilename, NULL);
        unlink(filename);
        g_free(filename);
    }

    return true;
}

static bool check_format(pinyin_context_t * context){
    const char * user_dir = context->m_user_dir;

    UserTableInfo user_table_info;
    gchar * filename = g_build_filename
        (user_dir, USER_TABLE_INFO, NULL);
    user_table_info.load(filename);
    g_free(filename);

    bool exists = user_table_info.is_conform
        (&context->m_system_table_info);

    if (exists)
        return exists;

    const pinyin_table_info_t * phrase_files = NULL;

    phrase_files = context->m_system_table_info.get_default_tables();
    _clean_user_files(user_dir, phrase_files);

    phrase_files = context->m_system_table_info.get_addon_tables();
    _clean_user_files(user_dir, phrase_files);

    filename = g_build_filename
        (user_dir, USER_PINYIN_INDEX, NULL);
    unlink(filename);
    g_free(filename);

    filename = g_build_filename
        (user_dir, USER_PHRASE_INDEX, NULL);
    unlink(filename);
    g_free(filename);

    filename = g_build_filename
        (user_dir, USER_BIGRAM, NULL);
    unlink(filename);
    g_free(filename);

    return exists;
}

static bool mark_version(pinyin_context_t * context){
    const char * userdir = context->m_user_dir;

    UserTableInfo user_table_info;
    user_table_info.make_conform(&context->m_system_table_info);

    gchar * filename = g_build_filename
        (userdir, USER_TABLE_INFO, NULL);
    bool retval = user_table_info.save(filename);
    g_free(filename);

    return retval;
}

static bool _load_phrase_library (const char * system_dir,
                                  const char * user_dir,
                                  FacadePhraseIndex * phrase_index,
                                  const pinyin_table_info_t * table_info){
    /* check whether the sub phrase index is already loaded. */
    PhraseIndexRange range;
    guint8 index = table_info->m_dict_index;

    int retval = phrase_index->get_range(index, range);
    if (ERROR_OK == retval)
        return false;

    if (SYSTEM_FILE == table_info->m_file_type) {
        /* system phrase library */
        MemoryChunk * chunk = new MemoryChunk;

        const char * systemfilename = table_info->m_system_filename;
        /* check bin file in system dir. */
        gchar * chunkfilename = g_build_filename(system_dir,
                                                 systemfilename, NULL);
#ifdef LIBPINYIN_USE_MMAP
        if (!chunk->mmap(chunkfilename))
            fprintf(stderr, "mmap %s failed!\n", chunkfilename);
#else
        if (!chunk->load(chunkfilename))
            fprintf(stderr, "open %s failed!\n", chunkfilename);
#endif

        g_free(chunkfilename);

        phrase_index->load(index, chunk);

        const char * userfilename = table_info->m_user_filename;

        chunkfilename = g_build_filename(user_dir,
                                         userfilename, NULL);

        MemoryChunk * log = new MemoryChunk;
        log->load(chunkfilename);
        g_free(chunkfilename);

        /* merge the chunk log. */
        phrase_index->merge(index, log);
        return true;
    }

    if (DICTIONARY == table_info->m_file_type) {
        /* addon dictionary. */
        MemoryChunk * chunk = new MemoryChunk;

        const char * systemfilename = table_info->m_system_filename;
        /* check bin file in system dir. */
        gchar * chunkfilename = g_build_filename(system_dir,
                                                 systemfilename, NULL);
#ifdef LIBPINYIN_USE_MMAP
        if (!chunk->mmap(chunkfilename))
            fprintf(stderr, "mmap %s failed!\n", chunkfilename);
#else
        if (!chunk->load(chunkfilename))
            fprintf(stderr, "open %s failed!\n", chunkfilename);
#endif

        g_free(chunkfilename);

        phrase_index->load(index, chunk);

        return true;
    }

    if (USER_FILE == table_info->m_file_type) {
        /* user phrase library */
        MemoryChunk * chunk = new MemoryChunk;
        const char * userfilename = table_info->m_user_filename;

        gchar * chunkfilename = g_build_filename(user_dir,
                                                 userfilename, NULL);

        /* check bin file exists. if not, create a new one. */
        if (chunk->load(chunkfilename)) {
            phrase_index->load(index, chunk);
        } else {
            delete chunk;
            phrase_index->create_sub_phrase(index);
        }

        g_free(chunkfilename);
        return true;
    }

    return false;
}

pinyin_context_t * pinyin_init(const char * systemdir, const char * userdir){
    pinyin_context_t * context = new pinyin_context_t;

    context->m_options = USE_TONE;

    context->m_system_dir = g_strdup(systemdir);
    context->m_user_dir = g_strdup(userdir);
    context->m_modified = false;

    gchar * filename = g_build_filename
        (context->m_system_dir, SYSTEM_TABLE_INFO, NULL);
    if (!context->m_system_table_info.load(filename)) {
        fprintf(stderr, "load %s failed!\n", filename);
        return NULL;
    }
    g_free(filename);


    check_format(context);

    context->m_full_pinyin_parser = new FullPinyinParser2;
    context->m_double_pinyin_parser = new DoublePinyinParser2;
    context->m_chewing_parser = new ZhuyinSimpleParser2;

    /* load chewing table. */
    context->m_pinyin_table = new FacadeChewingTable2;

    gchar * system_filename = g_build_filename
        (context->m_system_dir, SYSTEM_PINYIN_INDEX, NULL);
    gchar * user_filename = g_build_filename
        (context->m_user_dir, USER_PINYIN_INDEX, NULL);
    context->m_pinyin_table->load(system_filename, user_filename);
    g_free(user_filename);
    g_free(system_filename);


    /* load phrase table */
    context->m_phrase_table = new FacadePhraseTable3;

    system_filename = g_build_filename
        (context->m_system_dir, SYSTEM_PHRASE_INDEX, NULL);
    user_filename = g_build_filename
        (context->m_user_dir, USER_PHRASE_INDEX, NULL);
    context->m_phrase_table->load(system_filename, user_filename);
    g_free(user_filename);
    g_free(system_filename);


    context->m_phrase_index = new FacadePhraseIndex;

    /* load all default tables. */
    for (size_t i = 0; i < PHRASE_INDEX_LIBRARY_COUNT; ++i){
        const pinyin_table_info_t * phrase_files =
            context->m_system_table_info.get_default_tables();

        const pinyin_table_info_t * table_info =
            phrase_files + i;

        if (NOT_USED == table_info->m_file_type)
            continue;

        /* addon dictionary should not in default tables. */
        assert(DICTIONARY != table_info->m_file_type);

        _load_phrase_library(context->m_system_dir, context->m_user_dir,
                             context->m_phrase_index, table_info);
    }

    context->m_system_bigram = new Bigram;
    filename = g_build_filename(context->m_system_dir, SYSTEM_BIGRAM, NULL);
    context->m_system_bigram->attach(filename, ATTACH_READONLY);
    g_free(filename);

    context->m_user_bigram = new Bigram;
    filename = g_build_filename(context->m_user_dir, USER_BIGRAM, NULL);
    context->m_user_bigram->load_db(filename);
    g_free(filename);

    gfloat lambda = context->m_system_table_info.get_lambda();

    context->m_pinyin_lookup = new PhoneticLookup<2, 3>
        (lambda,
         context->m_pinyin_table, context->m_phrase_index,
         context->m_system_bigram, context->m_user_bigram);

    context->m_phrase_lookup = new PhraseLookup
        (lambda,
         context->m_phrase_table, context->m_phrase_index,
         context->m_system_bigram, context->m_user_bigram);

    /* load addon chewing table. */
    context->m_addon_pinyin_table = new FacadeChewingTable2;

    system_filename = g_build_filename
        (context->m_system_dir, ADDON_SYSTEM_PINYIN_INDEX, NULL);
    context->m_addon_pinyin_table->load(system_filename, NULL);
    g_free(system_filename);

    /* load addon phrase table */
    context->m_addon_phrase_table = new FacadePhraseTable3;

    system_filename = g_build_filename
        (context->m_system_dir, ADDON_SYSTEM_PHRASE_INDEX, NULL);
    context->m_addon_phrase_table->load(system_filename, NULL);
    g_free(system_filename);

    context->m_addon_phrase_index = new FacadePhraseIndex;

    /* don't load addon phrase libraries. */

    return context;
}

bool pinyin_load_phrase_library(pinyin_context_t * context,
                                guint8 index){
    if (!(index < PHRASE_INDEX_LIBRARY_COUNT))
        return false;

    const pinyin_table_info_t * phrase_files =
        context->m_system_table_info.get_default_tables();
    FacadePhraseIndex * phrase_index = context->m_phrase_index;
    const pinyin_table_info_t * table_info = phrase_files + index;

    /* Only SYSTEM_FILE or USER_FILE is allowed here. */
    assert(SYSTEM_FILE == table_info->m_file_type
           || USER_FILE == table_info->m_file_type);

    return _load_phrase_library(context->m_system_dir, context->m_user_dir,
                                phrase_index, table_info);
}

bool pinyin_unload_phrase_library(pinyin_context_t * context,
                                  guint8 index){
    assert(index < PHRASE_INDEX_LIBRARY_COUNT);

    /* default table. */
    /* only GBK table can be unloaded. */
    if (GBK_DICTIONARY != index)
        return false;

    context->m_phrase_index->unload(index);
    return true;
}

bool pinyin_load_addon_phrase_library(pinyin_context_t * context,
                                      guint8 index){
    if (!(index < PHRASE_INDEX_LIBRARY_COUNT))
        return false;

    const pinyin_table_info_t * phrase_files =
        context->m_system_table_info.get_addon_tables();
    FacadePhraseIndex * phrase_index = context->m_addon_phrase_index;
    const pinyin_table_info_t * table_info = phrase_files + index;

    if (NOT_USED == table_info->m_file_type)
        return false;

    /* Only DICTIONARY is allowed here. */
    assert(DICTIONARY == table_info->m_file_type);

    return _load_phrase_library(context->m_system_dir, context->m_user_dir,
                                phrase_index, table_info);
}

bool pinyin_unload_addon_phrase_library(pinyin_context_t * context,
                                        guint8 index){
    assert(index < PHRASE_INDEX_LIBRARY_COUNT);

    /* addon table. */
    context->m_addon_phrase_index->unload(index);
    return true;
}

import_iterator_t * pinyin_begin_add_phrases(pinyin_context_t * context,
                                             guint8 index){
    import_iterator_t * iter = new import_iterator_t;
    iter->m_context = context;
    iter->m_phrase_index = index;
    return iter;
}

static bool _add_phrase(pinyin_context_t * context,
                        guint8 index,
                        ChewingKeyVector keys,
                        ucs4_t * phrase,
                        glong phrase_length,
                        gint count) {
    /* if -1 == count, use the default value. */
    const gint default_count = 5;
    const guint32 unigram_factor = 3;
    if (-1 == count)
        count = default_count;

    FacadePhraseTable3 *  phrase_table = context->m_phrase_table;
    FacadeChewingTable2 * pinyin_table = context->m_pinyin_table;
    FacadePhraseIndex * phrase_index = context->m_phrase_index;

    bool result = false;

    /* check whether the phrase exists in phrase table */
    phrase_token_t token = null_token;
    GArray * tokenarray = g_array_new(FALSE, FALSE, sizeof(phrase_token_t));

    /* do phrase table search. */
    PhraseTokens tokens;
    memset(tokens, 0, sizeof(PhraseTokens));
    phrase_index->prepare_tokens(tokens);
    int retval = phrase_table->search(phrase_length, phrase, tokens);
    int num = reduce_tokens(tokens, tokenarray);
    phrase_index->destroy_tokens(tokens);

    /* find the best token candidate. */
    for (size_t i = 0; i < tokenarray->len; ++i) {
        phrase_token_t candidate = g_array_index(tokenarray, phrase_token_t, i);
        if (null_token == token) {
            token = candidate;
            continue;
        }

        if (PHRASE_INDEX_LIBRARY_INDEX(candidate) == index) {
            /* only one phrase string per sub phrase index. */
            assert(PHRASE_INDEX_LIBRARY_INDEX(token) != index);
            token = candidate;
            continue;
        }
    }
    g_array_free(tokenarray, TRUE);

    PhraseItem item;
    /* check whether it exists in the same sub phrase index; */
    if (null_token != token &&
        PHRASE_INDEX_LIBRARY_INDEX(token) == index) {
        /* if so, remove the phrase, add the pinyin for the phrase item,
           then add it back;*/
        phrase_index->get_phrase_item(token, item);
        assert(phrase_length == item.get_phrase_length());
        ucs4_t tmp_phrase[MAX_PHRASE_LENGTH];
        item.get_phrase_string(tmp_phrase);
        assert(0 == memcmp
               (phrase, tmp_phrase, sizeof(ucs4_t) * phrase_length));

        PhraseItem * removed_item = NULL;
        retval = phrase_index->remove_phrase_item(token, removed_item);
        if (ERROR_OK == retval) {
            /* maybe check whether there are duplicated pronunciations here. */
            removed_item->add_pronunciation((ChewingKey *)keys->data,
                                            count);
            phrase_index->add_phrase_item(token, removed_item);
            delete removed_item;
            result = true;
        }
    } else {
        /* if not exists in the same sub phrase index,
           get the maximum token,
           then add it directly with maximum token + 1; */
        PhraseIndexRange range;
        retval = phrase_index->get_range(index, range);

        if (ERROR_OK == retval) {
            token = range.m_range_end;
            if (0x00000000 == (token & PHRASE_MASK))
                token++;

            if (phrase_length == keys->len) { /* valid pinyin */
                phrase_table->add_index(phrase_length, phrase, token);
                pinyin_table->add_index
                    (keys->len, (ChewingKey *)(keys->data), token);

                item.set_phrase_string(phrase_length, phrase);
                item.add_pronunciation((ChewingKey *)(keys->data), count);
                phrase_index->add_phrase_item(token, &item);
                phrase_index->add_unigram_frequency(token,
                                                    count * unigram_factor);
                result = true;
            }
        }
    }

    return result;
}

bool pinyin_iterator_add_phrase(import_iterator_t * iter,
                                const char * phrase,
                                const char * pinyin,
                                gint count){

    pinyin_context_t * context = iter->m_context;
    guint8 index = iter->m_phrase_index;

    bool result = false;

    if (NULL == phrase || NULL == pinyin)
        return result;

    glong phrase_length = 0;
    ucs4_t * ucs4_phrase = g_utf8_to_ucs4(phrase, -1, NULL, &phrase_length, NULL);

    pinyin_option_t options = PINYIN_CORRECT_ALL | USE_TONE;
    FullPinyinParser2 parser;
    ChewingKeyVector keys =
        g_array_new(FALSE, FALSE, sizeof(ChewingKey));
    ChewingKeyRestVector key_rests =
        g_array_new(FALSE, FALSE, sizeof(ChewingKeyRest));

    /* parse the pinyin. */
    parser.parse(options, keys, key_rests, pinyin, strlen(pinyin));

    if (phrase_length != keys->len)
        return result;

    if (0 == phrase_length || phrase_length >= MAX_PHRASE_LENGTH)
        return result;

    result = _add_phrase(context, index, keys,
                         ucs4_phrase, phrase_length, count);

    g_array_free(key_rests, TRUE);
    g_array_free(keys, TRUE);
    g_free(ucs4_phrase);
    return result;
}

void pinyin_end_add_phrases(import_iterator_t * iter){
    /* compact the content memory chunk of phrase index. */
    iter->m_context->m_phrase_index->compact();
    iter->m_context->m_modified = true;
    delete iter;
}

export_iterator_t * pinyin_begin_get_phrases(pinyin_context_t * context,
                                             guint index){
    export_iterator_t * iter = new export_iterator_t;
    iter->m_context = context;
    iter->m_phrase_index = index;
    iter->m_next_token = null_token;
    iter->m_next_pronunciation = 0;

    /* probe next token. */
    PhraseIndexRange range;
    int retval = iter->m_context->m_phrase_index->get_range
        (iter->m_phrase_index, range);
    if (retval != ERROR_OK)
        return iter;

    PhraseItem item;
    phrase_token_t token = range.m_range_begin;
    for (; token < range.m_range_end; ++token) {
        retval = iter->m_context->m_phrase_index->get_phrase_item
            (token, item);
        if (ERROR_OK == retval && item.get_n_pronunciation() >= 1) {
            iter->m_next_token = token;
            break;
        }
    }
    return iter;
}

bool pinyin_iterator_has_next_phrase(export_iterator_t * iter){
    /* no next token. */
    if (null_token == iter->m_next_token)
        return false;
    return true;
}

/* phrase, pinyin should be freed by g_free(). */
bool pinyin_iterator_get_next_phrase(export_iterator_t * iter,
                                     gchar ** phrase,
                                     gchar ** pinyin,
                                     gint * count){
    /* count "-1" means default count. */
    *phrase = NULL; *pinyin = NULL; *count = -1;

    PhraseItem item;
    int retval = iter->m_context->m_phrase_index->get_phrase_item
        (iter->m_next_token, item);
    /* assume valid next token from previous call. */
    assert(ERROR_OK == retval);

    /* fill phrase and pronunciation pair. */
    ucs4_t phrase_ucs4[MAX_PHRASE_LENGTH];
    guint8 len = item.get_phrase_length();
    assert(item.get_phrase_string(phrase_ucs4));
    gchar * phrase_utf8 = g_ucs4_to_utf8
        (phrase_ucs4, len, NULL, NULL, NULL);

    guint8 nth_pronun = iter->m_next_pronunciation;
    guint8 n_pronuns = item.get_n_pronunciation();
    /* assume valid pronunciation from previous call. */
    assert(nth_pronun < n_pronuns);
    ChewingKey keys[MAX_PHRASE_LENGTH];
    guint32 freq = 0;
    assert(item.get_nth_pronunciation(nth_pronun, keys, freq));

    GPtrArray * array = g_ptr_array_new();
    for(size_t i = 0; i < len; ++i) {
        g_ptr_array_add(array, keys[i].get_pinyin_string());
    }
    g_ptr_array_add(array, NULL);

    gchar ** strings = (gchar **)g_ptr_array_free(array, FALSE);
    gchar * pinyins = g_strjoinv("'", strings);
    g_strfreev(strings);

    /* use default value. */
    *phrase = phrase_utf8; *pinyin = pinyins;
    if (freq > 0)
        *count = freq;

    /* probe next pronunciation. */
    nth_pronun ++;
    if (nth_pronun < n_pronuns) {
        iter->m_next_pronunciation = nth_pronun;
        return true;
    }

    iter->m_next_pronunciation = 0;
    /* probe next token. */
    PhraseIndexRange range;
    retval = iter->m_context->m_phrase_index->get_range
        (iter->m_phrase_index, range);
    if (retval != ERROR_OK) {
        iter->m_next_token = null_token;
        return true;
    }

    phrase_token_t token = iter->m_next_token + 1;
    iter->m_next_token = null_token;
    for (; token < range.m_range_end; ++token) {
        retval = iter->m_context->m_phrase_index->get_phrase_item
            (token, item);
        if (ERROR_OK == retval && item.get_n_pronunciation() >= 1) {
            iter->m_next_token = token;
            break;
        }
    }
    return true;
}

void pinyin_end_get_phrases(export_iterator_t * iter){
    delete iter;
}

static bool _write_files(pinyin_context_t * context){
    const pinyin_table_info_t * phrase_files =
        context->m_system_table_info.get_default_tables();

    /* skip the reserved zero phrase library. */
    for (size_t i = 1; i < PHRASE_INDEX_LIBRARY_COUNT; ++i) {
        PhraseIndexRange range;
        int retval = context->m_phrase_index->get_range(i, range);

        if (ERROR_NO_SUB_PHRASE_INDEX == retval)
            continue;

        const pinyin_table_info_t * table_info = phrase_files + i;

        if (NOT_USED == table_info->m_file_type)
            continue;

        const char * userfilename = table_info->m_user_filename;

        if (NULL == userfilename)
            continue;

        if (SYSTEM_FILE == table_info->m_file_type ||
            DICTIONARY == table_info->m_file_type) {
            /* system phrase library */
            MemoryChunk * chunk = new MemoryChunk;
            MemoryChunk * log = new MemoryChunk;
            const char * systemfilename = table_info->m_system_filename;

            /* check bin file in system dir. */
            gchar * chunkfilename = g_build_filename(context->m_system_dir,
                                                     systemfilename, NULL);
#ifdef LIBPINYIN_USE_MMAP
            if (!chunk->mmap(chunkfilename))
                fprintf(stderr, "mmap %s failed!\n", chunkfilename);
#else
            if (!chunk->load(chunkfilename))
                fprintf(stderr, "open %s failed!\n", chunkfilename);
#endif

            g_free(chunkfilename);
            context->m_phrase_index->diff(i, chunk, log);

            const char * userfilename = table_info->m_user_filename;
            gchar * tmpfilename = g_strdup_printf("%s.tmp", userfilename);

            gchar * tmppathname = g_build_filename(context->m_user_dir,
                                                   tmpfilename, NULL);

            log->save(tmppathname);

            g_free(tmpfilename);
            g_free(tmppathname);
            delete log;
        }

        if (USER_FILE == table_info->m_file_type) {
            /* user phrase library */
            MemoryChunk * chunk = new MemoryChunk;
            context->m_phrase_index->store(i, chunk);

            const char * userfilename = table_info->m_user_filename;
            gchar * tmpfilename = g_strdup_printf("%s.tmp", userfilename);
            gchar * tmppathname = g_build_filename(context->m_user_dir,
                                                   tmpfilename, NULL);

            chunk->save(tmppathname);

            g_free(tmpfilename);
            g_free(tmppathname);
            delete chunk;
        }
    }

    /* save user pinyin table */
    gchar * tmpfilename = g_build_filename
        (context->m_user_dir, USER_PINYIN_INDEX ".tmp", NULL);
    unlink(tmpfilename);

    context->m_pinyin_table->store(tmpfilename);

    g_free(tmpfilename);

    /* save user phrase table */
    tmpfilename = g_build_filename
        (context->m_user_dir, USER_PHRASE_INDEX ".tmp", NULL);
    unlink(tmpfilename);

    context->m_phrase_table->store(tmpfilename);

    g_free(tmpfilename);

    /* save user bi-gram */
    tmpfilename = g_build_filename
        (context->m_user_dir, USER_BIGRAM ".tmp", NULL);
    unlink(tmpfilename);
    context->m_user_bigram->save_db(tmpfilename);

    g_free(tmpfilename);

    return true;
}

static bool _rename_files(pinyin_context_t * context){
    const pinyin_table_info_t * phrase_files =
        context->m_system_table_info.get_default_tables();

    /* skip the reserved zero phrase library. */
    for (size_t i = 1; i < PHRASE_INDEX_LIBRARY_COUNT; ++i) {
        PhraseIndexRange range;
        int retval = context->m_phrase_index->get_range(i, range);

        if (ERROR_NO_SUB_PHRASE_INDEX == retval)
            continue;

        const pinyin_table_info_t * table_info = phrase_files + i;

        if (NOT_USED == table_info->m_file_type)
            continue;

        const char * userfilename = table_info->m_user_filename;

        if (NULL == userfilename)
            continue;

        if (SYSTEM_FILE == table_info->m_file_type ||
            DICTIONARY == table_info->m_file_type) {
            const char * userfilename = table_info->m_user_filename;
            gchar * tmpfilename = g_strdup_printf("%s.tmp", userfilename);

            gchar * tmppathname = g_build_filename(context->m_user_dir,
                                                   tmpfilename, NULL);
            g_free(tmpfilename);

            gchar * chunkpathname = g_build_filename(context->m_user_dir,
                                                     userfilename, NULL);

            int result = rename(tmppathname, chunkpathname);
            if (0 != result)
                fprintf(stderr, "rename %s to %s failed.\n",
                        tmppathname, chunkpathname);

            g_free(chunkpathname);
            g_free(tmppathname);
        }

        if (USER_FILE == table_info->m_file_type) {
            const char * userfilename = table_info->m_user_filename;
            gchar * tmpfilename = g_strdup_printf("%s.tmp", userfilename);
            gchar * tmppathname = g_build_filename(context->m_user_dir,
                                                   tmpfilename, NULL);
            g_free(tmpfilename);

            gchar * chunkpathname = g_build_filename(context->m_user_dir,
                                                     userfilename, NULL);

            int result = rename(tmppathname, chunkpathname);
            if (0 != result)
                fprintf(stderr, "rename %s to %s failed.\n",
                        tmppathname, chunkpathname);

            g_free(chunkpathname);
            g_free(tmppathname);
        }
    }

    /* save user pinyin table */
    gchar * tmpfilename = g_build_filename
        (context->m_user_dir, USER_PINYIN_INDEX ".tmp", NULL);
    gchar * filename = g_build_filename
        (context->m_user_dir, USER_PINYIN_INDEX, NULL);

    int result = rename(tmpfilename, filename);
    if (0 != result)
        fprintf(stderr, "rename %s to %s failed.\n",
                tmpfilename, filename);

    g_free(tmpfilename);
    g_free(filename);

    /* save user phrase table */
    tmpfilename = g_build_filename
        (context->m_user_dir, USER_PHRASE_INDEX ".tmp", NULL);
    filename = g_build_filename
        (context->m_user_dir, USER_PHRASE_INDEX, NULL);

    result = rename(tmpfilename, filename);
    if (0 != result)
        fprintf(stderr, "rename %s to %s failed.\n",
                tmpfilename, filename);

    g_free(tmpfilename);
    g_free(filename);

    /* save user bi-gram */
    tmpfilename = g_build_filename
        (context->m_user_dir, USER_BIGRAM ".tmp", NULL);
    filename = g_build_filename(context->m_user_dir, USER_BIGRAM, NULL);

    result = rename(tmpfilename, filename);
    if (0 != result)
        fprintf(stderr, "rename %s to %s failed.\n",
                tmpfilename, filename);

    g_free(tmpfilename);
    g_free(filename);

    return true;
}

bool pinyin_save(pinyin_context_t * context){
    if (!context->m_user_dir)
        return false;

    if (!context->m_modified)
        return false;

    context->m_phrase_index->compact();

    bool retval = _write_files(context) && _rename_files(context);

    mark_version(context);

    context->m_modified = false;
    return retval;
}

bool pinyin_set_full_pinyin_scheme(pinyin_context_t * context,
                                   FullPinyinScheme scheme){
    context->m_full_pinyin_parser->set_scheme(scheme);
    return true;
}

bool pinyin_set_double_pinyin_scheme(pinyin_context_t * context,
                                     DoublePinyinScheme scheme){
    context->m_double_pinyin_parser->set_scheme(scheme);
    return true;
}

bool pinyin_set_zhuyin_scheme(pinyin_context_t * context,
                               ZhuyinScheme scheme){
    delete context->m_chewing_parser;
    context->m_chewing_parser = NULL;

    switch(scheme) {
    case ZHUYIN_STANDARD:
    case ZHUYIN_IBM:
    case ZHUYIN_GINYIEH:
    case ZHUYIN_ETEN:
    case ZHUYIN_STANDARD_DVORAK: {
        ZhuyinSimpleParser2 * parser = new ZhuyinSimpleParser2();
        parser->set_scheme(scheme);
        context->m_chewing_parser = parser;
        break;
    }
    case ZHUYIN_HSU:
    case ZHUYIN_ETEN26:
    case ZHUYIN_HSU_DVORAK: {
        ZhuyinDiscreteParser2 * parser = new ZhuyinDiscreteParser2();
        parser->set_scheme(scheme);
        context->m_chewing_parser = parser;
        break;
    }
    case ZHUYIN_DACHEN_CP26:
        context->m_chewing_parser = new ZhuyinDaChenCP26Parser2();
        break;
    default:
        assert(FALSE);
    }
    return true;
}

void pinyin_fini(pinyin_context_t * context){
    delete context->m_full_pinyin_parser;
    delete context->m_double_pinyin_parser;
    delete context->m_chewing_parser;
    delete context->m_pinyin_table;
    delete context->m_phrase_table;
    delete context->m_phrase_index;
    delete context->m_system_bigram;
    delete context->m_user_bigram;
    delete context->m_pinyin_lookup;
    delete context->m_phrase_lookup;
    delete context->m_addon_pinyin_table;
    delete context->m_addon_phrase_table;
    delete context->m_addon_phrase_index;

    g_free(context->m_system_dir);
    g_free(context->m_user_dir);
    context->m_modified = false;

    delete context;
}

bool pinyin_mask_out(pinyin_context_t * context,
                     phrase_token_t mask,
                     phrase_token_t value) {

    context->m_pinyin_table->mask_out(mask, value);
    context->m_phrase_table->mask_out(mask, value);
    context->m_user_bigram->mask_out(mask, value);

    const pinyin_table_info_t * phrase_files =
        context->m_system_table_info.get_default_tables();

    /* mask out the phrase index. */
    for (size_t index = 1; index < PHRASE_INDEX_LIBRARY_COUNT; ++index) {
        PhraseIndexRange range;
        int retval = context->m_phrase_index->get_range(index, range);

        if (ERROR_NO_SUB_PHRASE_INDEX == retval)
            continue;

        const pinyin_table_info_t * table_info = phrase_files + index;

        if (NOT_USED == table_info->m_file_type)
            continue;

        const char * userfilename = table_info->m_user_filename;

        if (NULL == userfilename)
            continue;

        if (SYSTEM_FILE == table_info->m_file_type ||
            DICTIONARY == table_info->m_file_type) {
            /* system phrase library */
            MemoryChunk * chunk = new MemoryChunk;

            const char * systemfilename = table_info->m_system_filename;
            /* check bin file in system dir. */
            gchar * chunkfilename = g_build_filename(context->m_system_dir,
                                                     systemfilename, NULL);

#ifdef LIBPINYIN_USE_MMAP
            if (!chunk->mmap(chunkfilename))
                fprintf(stderr, "mmap %s failed!\n", chunkfilename);
#else
            if (!chunk->load(chunkfilename))
                fprintf(stderr, "open %s failed!\n", chunkfilename);
#endif

            g_free(chunkfilename);

            context->m_phrase_index->load(index, chunk);

            const char * userfilename = table_info->m_user_filename;

            chunkfilename = g_build_filename(context->m_user_dir,
                                             userfilename, NULL);

            MemoryChunk * log = new MemoryChunk;
            log->load(chunkfilename);
            g_free(chunkfilename);

            /* merge the chunk log with mask. */
            context->m_phrase_index->merge_with_mask(index, log, mask, value);
        }

        if (USER_FILE == table_info->m_file_type) {
            /* user phrase library */
            context->m_phrase_index->mask_out(index, mask, value);
        }
    }

    context->m_phrase_index->compact();
    return true;
}

/* copy from options to context->m_options. */
bool pinyin_set_options(pinyin_context_t * context,
                        pinyin_option_t options){
    context->m_options = options;
#if 0
    context->m_pinyin_table->set_options(context->m_options);
    context->m_pinyin_lookup->set_options(context->m_options);
#endif
    return true;
}


pinyin_instance_t * pinyin_alloc_instance(pinyin_context_t * context){
    pinyin_instance_t * instance = new pinyin_instance_t;
    instance->m_context = context;

    instance->m_prefixes = g_array_new(FALSE, FALSE, sizeof(phrase_token_t));

    instance->m_parsed_len = 0;

    instance->m_constraints = new ForwardPhoneticConstraints
        (context->m_phrase_index);

    instance->m_phrase_result = g_array_new
        (TRUE, TRUE, sizeof(phrase_token_t));
    instance->m_candidates =
        g_array_new(TRUE, TRUE, sizeof(lookup_candidate_t));

    return instance;
}

static bool _free_candidates(CandidateVector candidates) {
    /* free candidates */
    for (size_t i = 0; i < candidates->len; ++i) {
        lookup_candidate_t * candidate = &g_array_index
            (candidates, lookup_candidate_t, i);
        g_free(candidate->m_phrase_string);
    }
    g_array_set_size(candidates, 0);

    return true;
}

void pinyin_free_instance(pinyin_instance_t * instance){
    g_array_free(instance->m_prefixes, TRUE);
    delete instance->m_constraints;
    g_array_free(instance->m_phrase_result, TRUE);
    _free_candidates(instance->m_candidates);
    g_array_free(instance->m_candidates, TRUE);

    delete instance;
}

pinyin_context_t * pinyin_get_context (pinyin_instance_t * instance){
    return instance->m_context;
}

static bool pinyin_update_constraints(pinyin_instance_t * instance){
    PhoneticKeyMatrix & matrix = instance->m_matrix;
    ForwardPhoneticConstraints * constraints = instance->m_constraints;

    constraints->validate_constraint(&matrix);

    return true;
}


bool pinyin_guess_sentence(pinyin_instance_t * instance){
    pinyin_context_t * & context = instance->m_context;
    PhoneticKeyMatrix & matrix = instance->m_matrix;

    g_array_set_size(instance->m_prefixes, 0);
    g_array_append_val(instance->m_prefixes, sentence_start);

    pinyin_update_constraints(instance);
    bool retval = context->m_pinyin_lookup->get_nbest_match
        (instance->m_prefixes,
         &matrix,
         instance->m_constraints,
         &instance->m_nbest_results);

    return retval;
}

static void _compute_prefixes(pinyin_instance_t * instance,
                              const char * prefix){
    pinyin_context_t * & context = instance->m_context;
    FacadePhraseIndex * & phrase_index = context->m_phrase_index;

    glong len_str = 0;
    ucs4_t * ucs4_str = g_utf8_to_ucs4(prefix, -1, NULL, &len_str, NULL);
    GArray * tokenarray = g_array_new(FALSE, FALSE, sizeof(phrase_token_t));

    if (ucs4_str && len_str) {
        /* add prefixes. */
        for (ssize_t i = 1; i <= len_str; ++i) {
            if (i > MAX_PHRASE_LENGTH)
                break;

            ucs4_t * start = ucs4_str + len_str - i;

            PhraseTokens tokens;
            memset(tokens, 0, sizeof(tokens));
            phrase_index->prepare_tokens(tokens);
            int result = context->m_phrase_table->search(i, start, tokens);
            int num = reduce_tokens(tokens, tokenarray);
            phrase_index->destroy_tokens(tokens);

            if (result & SEARCH_OK)
                g_array_append_vals(instance->m_prefixes,
                                    tokenarray->data, tokenarray->len);
        }
    }
    g_array_free(tokenarray, TRUE);
    g_free(ucs4_str);
}

bool pinyin_guess_sentence_with_prefix(pinyin_instance_t * instance,
                                       const char * prefix){
    pinyin_context_t * & context = instance->m_context;
    PhoneticKeyMatrix & matrix = instance->m_matrix;

    g_array_set_size(instance->m_prefixes, 0);
    g_array_append_val(instance->m_prefixes, sentence_start);

    _compute_prefixes(instance, prefix);

    pinyin_update_constraints(instance);
    bool retval = context->m_pinyin_lookup->get_nbest_match
        (instance->m_prefixes,
         &matrix,
         instance->m_constraints,
         &instance->m_nbest_results);

    return retval;
}

bool pinyin_phrase_segment(pinyin_instance_t * instance,
                           const char * sentence){
    pinyin_context_t * & context = instance->m_context;

    const glong num_of_chars = g_utf8_strlen(sentence, -1);
    glong ucs4_len = 0;
    ucs4_t * ucs4_str = g_utf8_to_ucs4(sentence, -1, NULL, &ucs4_len, NULL);

    g_return_val_if_fail(num_of_chars == ucs4_len, FALSE);

    bool retval = context->m_phrase_lookup->get_best_match
        (ucs4_len, ucs4_str, instance->m_phrase_result);

    g_free(ucs4_str);
    return retval;
}

/* the returned sentence should be freed by g_free(). */
bool pinyin_get_sentence(pinyin_instance_t * instance,
                         guint8 index,
                         char ** sentence){
    pinyin_context_t * & context = instance->m_context;
    NBestMatchResults & results = instance->m_nbest_results;

    if (0 == results.size())
        return false;

    MatchResult result = NULL;
    assert(index < results.size());
    assert(results.get_result(index, result));

    bool retval = pinyin::convert_to_utf8
        (context->m_phrase_index, result,
         NULL, false, *sentence);

    return retval;
}

bool pinyin_parse_full_pinyin(pinyin_instance_t * instance,
                              const char * onepinyin,
                              ChewingKey * onekey){
    pinyin_context_t * & context = instance->m_context;
    pinyin_option_t options = context->m_options;

    gint16 distance = 0;
    int pinyin_len = strlen(onepinyin);
    bool retval = context->m_full_pinyin_parser->parse_one_key
        (options, *onekey, distance, onepinyin, pinyin_len);
    return retval;
}

size_t pinyin_parse_more_full_pinyins(pinyin_instance_t * instance,
                                      const char * pinyins){
    pinyin_context_t * & context = instance->m_context;
    pinyin_option_t options = context->m_options;
    PhoneticKeyMatrix & matrix = instance->m_matrix;

    ChewingKeyVector keys = g_array_new(TRUE, TRUE, sizeof(ChewingKey));
    ChewingKeyRestVector key_rests =
        g_array_new(TRUE, TRUE, sizeof(ChewingKeyRest));

    int parsed_len = context->m_full_pinyin_parser->parse
        (options, keys,
         key_rests, pinyins, strlen(pinyins));

    instance->m_parsed_len = parsed_len;

    fill_matrix(&matrix, keys, key_rests, parsed_len);

    resplit_step(options, &matrix);

    inner_split_step(options, &matrix);

    fuzzy_syllable_step(options, &matrix);

    g_array_free(key_rests, TRUE);
    g_array_free(keys, TRUE);
    return parsed_len;
}

bool pinyin_parse_double_pinyin(pinyin_instance_t * instance,
                                const char * onepinyin,
                                ChewingKey * onekey){
    pinyin_context_t * & context = instance->m_context;
    pinyin_option_t options = context->m_options;

    gint16 distance = 0;
    int pinyin_len = strlen(onepinyin);
    bool retval = context->m_double_pinyin_parser->parse_one_key
        (options, *onekey, distance, onepinyin, pinyin_len);
    return retval;
}

size_t pinyin_parse_more_double_pinyins(pinyin_instance_t * instance,
                                        const char * pinyins){
    pinyin_context_t * & context = instance->m_context;
    pinyin_option_t options = context->m_options;
    PhoneticKeyMatrix & matrix = instance->m_matrix;

    ChewingKeyVector keys = g_array_new(TRUE, TRUE, sizeof(ChewingKey));
    ChewingKeyRestVector key_rests =
        g_array_new(TRUE, TRUE, sizeof(ChewingKeyRest));

    int parsed_len = context->m_double_pinyin_parser->parse
        (options, keys,
         key_rests, pinyins, strlen(pinyins));

    instance->m_parsed_len = parsed_len;

    fill_matrix(&matrix, keys, key_rests, parsed_len);

    fuzzy_syllable_step(options, &matrix);

    g_array_free(key_rests, TRUE);
    g_array_free(keys, TRUE);
    return parsed_len;
}

bool pinyin_parse_chewing(pinyin_instance_t * instance,
                          const char * onechewing,
                          ChewingKey * onekey){
    pinyin_context_t * & context = instance->m_context;
    pinyin_option_t options = context->m_options;

    /* disable the zhuyin correction options. */
    options &= ~ZHUYIN_CORRECT_ALL;

    gint16 distance = 0;
    int chewing_len = strlen(onechewing);
    bool retval = context->m_chewing_parser->parse_one_key
        (options, *onekey, distance, onechewing, chewing_len );
    return retval;
}

size_t pinyin_parse_more_chewings(pinyin_instance_t * instance,
                                  const char * chewings){
    pinyin_context_t * & context = instance->m_context;
    pinyin_option_t options = context->m_options;
    PhoneticKeyMatrix & matrix = instance->m_matrix;

    /* disable the zhuyin correction options. */
    options &= ~ZHUYIN_CORRECT_ALL;

    ChewingKeyVector keys = g_array_new(TRUE, TRUE, sizeof(ChewingKey));
    ChewingKeyRestVector key_rests =
        g_array_new(TRUE, TRUE, sizeof(ChewingKeyRest));

    int parsed_len = context->m_chewing_parser->parse
        (options, keys,
         key_rests, chewings, strlen(chewings));

    instance->m_parsed_len = parsed_len;

    fill_matrix(&matrix, keys, key_rests, parsed_len);

    fuzzy_syllable_step(options, &matrix);

    g_array_free(key_rests, TRUE);
    g_array_free(keys, TRUE);
    return parsed_len;
}

size_t pinyin_get_parsed_input_length(pinyin_instance_t * instance) {
    return instance->m_parsed_len;
}

bool pinyin_in_chewing_keyboard(pinyin_instance_t * instance,
                                const char key, gchar *** symbols) {
    pinyin_context_t * & context = instance->m_context;
    pinyin_option_t options = context->m_options;

    /* disable the zhuyin correction options. */
    options &= ~ZHUYIN_CORRECT_ALL;

    return context->m_chewing_parser->in_chewing_scheme
        (options, key, *symbols);
}

static bool _token_get_phrase(FacadePhraseIndex * phrase_index,
                              phrase_token_t token,
                              guint * len,
                              gchar ** utf8_str) {
    PhraseItem item;
    ucs4_t buffer[MAX_PHRASE_LENGTH];

    int retval = phrase_index->get_phrase_item(token, item);
    if (ERROR_OK != retval)
        return false;

    item.get_phrase_string(buffer);
    guint length = item.get_phrase_length();
    if (len)
        *len = length;
    if (utf8_str)
        *utf8_str = g_ucs4_to_utf8(buffer, length, NULL, NULL, NULL);
    return true;
}

#if 0
static gint compare_item_with_token(gconstpointer lhs,
                                    gconstpointer rhs) {
    lookup_candidate_t * item_lhs = (lookup_candidate_t *)lhs;
    lookup_candidate_t * item_rhs = (lookup_candidate_t *)rhs;

    phrase_token_t token_lhs = item_lhs->m_token;
    phrase_token_t token_rhs = item_rhs->m_token;

    return (token_lhs - token_rhs);
}
#endif

static gint compare_item_with_phrase_length_and_frequency(gconstpointer lhs,
                                                          gconstpointer rhs) {
    lookup_candidate_t * item_lhs = (lookup_candidate_t *)lhs;
    lookup_candidate_t * item_rhs = (lookup_candidate_t *)rhs;

    guint8 len_lhs = item_lhs->m_phrase_length;
    guint8 len_rhs = item_rhs->m_phrase_length;

    if (len_lhs != len_rhs)
        return -(len_lhs - len_rhs); /* in descendant order */

    guint32 freq_lhs = item_lhs->m_freq;
    guint32 freq_rhs = item_rhs->m_freq;

    return -(freq_lhs - freq_rhs); /* in descendant order */
}

static gint compare_item_with_phrase_length_and_pinyin_length_and_frequency
(gconstpointer lhs, gconstpointer rhs) {
    lookup_candidate_t * item_lhs = (lookup_candidate_t *)lhs;
    lookup_candidate_t * item_rhs = (lookup_candidate_t *)rhs;

    guint8 len_lhs = item_lhs->m_phrase_length;
    guint8 len_rhs = item_rhs->m_phrase_length;

    if (len_lhs != len_rhs)
        return -(len_lhs - len_rhs); /* in descendant order */

    len_lhs = item_lhs->m_end - item_lhs->m_begin;
    len_rhs = item_rhs->m_end - item_rhs->m_begin;

    if (len_lhs != len_rhs)
        return -(len_lhs - len_rhs); /* in descendant order */

    guint32 freq_lhs = item_lhs->m_freq;
    guint32 freq_rhs = item_rhs->m_freq;

    return -(freq_lhs - freq_rhs); /* in descendant order */
}

static phrase_token_t _get_previous_token(pinyin_instance_t * instance,
                                          size_t offset) {
    pinyin_context_t * context = instance->m_context;
    TokenVector prefixes = instance->m_prefixes;
    NBestMatchResults & results = instance->m_nbest_results;

    phrase_token_t prev_token = null_token;
    ssize_t i;

    if (0 == offset) {
        /* get previous token from prefixes. */
        prev_token = sentence_start;
        size_t prev_token_len = 0;

        PhraseItem item;
        for (size_t i = 0; i < prefixes->len; ++i) {
            phrase_token_t token = g_array_index(prefixes, phrase_token_t, i);
            if (sentence_start == token)
                continue;

            int retval = context->m_phrase_index->get_phrase_item(token, item);
            if (ERROR_OK == retval) {
                size_t token_len = item.get_phrase_length();
                if (token_len > prev_token_len) {
                    /* found longer match, and save it. */
                    prev_token = token;
                    prev_token_len = token_len;
                }
            }
        }
    } else {
        /* get previous token from match results. */
        assert (0 < offset);

        /* no nbest match result. */
        if (0 == results.size())
            return prev_token;

        /* use the first candidate. */
        MatchResult result = NULL;
        assert(results.get_result(0, result));

        phrase_token_t cur_token = g_array_index
            (result, phrase_token_t, offset);
        if (null_token != cur_token) {
            for (i = offset - 1; i >= 0; --i) {
                cur_token = g_array_index(result, phrase_token_t, i);
                if (null_token != cur_token) {
                    prev_token = cur_token;
                    break;
                }
            }
        }
    }

    return prev_token;
}

static void _append_items(PhraseIndexRanges ranges,
                          lookup_candidate_t * template_item,
                          CandidateVector items) {
    /* reduce and append to a single GArray. */
    for (size_t m = 0; m < PHRASE_INDEX_LIBRARY_COUNT; ++m) {
        if (NULL == ranges[m])
            continue;

        for (size_t n = 0; n < ranges[m]->len; ++n) {
            PhraseIndexRange * range =
                &g_array_index(ranges[m], PhraseIndexRange, n);
            for (size_t k = range->m_range_begin;
                 k < range->m_range_end; ++k) {
                lookup_candidate_t item;
                item.m_candidate_type = template_item->m_candidate_type;
                item.m_token = k;
                item.m_begin = template_item->m_begin;
                item.m_end = template_item->m_end;
                item.m_freq = template_item->m_freq;
                g_array_append_val(items, item);
            }
        }
    }
}

static void _compute_frequency_of_items(pinyin_context_t * context,
                                        phrase_token_t prev_token,
                                        SingleGram * merged_gram,
                                        CandidateVector items) {
    pinyin_option_t & options = context->m_options;
    ssize_t i;

    PhraseItem cached_item;
    /* compute all freqs. */
    for (i = 0; i < items->len; ++i) {
        lookup_candidate_t * item = &g_array_index
            (items, lookup_candidate_t, i);
        phrase_token_t & token = item->m_token;

        gfloat bigram_poss = 0; guint32 total_freq = 0;

        gfloat lambda = context->m_system_table_info.get_lambda();

        /* handle addon candidates first. */
        if (ADDON_CANDIDATE == item->m_candidate_type) {
            total_freq = context->m_phrase_index->
                get_phrase_index_total_freq();

            /* assume the unigram of every addon phrases is 1. */
            context->m_addon_phrase_index->get_phrase_item
                (token, cached_item);

            /* Note: possibility value <= 1.0. */
            guint32 freq = ((1 - lambda) *
                            cached_item.get_unigram_frequency() /
                            (gfloat) total_freq) * 256 * 256 * 256;
            item->m_freq = freq;
            continue;
        }

        if (options & DYNAMIC_ADJUST) {
            if (null_token != prev_token) {
                guint32 bigram_freq = 0;
                merged_gram->get_total_freq(total_freq);
                merged_gram->get_freq(token, bigram_freq);
                if (0 != total_freq)
                    bigram_poss = bigram_freq / (gfloat)total_freq;
            }
        }

        /* compute the m_freq. */
        FacadePhraseIndex * & phrase_index = context->m_phrase_index;
        phrase_index->get_phrase_item(token, cached_item);
        total_freq = phrase_index->get_phrase_index_total_freq();
        assert (0 < total_freq);

        /* Note: possibility value <= 1.0. */
        guint32 freq = (lambda * bigram_poss * BIGRAM_FREQUENCY_DISCOUNT +
                        (1 - lambda) *
                        cached_item.get_unigram_frequency() /
                        (gfloat) total_freq) * 256 * 256 * 256;
        item->m_freq = freq;
    }
}

static bool _prepend_sentence_candidates(pinyin_instance_t * instance,
                                         CandidateVector candidates) {
    const size_t size = instance->m_nbest_results.size();

    /* check whether the nbest match candidate exists. */
    if (0 == size)
        return false;

    /* prepend nbest match candidates to candidates. */
    for (ssize_t i = size - 1; i >= 0; --i) {
        lookup_candidate_t candidate;
        candidate.m_candidate_type = NBEST_MATCH_CANDIDATE;
        candidate.m_nbest_index = i;
        g_array_prepend_val(candidates, candidate);
    }

    return true;
}

static bool _compute_phrase_length(pinyin_context_t * context,
                                   CandidateVector candidates) {
    FacadePhraseIndex * phrase_index = context->m_phrase_index;
    FacadePhraseIndex * addon_phrase_index = context->m_addon_phrase_index;

    /* populate m_phrase_length in lookup_candidate_t. */
    PhraseItem item;

    for(size_t i = 0; i < candidates->len; ++i) {
        lookup_candidate_t * candidate = &g_array_index
            (candidates, lookup_candidate_t, i);

        switch(candidate->m_candidate_type) {
        case NBEST_MATCH_CANDIDATE:
            assert(FALSE);
        case NORMAL_CANDIDATE:
        case PREDICTED_CANDIDATE: {
            phrase_index->get_phrase_item(candidate->m_token, item);
            candidate->m_phrase_length = item.get_phrase_length();
            break;
        }
        case ADDON_CANDIDATE: {
            addon_phrase_index->get_phrase_item(candidate->m_token, item);
            candidate->m_phrase_length = item.get_phrase_length();
            break;
        }
        case ZOMBIE_CANDIDATE:
            assert(FALSE);
        }
    }

    return true;
}

static bool _compute_phrase_strings_of_items(pinyin_instance_t * instance,
                                             CandidateVector candidates) {
    /* populate m_phrase_string in lookup_candidate_t. */

    for(size_t i = 0; i < candidates->len; ++i) {
        lookup_candidate_t * candidate = &g_array_index
            (candidates, lookup_candidate_t, i);

        switch(candidate->m_candidate_type) {
        case NBEST_MATCH_CANDIDATE: {
            gchar * sentence = NULL;
            pinyin_get_sentence(instance, candidate->m_nbest_index, &sentence);
            candidate->m_phrase_string = sentence;
            break;
        }
        case NORMAL_CANDIDATE:
        case PREDICTED_CANDIDATE:
            _token_get_phrase
                (instance->m_context->m_phrase_index,
                 candidate->m_token, NULL,
                 &(candidate->m_phrase_string));
            break;
        case ADDON_CANDIDATE:
            _token_get_phrase
                (instance->m_context->m_addon_phrase_index,
                 candidate->m_token, NULL,
                 &(candidate->m_phrase_string));
            break;
        case ZOMBIE_CANDIDATE:
            assert(FALSE);
        }
    }

    return true;
}

static gint compare_indexed_item_with_phrase_string(gconstpointer lhs,
                                                    gconstpointer rhs,
                                                    gpointer userdata) {
    size_t index_lhs = *((size_t *) lhs);
    size_t index_rhs = *((size_t *) rhs);
    CandidateVector candidates = (CandidateVector) userdata;

    lookup_candidate_t * candidate_lhs =
        &g_array_index(candidates, lookup_candidate_t, index_lhs);
    lookup_candidate_t * candidate_rhs =
        &g_array_index(candidates, lookup_candidate_t, index_rhs);

    return -strcmp(candidate_lhs->m_phrase_string,
                   candidate_rhs->m_phrase_string); /* in descendant order */
}


static bool _remove_duplicated_items_by_phrase_string
(pinyin_instance_t * instance,
 CandidateVector candidates) {
    size_t i;
    /* create the GArray of indexed item */
    GArray * indices = g_array_new(FALSE, FALSE, sizeof(size_t));
    for (i = 0; i < candidates->len; ++i)
        g_array_append_val(indices, i);

    /* sort the indices array by phrase array */
    g_array_sort_with_data
        (indices, compare_indexed_item_with_phrase_string, candidates);

    /* mark duplicated items as zombie candidate */
    lookup_candidate_t * cur_item, * saved_item = NULL;
    for (i = 0; i < indices->len; ++i) {
        size_t cur_index = g_array_index(indices, size_t, i);
        cur_item = &g_array_index(candidates, lookup_candidate_t, cur_index);

        /* handle the first candidate */
        if (NULL == saved_item) {
            saved_item = cur_item;
            continue;
        }

        if (0 == strcmp(saved_item->m_phrase_string,
                        cur_item->m_phrase_string)) {
            /* found duplicated candidates */

            /* both are nbest match candidate */
            if (NBEST_MATCH_CANDIDATE == saved_item->m_candidate_type &&
                NBEST_MATCH_CANDIDATE == cur_item->m_candidate_type) {
                /* keep the high possiblity one */
                if (saved_item->m_nbest_index < cur_item->m_nbest_index) {
                    cur_item->m_candidate_type = ZOMBIE_CANDIDATE;
                } else {
                    saved_item->m_candidate_type = ZOMBIE_CANDIDATE;
                    saved_item = cur_item;
                }

                continue;
            }

            /* keep nbest match candidate */
            if (NBEST_MATCH_CANDIDATE == saved_item->m_candidate_type) {
                cur_item->m_candidate_type = ZOMBIE_CANDIDATE;
                continue;
            }

            if (NBEST_MATCH_CANDIDATE == cur_item->m_candidate_type) {
                saved_item->m_candidate_type = ZOMBIE_CANDIDATE;
                saved_item = cur_item;
                continue;
            }

            /* keep the higher possiblity one
               to quickly move the word forward in the candidate list */
            if (cur_item->m_freq > saved_item->m_freq) {
                /* find better candidate */
                saved_item->m_candidate_type = ZOMBIE_CANDIDATE;
                saved_item = cur_item;
                continue;
            } else {
                cur_item->m_candidate_type = ZOMBIE_CANDIDATE;
                continue;
            }
        } else {
            /* keep the current candidate */
            saved_item = cur_item;
        }
    }

    g_array_free(indices, TRUE);

    /* remove zombie candidate from the returned candidates */
    for (i = 0; i < candidates->len; ++i) {
        lookup_candidate_t * candidate = &g_array_index
            (candidates, lookup_candidate_t, i);

        if (ZOMBIE_CANDIDATE == candidate->m_candidate_type) {
            g_free(candidate->m_phrase_string);
            g_array_remove_index(candidates, i);
            i--;
        }
    }

    return true;
}

/* offset must at the beginning of zero ChewingKey "'". */
static bool _check_offset(PhoneticKeyMatrix & matrix, size_t offset) {
    const size_t start = offset;

    ChewingKey key; ChewingKeyRest key_rest;
    const ChewingKey zero_key;

    if (start > 0) {
        const size_t index = start - 1;
        const size_t size = matrix.get_column_size(index);
        if (1 == size) {
            /* assume only one zero ChewingKey "'" here, but no check. */
            matrix.get_item(index, 0, key, key_rest);
            assert(zero_key != key);
        }
    }

    return true;
}

bool pinyin_guess_candidates(pinyin_instance_t * instance,
                             size_t offset,
                             sort_option_t sort_option) {

    pinyin_context_t * & context = instance->m_context;
    pinyin_option_t & options = context->m_options;
    PhoneticKeyMatrix & matrix = instance->m_matrix;
    CandidateVector candidates = instance->m_candidates;

    _free_candidates(candidates);

    if (0 == matrix.size())
        return false;

    /* lookup the previous token here. */
    phrase_token_t prev_token = null_token;

    if (options & DYNAMIC_ADJUST) {
        prev_token = _get_previous_token(instance, offset);
    }

    SingleGram merged_gram;
    SingleGram * system_gram = NULL, * user_gram = NULL;

    if (options & DYNAMIC_ADJUST) {
        if (null_token != prev_token) {
            context->m_system_bigram->load(prev_token, system_gram);
            context->m_user_bigram->load(prev_token, user_gram);
            merge_single_gram(&merged_gram, system_gram, user_gram);
        }
    }

    PhraseIndexRanges ranges;
    memset(ranges, 0, sizeof(ranges));
    context->m_phrase_index->prepare_ranges(ranges);

    PhraseIndexRanges addon_ranges;
    memset(addon_ranges, 0, sizeof(addon_ranges));
    context->m_addon_phrase_index->prepare_ranges(addon_ranges);

    _check_offset(matrix, offset);

    /* matrix reserved one extra slot. */
    const size_t start = offset;
    for (size_t end = start + 1; end < matrix.size();) {
        /* do pinyin search. */
        context->m_phrase_index->clear_ranges(ranges);
        int retval = search_matrix(context->m_pinyin_table, &matrix,
                                   start, end, ranges);

        context->m_addon_phrase_index->clear_ranges(addon_ranges);
        retval = search_matrix(context->m_addon_pinyin_table, &matrix,
                               start, end, addon_ranges) | retval;

        if ( !(retval & SEARCH_OK) ) {
            ++end;
            continue;
        }

        lookup_candidate_t template_item;
        template_item.m_begin = start; template_item.m_end = end;
        _append_items(ranges, &template_item, candidates);

        lookup_candidate_t addon_template_item;
        addon_template_item.m_candidate_type = ADDON_CANDIDATE;
        addon_template_item.m_begin = start;
        addon_template_item.m_end = end;
        _append_items(addon_ranges, &addon_template_item, candidates);

        if ( !(retval & SEARCH_CONTINUED) )
            break;

        /* skip the consecutive zero ChewingKey "'",
           to avoid duplicates of candidates. */
        ++end;
        ChewingKey key; ChewingKeyRest key_rest;
        const ChewingKey zero_key;
        for (; end < matrix.size(); ++end) {
            const size_t index = end - 1;
            const size_t size = matrix.get_column_size(index);

            /* assume only one zero ChewingKey "'" here, but no check. */
            if (1 != size)
                break;
            matrix.get_item(index, 0, key, key_rest);

            if (zero_key != key)
                break;
        }
    }

    context->m_phrase_index->destroy_ranges(ranges);

    /* post process to sort the candidates */

    _compute_phrase_length(context, candidates);

    _compute_frequency_of_items(context, prev_token, &merged_gram, candidates);

    /* sort the candidates. */
    switch (sort_option) {
    case SORT_BY_PHRASE_LENGTH_AND_FREQUENCY:
        g_array_sort(candidates,
                     compare_item_with_phrase_length_and_frequency);
        break;
    case SORT_BY_PHRASE_LENGTH_AND_PINYIN_LENGTH_AND_FREQUENCY:
        g_array_sort(candidates,
                     compare_item_with_phrase_length_and_pinyin_length_and_frequency);
        break;
    }

    /* post process to remove duplicated candidates */

    _prepend_sentence_candidates(instance, instance->m_candidates);

    _compute_phrase_strings_of_items(instance, instance->m_candidates);

    _remove_duplicated_items_by_phrase_string(instance, instance->m_candidates);

    if (system_gram)
        delete system_gram;
    if (user_gram)
        delete user_gram;

    return true;
}

bool pinyin_guess_predicted_candidates(pinyin_instance_t * instance,
                                       const char * prefix) {
    const guint32 length = 2;
    const guint32 filter = 10;

    pinyin_context_t * context = instance->m_context;
    FacadePhraseIndex * phrase_index = context->m_phrase_index;
    CandidateVector candidates = instance->m_candidates;
    TokenVector prefixes = instance->m_prefixes;
    phrase_token_t prev_token = null_token;

    _free_candidates(candidates);

    g_array_set_size(instance->m_prefixes, 0);
    _compute_prefixes(instance, prefix);

    if (0 == prefixes->len)
        return false;

    /* merge single gram. */
    SingleGram merged_gram;
    SingleGram * user_gram = NULL;
    for (gint i = prefixes->len - 1; i >= 0; --i) {
        prev_token = g_array_index(prefixes, phrase_token_t, i);

        context->m_user_bigram->load(prev_token, user_gram);
        merge_single_gram(&merged_gram, NULL, user_gram);

        if (user_gram)
            delete user_gram;

        if (merged_gram.get_length())
            break;
    }

    if (0 == merged_gram.get_length())
        return false;

    /* retrieve all items. */
    BigramPhraseWithCountArray tokens = g_array_new
        (FALSE, FALSE, sizeof(BigramPhraseItemWithCount));
    merged_gram.retrieve_all(tokens);

    /* sort the longer word first. */
    PhraseItem cached_item;
    for (ssize_t len = length; len > 0; --len) {
        /* append items. */
        for (size_t k = 0; k < tokens->len; ++k){
            BigramPhraseItemWithCount * phrase_item = &g_array_index
                (tokens, BigramPhraseItemWithCount, k);

            if (phrase_item->m_count < filter)
                continue;

            int result = phrase_index->get_phrase_item
                (phrase_item->m_token, cached_item);
            if (ERROR_NO_SUB_PHRASE_INDEX == result)
                continue;

            if (len != cached_item.get_phrase_length())
                continue;

            lookup_candidate_t item;
            item.m_candidate_type = PREDICTED_CANDIDATE;
            item.m_token = phrase_item->m_token;
            g_array_append_val(candidates, item);
        }

    }

    /* post process to sort the candidates */

    _compute_phrase_length(context, candidates);

    _compute_frequency_of_items(context, prev_token, &merged_gram, candidates);

    /* sort the candidates by phrase length and frequency. */
    g_array_sort(candidates, compare_item_with_phrase_length_and_frequency);

    /* post process to remove duplicated candidates */

    _compute_phrase_strings_of_items(instance, instance->m_candidates);

    _remove_duplicated_items_by_phrase_string(instance, instance->m_candidates);

    return true;
}

int pinyin_choose_candidate(pinyin_instance_t * instance,
                            size_t offset,
                            lookup_candidate_t * candidate){
    assert(PREDICTED_CANDIDATE != candidate->m_candidate_type);

    pinyin_context_t * context = instance->m_context;
    PhoneticKeyMatrix & matrix = instance->m_matrix;
    ForwardPhoneticConstraints * constraints = instance->m_constraints;
    NBestMatchResults & results = instance->m_nbest_results;

    if (NBEST_MATCH_CANDIDATE == candidate->m_candidate_type) {
        MatchResult best = NULL, other = NULL;
        assert(results.get_result(0, best));
        assert(results.get_result(candidate->m_nbest_index, other));
        constraints->diff_result(best, other);
        return matrix.size() - 1;
    }

    if (ADDON_CANDIDATE == candidate->m_candidate_type) {
        PhraseItem item;
        context->m_addon_phrase_index->get_phrase_item
            (candidate->m_token, item);

        guint8 len = item.get_phrase_length();
        guint8 npron = item.get_n_pronunciation();

        PhraseIndexRange range;
        context->m_phrase_index->get_range(ADDON_DICTIONARY, range);
        /* assume not over flow here. */
        phrase_token_t token = range.m_range_end;

        /* add pinyin index. */
        for (size_t i = 0; i < npron; ++i) {
            ChewingKey keys[MAX_PHRASE_LENGTH];
            guint32 freq = 0;
            item.get_nth_pronunciation(i, keys, freq);
            context->m_pinyin_table->add_index(len, keys, token);
        }
        /* add phrase index. */
        ucs4_t phrase[MAX_PHRASE_LENGTH];
        item.get_phrase_string(phrase);
        context->m_phrase_table->add_index(len, phrase, token);
        context->m_phrase_index->add_phrase_item(token, &item);

        /* update the candidate. */
        candidate->m_candidate_type = NORMAL_CANDIDATE;
        candidate->m_token = token;
    }

    /* sync m_constraints to the length of m_pinyin_keys. */
    bool retval = constraints->validate_constraint(&matrix);

    phrase_token_t token = candidate->m_token;
    guint8 len = constraints->add_constraint
        (candidate->m_begin, candidate->m_end, token);

    /* safe guard: validate the m_constraints again. */
    retval = constraints->validate_constraint(&matrix) && len;

    return offset + len;
}

bool pinyin_choose_predicted_candidate(pinyin_instance_t * instance,
                                       lookup_candidate_t * candidate){
    assert(PREDICTED_CANDIDATE == candidate->m_candidate_type);

    const guint32 initial_seed = 23 * 3;
    const guint32 unigram_factor = 7;

    pinyin_context_t * & context = instance->m_context;
    FacadePhraseIndex * & phrase_index = context->m_phrase_index;

    /* train uni-gram */
    phrase_token_t token = candidate->m_token;
    int error = phrase_index->add_unigram_frequency
        (token, initial_seed * unigram_factor);
    if (ERROR_INTEGER_OVERFLOW == error)
        return false;

    phrase_token_t prev_token = _get_previous_token(instance, 0);
    if (null_token == prev_token)
        return false;

    SingleGram * user_gram = NULL;
    context->m_user_bigram->load(prev_token, user_gram);

    if (NULL == user_gram)
        user_gram = new SingleGram;

    /* train bi-gram */
    guint32 total_freq = 0;
    assert(user_gram->get_total_freq(total_freq));
    guint32 freq = 0;
    if (!user_gram->get_freq(token, freq)) {
        assert(user_gram->insert_freq(token, initial_seed));
    } else {
        assert(user_gram->set_freq(token, freq + initial_seed));
    }
    assert(user_gram->set_total_freq(total_freq + initial_seed));
    context->m_user_bigram->store(prev_token, user_gram);
    delete user_gram;
    return true;
}

bool pinyin_clear_constraint(pinyin_instance_t * instance,
                             size_t offset){
    ForwardPhoneticConstraints * constraints = instance->m_constraints;

    bool retval = constraints->clear_constraint(offset);

    return retval;
}

bool pinyin_lookup_tokens(pinyin_instance_t * instance,
                          const char * phrase, GArray * tokenarray){
    pinyin_context_t * & context = instance->m_context;
    FacadePhraseIndex * & phrase_index = context->m_phrase_index;

    glong ucs4_len = 0;
    ucs4_t * ucs4_phrase = g_utf8_to_ucs4(phrase, -1, NULL, &ucs4_len, NULL);

    PhraseTokens tokens;
    memset(tokens, 0, sizeof(PhraseTokens));
    phrase_index->prepare_tokens(tokens);
    int retval = context->m_phrase_table->search(ucs4_len, ucs4_phrase, tokens);
    int num = reduce_tokens(tokens, tokenarray);
    phrase_index->destroy_tokens(tokens);

    return SEARCH_OK & retval;
}

bool pinyin_train(pinyin_instance_t * instance, guint8 index){
    if (!instance->m_context->m_user_dir)
        return false;

    pinyin_context_t * context = instance->m_context;
    PhoneticKeyMatrix & matrix = instance->m_matrix;
    NBestMatchResults & results = instance->m_nbest_results;

    if (0 == results.size())
        return false;

    context->m_modified = true;

    MatchResult result = NULL;
    assert(index < results.size());
    assert(results.get_result(index, result));

    bool retval = context->m_pinyin_lookup->train_result3
        (&matrix, instance->m_constraints, result);

    return retval;
}

bool pinyin_reset(pinyin_instance_t * instance){
    instance->m_parsed_len = 0;
    instance->m_matrix.clear_all();

    g_array_set_size(instance->m_prefixes, 0);

    instance->m_constraints->clear();
    instance->m_nbest_results.clear();
    g_array_set_size(instance->m_phrase_result, 0);
    _free_candidates(instance->m_candidates);

    return true;
}

bool pinyin_get_zhuyin_string(pinyin_instance_t * instance,
                              ChewingKey * key,
                              gchar ** utf8_str) {
    *utf8_str = NULL;
    if (0 == key->get_table_index())
        return false;

    *utf8_str = key->get_zhuyin_string();
    return true;
}

bool pinyin_get_pinyin_string(pinyin_instance_t * instance,
                              ChewingKey * key,
                              gchar ** utf8_str) {
    *utf8_str = NULL;
    if (0 == key->get_table_index())
        return false;

    *utf8_str = key->get_pinyin_string();
    return true;
}

bool pinyin_get_luoma_pinyin_string(pinyin_instance_t * instance,
                                    ChewingKey * key,
                                    gchar ** utf8_str) {
    *utf8_str = NULL;
    if (0 == key->get_table_index())
        return false;

    *utf8_str = key->get_luoma_pinyin_string();
    return true;
}

bool pinyin_get_secondary_zhuyin_string(pinyin_instance_t * instance,
                                        ChewingKey * key,
                                        gchar ** utf8_str) {
    *utf8_str = NULL;
    if (0 == key->get_table_index())
        return false;

    *utf8_str = key->get_secondary_zhuyin_string();
    return true;
}

bool pinyin_get_pinyin_strings(pinyin_instance_t * instance,
                               ChewingKey * key,
                               gchar ** shengmu,
                               gchar ** yunmu) {
    if (0 == key->get_table_index())
        return false;

    if (shengmu)
        *shengmu = key->get_shengmu_string();
    if (yunmu)
        *yunmu = key->get_yunmu_string();
    return true;
}

bool pinyin_get_pinyin_is_incomplete(pinyin_instance_t * instance,
                                     ChewingKey * key) {
    if (CHEWING_ZERO_MIDDLE == key->m_middle &&
        CHEWING_ZERO_FINAL == key->m_final) {
        assert(CHEWING_ZERO_TONE == key->m_tone);
        return true;
    }

    return false;
}

bool pinyin_token_get_phrase(pinyin_instance_t * instance,
                             phrase_token_t token,
                             guint * len,
                             gchar ** utf8_str) {
    pinyin_context_t * & context = instance->m_context;

    return _token_get_phrase(context->m_phrase_index,
                             token, len, utf8_str);
}

bool pinyin_token_get_n_pronunciation(pinyin_instance_t * instance,
                                      phrase_token_t token,
                                      guint * num){
    *num = 0;
    pinyin_context_t * & context = instance->m_context;
    PhraseItem item;

    int retval = context->m_phrase_index->get_phrase_item(token, item);
    if (ERROR_OK != retval)
        return false;

    *num = item.get_n_pronunciation();
    return true;
}

bool pinyin_token_get_nth_pronunciation(pinyin_instance_t * instance,
                                        phrase_token_t token,
                                        guint nth,
                                        ChewingKeyVector keys){
    g_array_set_size(keys, 0);
    pinyin_context_t * & context = instance->m_context;
    PhraseItem item;
    ChewingKey buffer[MAX_PHRASE_LENGTH];
    guint32 freq = 0;

    int retval = context->m_phrase_index->get_phrase_item(token, item);
    if (ERROR_OK != retval)
        return false;

    item.get_nth_pronunciation(nth, buffer, freq);
    guint8 len = item.get_phrase_length();
    g_array_append_vals(keys, buffer, len);
    return true;
}

bool pinyin_token_get_unigram_frequency(pinyin_instance_t * instance,
                                        phrase_token_t token,
                                        guint * freq) {
    *freq = 0;
    pinyin_context_t * & context = instance->m_context;
    PhraseItem item;

    int retval = context->m_phrase_index->get_phrase_item(token, item);
    if (ERROR_OK != retval)
        return false;

    *freq = item.get_unigram_frequency();
    return true;
}

bool pinyin_token_add_unigram_frequency(pinyin_instance_t * instance,
                                        phrase_token_t token,
                                        guint delta){
    pinyin_context_t * & context = instance->m_context;
    int retval = context->m_phrase_index->add_unigram_frequency
        (token, delta);
    return ERROR_OK == retval;
}

bool pinyin_get_n_candidate(pinyin_instance_t * instance,
                            guint * num) {
    *num = instance->m_candidates->len;
    return true;
}

bool pinyin_get_candidate(pinyin_instance_t * instance,
                          guint index,
                          lookup_candidate_t ** candidate) {
    CandidateVector & candidates = instance->m_candidates;

    *candidate = NULL;

    if (index >= candidates->len)
        return false;

    *candidate = &g_array_index(candidates, lookup_candidate_t, index);

    return true;
}

bool pinyin_get_candidate_type(pinyin_instance_t * instance,
                               lookup_candidate_t * candidate,
                               lookup_candidate_type_t * type) {
    *type = candidate->m_candidate_type;
    return true;
}

bool pinyin_get_candidate_string(pinyin_instance_t * instance,
                                 lookup_candidate_t * candidate,
                                 const gchar ** utf8_str) {
    *utf8_str = candidate->m_phrase_string;
    return true;
}

bool pinyin_get_candidate_nbest_index(pinyin_instance_t * instance,
                                      lookup_candidate_t * candidate,
                                      guint8 * index) {
    assert(NBEST_MATCH_CANDIDATE == candidate->m_candidate_type);
    *index = candidate->m_nbest_index;
    return true;
}

#if 0
bool pinyin_get_n_pinyin(pinyin_instance_t * instance,
                         guint * num) {
    *num = 0;

    if (instance->m_pinyin_keys->len !=
        instance->m_pinyin_key_rests->len)
        return false;

    *num = instance->m_pinyin_keys->len;
    return true;
}
#endif

/* skip the beginning of zero ChewingKey "'". */
static size_t _compute_pinyin_start(PhoneticKeyMatrix & matrix,
                                    size_t offset) {
    size_t start = offset;
    ChewingKey key; ChewingKeyRest key_rest;
    const ChewingKey zero_key;
    for (; start < matrix.size() - 1; ++start) {
        size_t size = matrix.get_column_size(start);
        if (1 != size)
            break;

        matrix.get_item(start, 0, key, key_rest);
        if (zero_key != key)
            break;
    }

    return start;
}

bool pinyin_get_pinyin_key(pinyin_instance_t * instance,
                           size_t offset,
                           ChewingKey ** ppkey) {
    PhoneticKeyMatrix & matrix = instance->m_matrix;
    *ppkey = NULL;

    if (offset >= matrix.size() - 1)
        return false;

    if (0 == matrix.get_column_size(offset))
        return false;

    _check_offset(matrix, offset);
    offset = _compute_pinyin_start(matrix, offset);

    static ChewingKey key;
    ChewingKeyRest key_rest;
    matrix.get_item(offset, 0, key, key_rest);

    *ppkey = &key;
    return true;
}

bool pinyin_get_pinyin_key_rest(pinyin_instance_t * instance,
                                size_t offset,
                                ChewingKeyRest ** ppkey_rest) {
    PhoneticKeyMatrix & matrix = instance->m_matrix;
    *ppkey_rest = NULL;

    if (offset >= matrix.size() - 1)
        return false;

    if (0 == matrix.get_column_size(offset))
        return false;

    _check_offset(matrix, offset);
    offset = _compute_pinyin_start(matrix, offset);

    ChewingKey key;
    static ChewingKeyRest key_rest;
    matrix.get_item(offset, 0, key, key_rest);

    *ppkey_rest = &key_rest;
    return true;
}

bool pinyin_get_pinyin_key_rest_positions(pinyin_instance_t * instance,
                                          ChewingKeyRest * key_rest,
                                          guint16 * begin, guint16 * end) {
    if (begin)
        *begin = key_rest->m_raw_begin;

    if (end)
        *end = key_rest->m_raw_end;

    return true;
}

bool pinyin_get_pinyin_key_rest_length(pinyin_instance_t * instance,
                                       ChewingKeyRest * key_rest,
                                       guint16 * length) {
    *length = key_rest->length();
    return true;
}

/* find the first zero ChewingKey "'". */
static size_t _compute_zero_start(PhoneticKeyMatrix & matrix, size_t offset) {
    ChewingKey key; ChewingKeyRest key_rest;
    const ChewingKey zero_key;

    ssize_t index = offset - 1;
    for (; index > 0; --index) {
        const size_t size = matrix.get_column_size(index);

        if (1 != size)
            break;

        matrix.get_item(index, 0, key, key_rest);
        if (zero_key == key)
            offset = index;
        else
            break;
    }

    return offset;
}

/* when lookup offset:
   get the previous non-zero ChewingKey, then the first zero ChewingKey. */
bool pinyin_get_pinyin_offset(pinyin_instance_t * instance,
                              size_t cursor,
                              size_t * poffset) {
    PhoneticKeyMatrix & matrix = instance->m_matrix;
    size_t offset = std_lite::min(cursor, instance->m_parsed_len);

    /* find the first ChewingKey. */
    for (; offset > 0; --offset) {
        const size_t size = matrix.get_column_size(offset);

        if (size > 0)
            break;
    }

    offset = _compute_zero_start(matrix, offset);
    _check_offset(matrix, offset);

    *poffset = offset;
    return true;
}

bool pinyin_get_left_pinyin_offset(pinyin_instance_t * instance,
                                   size_t offset,
                                   size_t * pleft) {
    PhoneticKeyMatrix & matrix = instance->m_matrix;
    _check_offset(matrix, offset);

    /* find the ChewingKey ends at offset. */
    size_t left = offset > 0 ? offset - 1 : 0;

    ChewingKey key; ChewingKeyRest key_rest;
    for (; left > 0; --left) {
        const size_t size = matrix.get_column_size(left);

        size_t i = 0;
        for (; i < size; ++i) {
            matrix.get_item(left, i, key, key_rest);

            if (offset == key_rest.m_raw_end)
                break;
        }

        if (i < size)
            break;
    }

    left = _compute_zero_start(matrix, left);
    _check_offset(matrix, left);

    *pleft = left;
    return true;
}

bool pinyin_get_right_pinyin_offset(pinyin_instance_t * instance,
                                    size_t offset,
                                    size_t * pright) {
    PhoneticKeyMatrix & matrix = instance->m_matrix;
    _check_offset(matrix, offset);

    /* find the first non-zero ChewingKey. */
    size_t right = offset;

    ChewingKey key; ChewingKeyRest key_rest;
    const ChewingKey zero_key;
    for (size_t index = right; index < matrix.size() - 1; ++index) {
        const size_t size = matrix.get_column_size(index);

        if (1 != size)
            break;

        matrix.get_item(index, 0, key, key_rest);
        if (zero_key == key)
            right = index + 1;
        else
            break;
    }

    if (0 == matrix.get_column_size(right))
        return false;

    matrix.get_item(right, 0, key, key_rest);
    right = key_rest.m_raw_end;
    _check_offset(matrix, right);

    *pright = right;
    return true;
}

static bool _pre_compute_tokens(pinyin_context_t * context,
                                TokenVector cached_tokens,
                                ucs4_t * phrase,
                                size_t phrase_length) {
    FacadePhraseIndex * phrase_index = context->m_phrase_index;
    FacadePhraseTable3 * phrase_table = context->m_phrase_table;

    /* do phrase table search. */
    PhraseTokens tokens;
    memset(tokens, 0, sizeof(PhraseTokens));
    phrase_index->prepare_tokens(tokens);

    for (size_t i = 0; i < phrase_length; ++i) {
        phrase_token_t token = null_token;
        ucs4_t character = phrase[i];

        phrase_index->clear_tokens(tokens);
        int retval = phrase_table->search(1, &character, tokens);

        int num = get_first_token(tokens, token);
        /* en-counter un-known character, such as the emoji unicode. */
        if (0 == num) {
            phrase_index->destroy_tokens(tokens);
            return false;
        }

        g_array_append_val(cached_tokens, token);
    }

    phrase_index->destroy_tokens(tokens);

    return true;
}

static bool _get_char_offset_recur(pinyin_instance_t * instance,
                                   TokenVector cached_tokens,
                                   size_t start,
                                   size_t offset,
                                   size_t * plength) {
    pinyin_context_t * context = instance->m_context;
    PhoneticKeyMatrix & matrix = instance->m_matrix;
    FacadePhraseIndex * phrase_index = context->m_phrase_index;
    size_t length = *plength;

    if (start > offset)
        return true;

    const size_t size = matrix.get_column_size(start);
    /* assume pinyin parsers will filter invalid keys. */
    assert(size > 0);

    bool result = false;

    PhraseItem item;
    for (size_t i = 0; i < size; ++i) {
        ChewingKey key; ChewingKeyRest key_rest;
        matrix.get_item(start, i, key, key_rest);

        const size_t newstart = key_rest.m_raw_end;

        const ChewingKey zero_key;
        if (zero_key == key) {
            /* assume only one key here for "'" or the last key. */
            assert(1 == size);
            return _get_char_offset_recur
                (instance, cached_tokens, newstart, offset, plength);
        }

        /* check pronunciation */
        phrase_token_t token = g_array_index
            (cached_tokens, phrase_token_t, length);
        phrase_index->get_phrase_item(token, item);

        gfloat pinyin_poss = item.get_pronunciation_possibility(&key);
        if (pinyin_poss < FLT_EPSILON)
            continue;

        if (newstart > offset)
            return true;

        ++length;

        result = _get_char_offset_recur
            (instance, cached_tokens, newstart, offset, &length);
        if (result) {
            *plength = length;
            return result;
        }

        --length;
    }

    return result;
}

bool pinyin_get_character_offset(pinyin_instance_t * instance,
                                 const char * phrase,
                                 size_t offset,
                                 size_t * plength) {
    pinyin_context_t * context = instance->m_context;
    PhoneticKeyMatrix & matrix = instance->m_matrix;

    if (0 == matrix.size())
        return false;

    assert(offset < matrix.size());
    _check_offset(matrix, offset);

    if (NULL == phrase)
        return false;

    glong phrase_length = 0;
    ucs4_t * ucs4_phrase = g_utf8_to_ucs4(phrase, -1, NULL, &phrase_length, NULL);

    if (0 == phrase_length)
        return false;

    size_t length = 0;
    const size_t start = 0;

    /* pre-compute the tokens vector from phrase. */
    TokenVector cached_tokens = g_array_new(TRUE, TRUE, sizeof(phrase_token_t));

    bool retval = _pre_compute_tokens
        (context, cached_tokens, ucs4_phrase, phrase_length);

    if (!retval) {
        g_array_free(cached_tokens, TRUE);
        g_free(ucs4_phrase);
        return false;
    }

    assert(cached_tokens->len == phrase_length);

    bool result = _get_char_offset_recur
        (instance, cached_tokens, start, offset, &length);

    g_array_free(cached_tokens, TRUE);
    g_free(ucs4_phrase);

    *plength = length;
    return result;
}

#if 0
bool pinyin_get_character_offset(pinyin_instance_t * instance,
                                 size_t offset,
                                 size_t * plength) {
    pinyin_context_t * context = instance->m_context;
    FacadePhraseIndex * phrase_index = context->m_phrase_index;

    PhoneticKeyMatrix & matrix = instance->m_matrix;
    MatchResults results = instance->m_match_results;
    _check_offset(matrix, offset);

    size_t length = 0;
    PhraseItem item;
    for (size_t i = 0; i < offset; ++i) {
        phrase_token_t token = g_array_index(results, phrase_token_t, i);
        if (null_token == token)
            continue;

        int retval = phrase_index->get_phrase_item(token, item);
        assert(ERROR_OK == retval);
        guint8 len = item.get_phrase_length();
        length += len;
    }

    *plength = length;
    return true;
}
#endif

bool pinyin_get_n_phrase(pinyin_instance_t * instance,
                         guint * num) {
    *num = instance->m_phrase_result->len;
    return true;
}

bool pinyin_get_phrase_token(pinyin_instance_t * instance,
                             guint index,
                             phrase_token_t * token){
    MatchResult & result = instance->m_phrase_result;

    *token = null_token;

    if (index >= result->len)
        return false;

    *token = g_array_index(result, phrase_token_t, index);

    return true;
}

/* for auxiliary text:
   use slow string concatenation,
   and show the first ChewingKey and ChewingKeyRest together. */
static gchar * _get_aux_text_prefix(pinyin_instance_t * instance,
                                    size_t cursor,
                                    pinyin_option_t options) {
    PhoneticKeyMatrix & matrix = instance->m_matrix;
    gchar * prefix = g_strdup("");

    assert(cursor < matrix.size());
    size_t offset = 0;
    ChewingKey key; ChewingKeyRest key_rest;
    while (offset < matrix.size()) {
        offset = _compute_pinyin_start(matrix, offset);

        /* at the end of user input */
        if (matrix.size() - 1 == offset)
            break;

        assert(matrix.get_column_size(offset) >= 1);
        matrix.get_item(offset, 0, key, key_rest);

        if (cursor < key_rest.m_raw_end)
            break;

        gchar * str = NULL;
        if (IS_PINYIN == options)
            str = key.get_pinyin_string();
        else if (IS_ZHUYIN == options)
            str = key.get_zhuyin_string();
        else
            assert(FALSE);

        gchar * newprefix = g_strconcat(prefix, str, " ", NULL);

        g_free(str);
        g_free(prefix);
        prefix = newprefix;

        offset = key_rest.m_raw_end;
    }

    return prefix;
}

static gchar * _get_aux_text_postfix(pinyin_instance_t * instance,
                                     size_t cursor,
                                     pinyin_option_t options) {
    PhoneticKeyMatrix & matrix = instance->m_matrix;
    gchar * postfix = g_strdup("");

    assert(cursor < matrix.size());
    size_t offset = 0;
    ChewingKey key; ChewingKeyRest key_rest;
    while (offset < matrix.size()) {
        offset = _compute_pinyin_start(matrix, offset);

        /* at the end of user input */
        if (matrix.size() - 1 == offset)
            break;

        assert(matrix.get_column_size(offset) >= 1);
        matrix.get_item(offset, 0, key, key_rest);

        if (cursor > key_rest.m_raw_begin) {
            offset = key_rest.m_raw_end;
            continue;
        }

        gchar * str = NULL;
        if (IS_PINYIN == options)
            str = key.get_pinyin_string();
        else if (IS_ZHUYIN == options)
            str = key.get_zhuyin_string();
        else
            assert(FALSE);

        gchar * newpostfix = g_strconcat(postfix, str, " ", NULL);

        g_free(str);
        g_free(postfix);
        postfix = newpostfix;

        offset = key_rest.m_raw_end;
    }

    return postfix;
}

bool pinyin_get_full_pinyin_auxiliary_text(pinyin_instance_t * instance,
                                           size_t cursor,
                                           gchar ** aux_text) {
    PhoneticKeyMatrix & matrix = instance->m_matrix;
    if (0 == matrix.size()) {
        *aux_text = g_strdup("");
        return false;
    }

    cursor = std_lite::min(cursor, instance->m_parsed_len);
    gchar * prefix = _get_aux_text_prefix
        (instance, cursor, IS_PINYIN);
    gchar * postfix = _get_aux_text_postfix
        (instance, cursor, IS_PINYIN);

    gchar * middle = NULL;
    assert(cursor < matrix.size());
    size_t offset = 0;
    ChewingKey key; ChewingKeyRest key_rest;
    while(offset < matrix.size()) {
        size_t newoffset = _compute_pinyin_start(matrix, offset);

        /* at the pinyin boundary of user input */
        if (offset <= cursor && cursor <= newoffset){
            middle = g_strdup("|");
            break;
        }

        offset = newoffset;
        assert(matrix.get_column_size(offset) >= 1);
        matrix.get_item(offset, 0, key, key_rest);

        /* at the middle of pinyin key */
        const size_t begin = key_rest.m_raw_begin;
        const size_t end = key_rest.m_raw_end;
        const size_t len = cursor - begin;
        if (begin < cursor && cursor < end) {
            gchar * pinyin = key.get_pinyin_string();
            gchar * left = g_strndup(pinyin, len);
            gchar * right = g_strdup(pinyin + len);
            middle = g_strconcat(left, "|", right, " ", NULL);
            g_free(left);
            g_free(right);
            g_free(pinyin);
            break;
        }

        offset = key_rest.m_raw_end;
    }

    gchar * auxtext = g_strconcat(prefix, middle, postfix, NULL);
    g_free(prefix);
    g_free(middle);
    g_free(postfix);

    *aux_text = auxtext;
    return true;
}

bool pinyin_get_double_pinyin_auxiliary_text(pinyin_instance_t * instance,
                                             size_t cursor,
                                             gchar ** aux_text) {
    PhoneticKeyMatrix & matrix = instance->m_matrix;
    if (0 == matrix.size()) {
        *aux_text = g_strdup("");
        return false;
    }

    cursor = std_lite::min(cursor, instance->m_parsed_len);
    gchar * prefix = _get_aux_text_prefix
        (instance, cursor, IS_PINYIN);
    gchar * postfix = _get_aux_text_postfix
        (instance, cursor, IS_PINYIN);

    gchar * middle = NULL;
    /* no "'" support in double pinyin. */
    assert(cursor < matrix.size());
    size_t offset = 0;
    ChewingKey key; ChewingKeyRest key_rest;
    while(offset < matrix.size()) {
        /* at the pinyin boundary of user input */
        if (cursor == offset) {
            middle = g_strdup("|");
            break;
        }

        assert(matrix.get_column_size(offset) >= 1);
        matrix.get_item(offset, 0, key, key_rest);

        const size_t begin = key_rest.m_raw_begin;
        const size_t end = key_rest.m_raw_end;
        if (!(begin < cursor && cursor < end)) {
            offset = key_rest.m_raw_end;
            continue;
        }

        gchar * shengmu = key.get_shengmu_string();
        gchar * yunmu = key.get_yunmu_string();
        const size_t len = cursor - begin;
        switch(len) {
        case 1:
            middle = g_strconcat(shengmu, "|", yunmu, NULL);
            break;
        case 2:
            middle = g_strconcat(shengmu, yunmu, "|", NULL);
            break;
        default:
            assert(FALSE);
        }

        g_free(shengmu);
        g_free(yunmu);

        gchar * newmiddle = NULL;

        if (CHEWING_ZERO_TONE != key.m_tone) {
            newmiddle = g_strdup_printf("%s%d", middle, key.m_tone);
            g_free(middle);
            middle = newmiddle;
        }

        newmiddle = g_strconcat(middle, " ", NULL);
        g_free(middle);
        middle = newmiddle;

        offset = key_rest.m_raw_end;
    }

    gchar * auxtext = g_strconcat(prefix, middle, postfix, NULL);
    g_free(prefix);
    g_free(middle);
    g_free(postfix);

    *aux_text = auxtext;
    return true;
}

bool pinyin_get_chewing_auxiliary_text(pinyin_instance_t * instance,
                                       size_t cursor,
                                       gchar ** aux_text) {
    PhoneticKeyMatrix & matrix = instance->m_matrix;
    if (0 == matrix.size()) {
        *aux_text = g_strdup("");
        return false;
    }

    cursor = std_lite::min(cursor, instance->m_parsed_len);
    gchar * prefix = _get_aux_text_prefix
        (instance, cursor, IS_ZHUYIN);
    gchar * postfix = _get_aux_text_postfix
        (instance, cursor, IS_ZHUYIN);

    gchar * middle = NULL;
    /* no "'" support in zhuyin */
    assert(cursor < matrix.size());
    size_t offset = 0;
    ChewingKey key; ChewingKeyRest key_rest;
    while(offset < matrix.size()) {
        /* at the pinyin boundary of user input */
        if (cursor == offset) {
            middle = g_strdup("|");
            break;
        }

        assert(matrix.get_column_size(offset) >= 1);
        matrix.get_item(offset, 0, key, key_rest);

        const size_t begin = key_rest.m_raw_begin;
        const size_t end = key_rest.m_raw_end;
        if (!(begin < cursor && cursor < end)) {
            offset = key_rest.m_raw_end;
            continue;
        }

        gchar * zhuyin = key.get_zhuyin_string();
        const size_t len = cursor - begin;
        gchar * left = g_utf8_substring(zhuyin, 0, len);
        gchar * right = g_utf8_substring(zhuyin, len, end);

        middle = g_strconcat(left, "|", right, " ", NULL);

        g_free(left);
        g_free(right);
        g_free(zhuyin);

        offset = key_rest.m_raw_end;
    }

    gchar * auxtext = g_strconcat(prefix, middle, postfix, NULL);
    g_free(prefix);
    g_free(middle);
    g_free(postfix);

    *aux_text = auxtext;
    return true;
}

static bool _remember_phrase_recur(pinyin_instance_t * instance,
                                   ChewingKeyVector cached_keys,
                                   TokenVector cached_tokens,
                                   size_t start,
                                   ucs4_t * phrase,
                                   gint count) {
    pinyin_context_t * context = instance->m_context;
    PhoneticKeyMatrix & matrix = instance->m_matrix;
    FacadePhraseIndex * phrase_index = context->m_phrase_index;
    const guint8 index = USER_DICTIONARY;
    const size_t end = matrix.size() - 1;
    const glong phrase_length = cached_tokens->len;

    if (start > end)
        return false;

    /* only do remember phrase with 'start' and 'end' */
    if (start == end) {
        if (cached_keys->len != phrase_length)
            return false;

        /* as cached_keys and phrase has the same length. */
        if (cached_keys->len <= 0)
            return false;
        if (cached_keys->len > MAX_PHRASE_LENGTH)
            return false;

        return _add_phrase(context, index, cached_keys,
                           phrase, phrase_length, count);
    }

    const size_t size = matrix.get_column_size(start);
    /* assume pinyin parsers will filter invalid keys. */
    if (size <= 0)
        return false;

    bool result = false;

    PhraseItem item;
    for (size_t i = 0; i < size; ++i) {
        ChewingKey key; ChewingKeyRest key_rest;
        matrix.get_item(start, i, key, key_rest);

        const size_t newstart = key_rest.m_raw_end;

        const ChewingKey zero_key;
        if (zero_key == key) {
            /* assume only one key here for "'" or the last key. */
            if (1 != size)
                return false;

            return _remember_phrase_recur
                (instance, cached_keys, cached_tokens,
                 newstart, phrase, count);
        }

#if 0
        /* meet in-complete pinyin */
        if (CHEWING_ZERO_MIDDLE == key.m_middle &&
            CHEWING_ZERO_FINAL == key.m_final) {
            assert(CHEWING_ZERO_TONE == key.m_tone);
            return false;
        }
#endif

        /* check pronunciation */
        if (cached_keys->len >= phrase_length)
            return false;

        phrase_token_t token = g_array_index
            (cached_tokens, phrase_token_t, cached_keys->len);
        phrase_index->get_phrase_item(token, item);

        gfloat pinyin_poss = item.get_pronunciation_possibility(&key);
        if (pinyin_poss < FLT_EPSILON)
            continue;

        /* push value */
        g_array_append_val(cached_keys, key);

        result = _remember_phrase_recur
            (instance, cached_keys, cached_tokens,
             newstart, phrase, count) || result;

        /* pop value */
        g_array_set_size(cached_keys, cached_keys->len - 1);
    }

    return result;
}

bool pinyin_remember_user_input(pinyin_instance_t * instance,
                                const char * phrase,
                                gint count) {
    pinyin_context_t * context = instance->m_context;

    if (NULL == phrase)
        return false;

    glong phrase_length = 0;
    ucs4_t * ucs4_phrase = g_utf8_to_ucs4(phrase, -1, NULL, &phrase_length, NULL);

    if (0 == phrase_length || phrase_length >= MAX_PHRASE_LENGTH)
        return false;

    const size_t start = 0;

    /* pre-compute the tokens vector from phrase. */
    TokenVector cached_tokens = g_array_new(TRUE, TRUE, sizeof(phrase_token_t));

    bool retval = _pre_compute_tokens
        (context, cached_tokens, ucs4_phrase, phrase_length);

    if (!retval) {
        g_array_free(cached_tokens, TRUE);
        g_free(ucs4_phrase);
        return false;
    }

    if (cached_tokens->len != phrase_length)
        return false;

    ChewingKeyVector cached_keys = g_array_new(TRUE, TRUE, sizeof(ChewingKey));

    bool result = _remember_phrase_recur
        (instance, cached_keys, cached_tokens,
         start, ucs4_phrase, count);

    g_array_free(cached_tokens, TRUE);
    g_array_free(cached_keys, TRUE);
    g_free(ucs4_phrase);
    return result;
}

bool pinyin_is_user_candidate(pinyin_instance_t * instance,
                              lookup_candidate_t * candidate) {
    if (NORMAL_CANDIDATE != candidate->m_candidate_type)
        return false;

    phrase_token_t token = candidate->m_token;
    guint8 index = PHRASE_INDEX_LIBRARY_INDEX(token);
    if (USER_DICTIONARY != index)
        return false;

    return true;
}

bool pinyin_remove_user_candidate(pinyin_instance_t * instance,
                                  lookup_candidate_t * candidate) {
    pinyin_context_t * context = instance->m_context;
    FacadePhraseIndex * phrase_index = context->m_phrase_index;
    FacadePhraseTable3 * phrase_table = context->m_phrase_table;
    FacadeChewingTable2 * pinyin_table = context->m_pinyin_table;
    Bigram * user_bigram = context->m_user_bigram;

    assert(NORMAL_CANDIDATE == candidate->m_candidate_type);

    phrase_token_t token = candidate->m_token;
    guint8 index = PHRASE_INDEX_LIBRARY_INDEX(token);
    assert(USER_DICTIONARY == index);

    /* remove from phrase index */
    PhraseItem * item = NULL;
    int retval = phrase_index->remove_phrase_item(token, item);
    assert(ERROR_OK == retval);

    /* remove from phrase table */
    const guint8 length = item->get_phrase_length();
    ucs4_t phrase[MAX_PHRASE_LENGTH];
    item->get_phrase_string(phrase);
    retval = phrase_table->remove_index(length, phrase, token);
    assert(ERROR_OK == retval);

    /* remove from pinyin table */
    const guint8 num = item->get_n_pronunciation();
    ChewingKey keys[MAX_PHRASE_LENGTH];
    guint32 freq = 0;
    for (size_t i = 0; i < num; ++i) {
        item->get_nth_pronunciation(i, keys, freq);
        retval = pinyin_table->remove_index(length, keys, token);
        assert(ERROR_OK == retval);
    }

    delete item;

    /* remove from user bigram */
    phrase_token_t mask = PHRASE_INDEX_LIBRARY_MASK | PHRASE_MASK;
    user_bigram->mask_out(mask, token);

    return true;
}


/**
 *  Note: prefix is the text before the pre-edit string.
 */
