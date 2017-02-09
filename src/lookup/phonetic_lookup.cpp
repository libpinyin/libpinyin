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

bool dump_max_value(GPtrArray * values){
    if (0 == values->len)
        return false;

    const trellis_value_t * max =
        (const trellis_value_t *) g_ptr_array_index(values, 0);

    for (size_t i = 1; i < values->len; ++i) {
        const trellis_value_t * cur =
            (const trellis_value_t *) g_ptr_array_index(values, i);

        if (cur->m_poss > max->m_poss)
            max = cur;
    }

    printf("max value: %f\n", max->m_poss);

    return true;
}

bool dump_all_values(GPtrArray * values) {
    if (0 == values->len)
        return false;

    printf("values:");
    for (size_t i = 0; i < values->len; ++i) {
        const trellis_value_t * cur =
            (const trellis_value_t *) g_ptr_array_index(values, i);

        printf("%f\t", cur->m_poss);
    }
    printf("\n");

    return true;
}

int ForwardPhoneticConstraints::add_constraint(size_t start, size_t end,
                                               phrase_token_t token) {

    if (end > m_constraints->len)
        return 0;

    for (size_t i = start; i < end; ++i){
        clear_constraint(i);
    }

    /* store one step constraint */
    trellis_constraint_t * constraint = &g_array_index
        (m_constraints, trellis_constraint_t, start);
    constraint->m_type = CONSTRAINT_ONESTEP;
    constraint->m_token = token;
    constraint->m_constraint_step = end;

    /* propagate no search constraint */
    for (size_t i = start + 1; i < end; ++i){
        constraint = &g_array_index(m_constraints, trellis_constraint_t, i);
        constraint->m_type = CONSTRAINT_NOSEARCH;
        constraint->m_constraint_step = start;
    }

    return end - start;
}

bool ForwardPhoneticConstraints::clear_constraint(size_t index) {
    if (index < 0 || index >= m_constraints->len)
        return false;

    trellis_constraint_t * constraint = &g_array_index
        (m_constraints, trellis_constraint_t, index);

    if (NO_CONSTRAINT == constraint->m_type)
        return false;

    if (CONSTRAINT_NOSEARCH == constraint->m_type){
        index = constraint->m_constraint_step;
        constraint = &g_array_index(m_constraints, trellis_constraint_t, index);
    }

    /* now var constraint points to the one step constraint. */
    assert(constraint->m_type == CONSTRAINT_ONESTEP);

    /* phrase_token_t token = constraint->m_token; */
    size_t end = constraint->m_constraint_step;
    for (size_t i = index; i < end; ++i){
        if (i >= m_constraints->len)
            continue;

        constraint = &g_array_index
            (m_constraints, trellis_constraint_t, i);
        constraint->m_type = NO_CONSTRAINT;
    }

    return true;
}

bool ForwardPhoneticConstraints::validate_constraint(PhoneticKeyMatrix * matrix) {
    /* resize m_constraints array first */
    const size_t oldlength = m_constraints->len;
    const size_t newlength = matrix->size();

    if ( newlength > oldlength ){
        g_array_set_size(m_constraints, newlength);

        /* initialize new element */
        for( size_t i = oldlength; i < newlength; ++i){
            trellis_constraint_t * constraint = &g_array_index
                (m_constraints, trellis_constraint_t, i);
            constraint->m_type = NO_CONSTRAINT;
        }

    }else if (newlength < oldlength ){
        /* just shrink it */
        g_array_set_size(m_constraints, newlength);
    }

    GArray * keys = g_array_new(TRUE, TRUE, sizeof(ChewingKey));
    PhraseItem item;
    for (size_t i = 0; i < m_constraints->len; ++i){
        trellis_constraint_t * constraint = &g_array_index
            (m_constraints, trellis_constraint_t, i);

        /* handle one step constraint */
        if ( constraint->m_type == CONSTRAINT_ONESTEP ){

            phrase_token_t token = constraint->m_token;
            m_phrase_index->get_phrase_item(token, item);
            guint32 end = constraint->m_constraint_step;

            /* clear too long constraint */
            if (end >= m_constraints->len){
                clear_constraint(i);
                continue;
            }

            gfloat pinyin_poss = compute_pronunciation_possibility
                (matrix, i, end, keys, item);
            /* clear invalid pinyin */
            if (pinyin_poss < FLT_EPSILON)
                clear_constraint(i);
        }
    }

    g_array_free(keys, TRUE);
    return true;
}


bool ForwardPhoneticConstraints::diff_result(MatchResult best,
                                             MatchResult other){
    bool changed = false;

    assert(best->len == other->len);

    for (size_t pos = 0; pos < other->len; ++pos) {
        phrase_token_t other_token = g_array_index(other, phrase_token_t, pos);

        if (null_token == other_token)
            continue;

        phrase_token_t best_token = g_array_index(best, phrase_token_t, pos);

        /* the same token */
        if (best_token == other_token)
            continue;

        changed = true;

        /* skip the tail node, as not searched in nbest algorithm. */
        size_t next_pos = other->len - 1;
        for (size_t i = pos + 1; i < other->len; ++i) {
            phrase_token_t token = g_array_index(other, phrase_token_t, i);

            if (null_token != token) {
                next_pos = i;
                break;
            }
        }

        assert(add_constraint(pos, next_pos, other_token));
    }

    return changed;
}

};
