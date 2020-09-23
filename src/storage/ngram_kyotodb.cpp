/* 
 *  libpinyin
 *  Library to deal with pinyin.
 *  
 *  Copyright (C) 2013 Peng Wu <alexepico@gmail.com>
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

#include "ngram.h"
#include <assert.h>
#include <errno.h>
#include <kchashdb.h>
#include <kcstashdb.h>
#include "kyotodb_utils.h"


/* Use DB::visitor to get_all_items. */

using namespace pinyin;
using namespace kyotocabinet;


Bigram::Bigram(){
	m_db = NULL;
}

Bigram::~Bigram(){
	reset();
}

void Bigram::reset(){
    if ( m_db ){
        m_db->synchronize();
        m_db->close();
        delete m_db;
        m_db = NULL;
    }
}


/* Use StashDB for load_db/save_db methods. */
bool Bigram::load_db(const char * dbfile){
    reset();

    /* create in-memory db. */
    m_db = new StashDB;

    if ( !m_db->open("-", BasicDB::OREADER|BasicDB::OWRITER|BasicDB::OCREATE) )
        return false;

    if (!m_db->load_snapshot(dbfile, NULL))
        return false;

#if 0
    /* load db into memory. */
    BasicDB * tmp_db = new HashDB;
    if (!tmp_db->open(dbfile, BasicDB::OREADER))
        return false;

    CopyVisitor visitor(m_db);
    tmp_db->iterate(&visitor, false);

    tmp_db->close();
    delete tmp_db;
#endif

    return true;
}

bool Bigram::save_db(const char * dbfile){

    int ret = unlink(dbfile);
    if ( ret != 0 && errno != ENOENT)
        return false;

    if (!m_db->dump_snapshot(dbfile, NULL))
        return false;

#if 0
    BasicDB * tmp_db = new HashDB;

    if ( !tmp_db->open(dbfile, BasicDB::OWRITER|BasicDB::OCREATE) )
        return false;

    CopyVisitor visitor(tmp_db);
    m_db->iterate(&visitor, false);

    tmp_db->synchronize();
    tmp_db->close();
    delete tmp_db;
#endif

    return true;
}

bool Bigram::attach(const char * dbfile, guint32 flags){
    reset();
    uint32_t mode = attach_options(flags);

    if (!dbfile)
        return false;

    m_db = new HashDB;

    return m_db->open(dbfile, mode);
}

/* Use DB interface, first check, second reserve the memory chunk,
   third get value into the chunk. */
bool Bigram::load(phrase_token_t index, SingleGram * & single_gram,
                  bool copy){
    single_gram = NULL;
    if ( !m_db )
        return false;

    const char * kbuf = (char *) &index;
    const int32_t vsiz = m_db->check(kbuf, sizeof(phrase_token_t));
    /* -1 on failure. */
    if (-1 == vsiz)
        return false;

    m_chunk.set_size(vsiz);
    char * vbuf = (char *) m_chunk.begin();
    assert (vsiz == m_db->get(kbuf, sizeof(phrase_token_t),
                              vbuf, vsiz));

    single_gram = new SingleGram(m_chunk.begin(), vsiz, copy);
    return true;
}

bool Bigram::store(phrase_token_t index, SingleGram * single_gram){
    if ( !m_db )
        return false;

    const char * kbuf = (char *) &index;
    char * vbuf = (char *) single_gram->m_chunk.begin();
    size_t vsiz = single_gram->m_chunk.size();
    return m_db->set(kbuf, sizeof(phrase_token_t), vbuf, vsiz);
}

bool Bigram::remove(/* in */ phrase_token_t index){
    if ( !m_db )
        return false;

    const char * kbuf = (char *) &index;
    return m_db->remove(kbuf, sizeof(phrase_token_t));
}

class KeyCollectVisitor : public DB::Visitor {
private:
    GArray * m_items;
public:
    KeyCollectVisitor(GArray * items) {
        m_items = items;
    }

    virtual const char* visit_full(const char* kbuf, size_t ksiz,
                                   const char* vbuf, size_t vsiz, size_t* sp) {
        assert(ksiz == sizeof(phrase_token_t));
        const phrase_token_t * token = (phrase_token_t *) kbuf;
        g_array_append_val(m_items, *token);
        return NOP;
    }

    virtual const char* visit_empty(const char* kbuf, size_t ksiz, size_t* sp) {
        /* assume no empty record. */
        assert (FALSE);
        return NOP;
    }
};

bool Bigram::get_all_items(GArray * items){
    g_array_set_size(items, 0);

    if ( !m_db )
        return false;

    KeyCollectVisitor visitor(items);
    m_db->iterate(&visitor, false);

    return true;
}

/* Note: sync mask_out code with ngram_bdb.cpp. */
bool Bigram::mask_out(phrase_token_t mask, phrase_token_t value){
    GArray * items = g_array_new(FALSE, FALSE, sizeof(phrase_token_t));

    if (!get_all_items(items)) {
        g_array_free(items, TRUE);
        return false;
    }

    for (size_t i = 0; i < items->len; ++i) {
        phrase_token_t index = g_array_index(items, phrase_token_t, i);

        if ((index & mask) == value) {
            assert(remove(index));
            continue;
        }

        SingleGram * gram = NULL;
        assert(load(index, gram));

        int num = gram->mask_out(mask, value);
        if (0 == num) {
            delete gram;
            continue;
        }

        if (0 == gram->get_length()) {
            assert(remove(index));
        } else {
            assert(store(index, gram));
        }

        delete gram;
    }

    g_array_free(items, TRUE);
    return true;
}
