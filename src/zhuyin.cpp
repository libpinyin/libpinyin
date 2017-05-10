/* 
 *  libpinyin
 *  Library to deal with pinyin.
 *  
 *  Copyright (C) 2017 Peng Wu <alexepico@gmail.com>
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

#include "zhuyin.h"
#include <stdio.h>
#include <unistd.h>
#include <glib/gstdio.h>
#include "pinyin_internal.h"


using namespace pinyin;

/* a glue layer for input method integration. */

typedef GArray * CandidateVector; /* GArray of lookup_candidate_t */

struct _zhuyin_context_t{
    pinyin_option_t m_options;

    /* input parsers. */
    FullPinyinScheme m_full_pinyin_scheme;
    FullPinyinParser2 * m_full_pinyin_parser;
    ZhuyinParser2 * m_chewing_parser;

    /* default tables. */
    FacadeChewingTable2 * m_pinyin_table;
    FacadePhraseTable3 * m_phrase_table;
    FacadePhraseIndex * m_phrase_index;
    Bigram * m_system_bigram;
    Bigram * m_user_bigram;

    /* lookups. */
    PhoneticLookup<1> * m_pinyin_lookup;
    PhraseLookup * m_phrase_lookup;

    char * m_system_dir;
    char * m_user_dir;
    bool m_modified;

    SystemTableInfo2 m_system_table_info;
};

struct _zhuyin_instance_t{
    /* pointer of zhuyin_context_t. */
    zhuyin_context_t * m_context;

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
        m_candidate_type = NORMAL_CANDIDATE_AFTER_CURSOR;
        m_phrase_string = NULL;
        m_token = null_token;
        m_phrase_length = 0;
        m_nbest_index = -1;
        m_begin = 0; m_end = 0;
        m_freq = 0;
    }
};

struct _import_iterator_t{
    zhuyin_context_t * m_context;
    guint8 m_phrase_index;
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

static bool check_format(zhuyin_context_t * context){
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

static bool mark_version(zhuyin_context_t * context){
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

zhuyin_context_t * zhuyin_init(const char * systemdir, const char * userdir){
    zhuyin_context_t * context = new zhuyin_context_t;

    context->m_options = USE_TONE | FORCE_TONE;

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

    context->m_full_pinyin_scheme = FULL_PINYIN_DEFAULT;
    context->m_full_pinyin_parser = new FullPinyinParser2;
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

    context->m_pinyin_lookup = new PhoneticLookup<1>
        ( lambda,
          context->m_pinyin_table, context->m_phrase_index,
          context->m_system_bigram, context->m_user_bigram);

    context->m_phrase_lookup = new PhraseLookup
        (lambda,
         context->m_phrase_table, context->m_phrase_index,
         context->m_system_bigram, context->m_user_bigram);

    return context;
}

bool zhuyin_load_phrase_library(zhuyin_context_t * context,
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

bool zhuyin_unload_phrase_library(zhuyin_context_t * context,
                                  guint8 index){
    assert(index < PHRASE_INDEX_LIBRARY_COUNT);

    /* default table. */
    /* tsi.bin can't be unloaded. */
    if (TSI_DICTIONARY == index)
        return false;

    context->m_phrase_index->unload(index);
    return true;
}

import_iterator_t * zhuyin_begin_add_phrases(zhuyin_context_t * context,
                                             guint8 index){
    import_iterator_t * iter = new import_iterator_t;
    iter->m_context = context;
    iter->m_phrase_index = index;
    return iter;
}

static bool _add_phrase(zhuyin_context_t * context,
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

bool zhuyin_iterator_add_phrase(import_iterator_t * iter,
                                const char * phrase,
                                const char * pinyin,
                                gint count){
    zhuyin_context_t * context = iter->m_context;
    guint8 index = iter->m_phrase_index;

    bool result = false;

    if (NULL == phrase || NULL == pinyin)
        return result;

    glong phrase_length = 0;
    ucs4_t * ucs4_phrase = g_utf8_to_ucs4(phrase, -1, NULL, &phrase_length, NULL);

    pinyin_option_t options = USE_TONE | FORCE_TONE;
    ZhuyinDirectParser2 parser;
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

void zhuyin_end_add_phrases(import_iterator_t * iter){
    /* compact the content memory chunk of phrase index. */
    iter->m_context->m_phrase_index->compact();
    iter->m_context->m_modified = true;
    delete iter;
}

bool zhuyin_save(zhuyin_context_t * context){
    if (!context->m_user_dir)
        return false;

    if (!context->m_modified)
        return false;

    context->m_phrase_index->compact();

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
            g_free(tmpfilename);

            gchar * chunkpathname = g_build_filename(context->m_user_dir,
                                                     userfilename, NULL);
            log->save(tmppathname);

            int result = rename(tmppathname, chunkpathname);
            if (0 != result)
                fprintf(stderr, "rename %s to %s failed.\n",
                        tmppathname, chunkpathname);

            g_free(chunkpathname);
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
            g_free(tmpfilename);

            gchar * chunkpathname = g_build_filename(context->m_user_dir,
                                                     userfilename, NULL);

            chunk->save(tmppathname);

            int result = rename(tmppathname, chunkpathname);
            if (0 != result)
                fprintf(stderr, "rename %s to %s failed.\n",
                        tmppathname, chunkpathname);

            g_free(chunkpathname);
            g_free(tmppathname);
            delete chunk;
        }
    }

    /* save user pinyin table */
    gchar * tmpfilename = g_build_filename
        (context->m_user_dir, USER_PINYIN_INDEX ".tmp", NULL);
    unlink(tmpfilename);
    gchar * filename = g_build_filename
        (context->m_user_dir, USER_PINYIN_INDEX, NULL);

    context->m_pinyin_table->store(tmpfilename);

    int result = rename(tmpfilename, filename);
    if (0 != result)
        fprintf(stderr, "rename %s to %s failed.\n",
                tmpfilename, filename);

    g_free(tmpfilename);
    g_free(filename);

    /* save user phrase table */
    tmpfilename = g_build_filename
        (context->m_user_dir, USER_PHRASE_INDEX ".tmp", NULL);
    unlink(tmpfilename);
    filename = g_build_filename
        (context->m_user_dir, USER_PHRASE_INDEX, NULL);

    context->m_phrase_table->store(tmpfilename);

    result = rename(tmpfilename, filename);
    if (0 != result)
        fprintf(stderr, "rename %s to %s failed.\n",
                tmpfilename, filename);

    g_free(tmpfilename);
    g_free(filename);

    /* save user bi-gram */
    tmpfilename = g_build_filename
        (context->m_user_dir, USER_BIGRAM ".tmp", NULL);
    unlink(tmpfilename);
    filename = g_build_filename(context->m_user_dir, USER_BIGRAM, NULL);
    context->m_user_bigram->save_db(tmpfilename);

    result = rename(tmpfilename, filename);
    if (0 != result)
        fprintf(stderr, "rename %s to %s failed.\n",
                tmpfilename, filename);

    g_free(tmpfilename);
    g_free(filename);

    mark_version(context);

    context->m_modified = false;
    return true;
}

bool zhuyin_set_full_pinyin_scheme(zhuyin_context_t * context,
                                   FullPinyinScheme scheme){
    context->m_full_pinyin_scheme = scheme;
    context->m_full_pinyin_parser->set_scheme(scheme);
    return true;
}

bool zhuyin_set_chewing_scheme(zhuyin_context_t * context,
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

void zhuyin_fini(zhuyin_context_t * context){
    delete context->m_full_pinyin_parser;
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
    context->m_modified = false;

    delete context;
}

bool zhuyin_mask_out(zhuyin_context_t * context,
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
bool zhuyin_set_options(zhuyin_context_t * context,
                        pinyin_option_t options){
    context->m_options = options;
#if 0
    context->m_pinyin_table->set_options(context->m_options);
    context->m_pinyin_lookup->set_options(context->m_options);
#endif
    return true;
}
