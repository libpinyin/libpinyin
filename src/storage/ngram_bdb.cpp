/* 
 *  libpinyin
 *  Library to deal with pinyin.
 *  
 *  Copyright (C) 2015 Peng Wu <alexepico@gmail.com>
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

#include <errno.h>
#include <glib.h>
#include "memory_chunk.h"
#include "novel_types.h"
#include "ngram.h"
#include "bdb_utils.h"

using namespace pinyin;


Bigram::Bigram(){
	m_db = NULL;
}

Bigram::~Bigram(){
	reset();
}

void Bigram::reset(){
    if ( m_db ){
        m_db->sync(m_db, 0);
        m_db->close(m_db, 0);
        m_db = NULL;
    }
}

bool Bigram::load_db(const char * dbfile){
    reset();

    /* create in memory db. */
    int ret = db_create(&m_db, NULL, 0);
    assert(ret == 0);

    ret = m_db->open(m_db, NULL, NULL, NULL,
                     DB_HASH, DB_CREATE, 0600);
    if ( ret != 0 )
        return false;

    /* load db into memory. */
    DB * tmp_db = NULL;
    ret = db_create(&tmp_db, NULL, 0);
    assert(0 == ret);

    if (NULL == tmp_db)
        return false;

    ret = tmp_db->open(tmp_db, NULL, dbfile, NULL,
                       DB_HASH, DB_RDONLY, 0600);
    if ( ret != 0 )
        return false;

    if ( !copy_bdb(tmp_db, m_db) )
        return false;

    if ( tmp_db != NULL )
        tmp_db->close(tmp_db, 0);

    return true;
}

bool Bigram::save_db(const char * dbfile){
    DB * tmp_db = NULL;

    int ret = unlink(dbfile);
    if ( ret != 0 && errno != ENOENT)
        return false;

    ret = db_create(&tmp_db, NULL, 0);
    assert(0 == ret);

    if (NULL == tmp_db)
        return false;

    ret = tmp_db->open(tmp_db, NULL, dbfile, NULL,
                       DB_HASH, DB_CREATE, 0600);
    if ( ret != 0 )
        return false;

    if ( !copy_bdb(m_db, tmp_db) )
        return false;

    if ( tmp_db != NULL ) {
        tmp_db->sync(m_db, 0);
        tmp_db->close(tmp_db, 0);
    }

    return true;
}

bool Bigram::attach(const char * dbfile, guint32 flags){
    reset();
    u_int32_t db_flags = attach_options(flags);

    if ( !dbfile )
        return false;
    int ret = db_create(&m_db, NULL, 0);
    assert(0 == ret);
	
    ret = m_db->open(m_db, NULL, dbfile, NULL,
                     DB_HASH, db_flags, 0644);
    if ( ret != 0)
        return false;

    return true;
}

bool Bigram::load(phrase_token_t index, SingleGram * & single_gram,
                  bool copy){
    single_gram = NULL;
    if ( !m_db )
        return false;

    DBT db_key;
    memset(&db_key, 0, sizeof(DBT));
    db_key.data = &index;
    db_key.size = sizeof(phrase_token_t);

    DBT db_data;
    memset(&db_data, 0, sizeof(DBT));
    int ret = m_db->get(m_db, NULL, &db_key, &db_data, 0);
    if ( ret != 0 )
        return false;

    single_gram = new SingleGram(db_data.data, db_data.size, copy);
    return true;
}

bool Bigram::store(phrase_token_t index, SingleGram * single_gram){
    if ( !m_db )
        return false;

    DBT db_key;
    memset(&db_key, 0, sizeof(DBT));
    db_key.data = &index;
    db_key.size = sizeof(phrase_token_t);
    DBT db_data;
    memset(&db_data, 0, sizeof(DBT));
    db_data.data = single_gram->m_chunk.begin();
    db_data.size = single_gram->m_chunk.size();
    
    int ret = m_db->put(m_db, NULL, &db_key, &db_data, 0);
    return ret == 0;
}

bool Bigram::remove(/* in */ phrase_token_t index){
    if ( !m_db )
        return false;

    DBT db_key;
    memset(&db_key, 0, sizeof(DBT));
    db_key.data = &index;
    db_key.size = sizeof(phrase_token_t);

    int ret = m_db->del(m_db, NULL, &db_key, 0);
    return 0 == ret;
}

bool Bigram::get_all_items(GArray * items){
    g_array_set_size(items, 0);

    if ( !m_db )
        return false;

    DBC * cursorp = NULL;
    DBT key, data;
    int ret;
    /* Get a cursor */
    m_db->cursor(m_db, NULL, &cursorp, 0);

    if (NULL == cursorp)
        return false;

    /* Initialize our DBTs. */
    memset(&key, 0, sizeof(DBT));
    memset(&data, 0, sizeof(DBT));
	
    /* Iterate over the database, retrieving each record in turn. */
    while ((ret = cursorp->c_get(cursorp, &key, &data, DB_NEXT)) == 0) {
        assert(key.size == sizeof(phrase_token_t));
        phrase_token_t * token = (phrase_token_t *)key.data;
        g_array_append_val(items, *token);

        /* Initialize our DBTs. */
        memset(&key, 0, sizeof(DBT));
        memset(&data, 0, sizeof(DBT));
    }

    assert (ret == DB_NOTFOUND);

    /* Cursors must be closed */
    if (cursorp != NULL) 
        cursorp->c_close(cursorp); 

    return true;
}

/* Note: sync mask_out code with ngram_kyotodb.cpp. */
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
