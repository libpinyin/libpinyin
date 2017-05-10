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
