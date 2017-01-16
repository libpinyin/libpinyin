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

#include "phonetic_lookup.h"

namespace pinyin{

/* use maximum heap to get the topest results. */
static bool get_top_results(/* out */ GPtrArray * topresults,
                            /* in */ GPtrArray * candidates);


int ForwardPhoneticConstraints::add_constraint(size_t start, size_t end,
                                               phrase_token_t token) {

    if (end > constraints->len)
        return 0;

    for (size_t i = start; i < end; ++i){
        clear_constraint(constraints, i);
    }

    /* store one step constraint */
    lookup_constraint_t * constraint = &g_array_index
        (constraints, lookup_constraint_t, start);
    constraint->m_type = CONSTRAINT_ONESTEP;
    constraint->m_token = token;
    constraint->m_end = end;

    /* propagate no search constraint */
    for (size_t i = start + 1; i < end; ++i){
        constraint = &g_array_index(constraints, lookup_constraint_t, i);
        constraint->m_type = CONSTRAINT_NOSEARCH;
        constraint->m_constraint_step = start;
    }

    return end - start;
}

bool ForwardPhoneticConstraints::clear_constraint(size_t index) {
    if (index < 0 || index >= constraints->len)
        return false;

    lookup_constraint_t * constraint = &g_array_index
        (constraints, lookup_constraint_t, index);

    if (NO_CONSTRAINT == constraint->m_type)
        return false;

    if (CONSTRAINT_NOSEARCH == constraint->m_type){
        index = constraint->m_constraint_step;
        constraint = &g_array_index(constraints, lookup_constraint_t, index);
    }

    /* now var constraint points to the one step constraint. */
    assert(constraint->m_type == CONSTRAINT_ONESTEP);

    /* phrase_token_t token = constraint->m_token; */
    size_t end = constraint->m_end;
    for (size_t i = index; i < end; ++i){
        if (i >= constraints->len)
            continue;

        constraint = &g_array_index
            (constraints, lookup_constraint_t, i);
        constraint->m_type = NO_CONSTRAINT;
    }

    return true;
}

bool ForwardPhoneticConstraints::validate_constraint(PhoneticKeyMatrix * matrix) {
    /* resize constraints array first */
    const size_t oldlength = constraints->len;
    const size_t newlength = matrix->size();

    if ( newlength > oldlength ){
        g_array_set_size(constraints, newlength);

        /* initialize new element */
        for( size_t i = oldlength; i < newlength; ++i){
            lookup_constraint_t * constraint = &g_array_index
                (constraints, lookup_constraint_t, i);
            constraint->m_type = NO_CONSTRAINT;
        }

    }else if (newlength < oldlength ){
        /* just shrink it */
        g_array_set_size(constraints, newlength);
    }

    GArray * keys = g_array_new(TRUE, TRUE, sizeof(ChewingKey));
    PhraseItem item;
    for (size_t i = 0; i < constraints->len; ++i){
        lookup_constraint_t * constraint = &g_array_index
            (constraints, lookup_constraint_t, i);

        /* handle one step constraint */
        if ( constraint->m_type == CONSTRAINT_ONESTEP ){

            phrase_token_t token = constraint->m_token;
            m_phrase_index->get_phrase_item(token, item);
            guint32 end = constraint->m_end;

            /* clear too long constraint */
            if (end >= constraints->len){
                clear_constraint(constraints, i);
                continue;
            }

            gfloat pinyin_poss = compute_pronunciation_possibility
                (matrix, i, end, keys, item);
            /* clear invalid pinyin */
            if (pinyin_poss < FLT_EPSILON)
                clear_constraint(constraints, i);
        }
    }

    g_array_free(keys, TRUE);
    return true;
}


};
