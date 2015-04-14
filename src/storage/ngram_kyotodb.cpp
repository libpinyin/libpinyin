/* 
 *  libpinyin
 *  Library to deal with pinyin.
 *  
 *  Copyright (C) 2013 Peng Wu <alexepico@gmail.com>
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

#include "ngram.h"
#include <assert.h>
#include <errno.h>
#include <kchashdb.h>
#include <kcprotodb.h>


/* Use DB interface, first check, second reserve the memory chunk,
   third get value into the chunk. */

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
    }
}

class CopyVisitor : public DB::Visitor {
private:
    BasicDB * m_db;
public:
    CopyVisitor(BasicDB * db) {
        m_db = db;
    }

    virtual const char* visit_full(const char* kbuf, size_t ksiz,
                                   const char* vbuf, size_t vsiz, size_t* sp) {
        m_db->set(kbuf, ksiz, vbuf, vsiz);
        return NOP;
    }

    virtual const char* visit_empty(const char* kbuf, size_t ksiz, size_t* sp) {
        /* assume no empty record. */
        assert (FALSE);
        return NOP;
    }
};

/* Use ProtoHashDB for load_db/save_db methods. */
bool Bigram::load_db(const char * dbfile){
    reset();

    /* create on-memory db. */
    m_db = new ProtoHashDB;

    /* load db into memory. */
    BasicDB * tmp_db = new HashDB;
    tmp_db->open(dbfile, BasicDB::OREADER);

    CopyVisitor visitor(m_db);
    tmp_db->iterate(&visitor, false);

    if (tmp_db != NULL)
        tmp_db->close();

    return true;
}

bool Bigram::save_db(const char * dbfile){

    int ret = unlink(dbfile);
    if ( ret != 0 && errno != ENOENT)
        return false;

    BasicDB * tmp_db = new HashDB;

    if ( !tmp_db->open(dbfile, BasicDB::OWRITER|BasicDB::OCREATE) )
        return false;

    CopyVisitor visitor(tmp_db);
    m_db->iterate(&visitor, false);

    if (tmp_db != NULL) {
        tmp_db->synchronize();
        tmp_db->close();
    }

    return true;
}

bool Bigram::attach(const char * dbfile, guint32 flags){
    reset();
    uint32_t mode = 0;

    if (flags & ATTACH_READONLY)
        mode |= BasicDB::OREADER;
    if (flags & ATTACH_READWRITE) {
        assert( !( flags & ATTACH_READONLY ) );
        mode |= BasicDB::OREADER | BasicDB::OWRITER;
    }
    if (flags & ATTACH_CREATE)
        mode |= BasicDB::OCREATE;

    if (!dbfile)
        return false;

    m_db = new HashDB;

    return m_db->open(dbfile, mode);
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
