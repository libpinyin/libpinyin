/* 
 *  libpinyin
 *  Library to deal with pinyin.
 *  
 *  Copyright (C) 2012 Peng Wu <alexepico@gmail.com>
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

#include "pinyin_lookup2.h"
#include "stl_lite.h"

using namespace pinyin;

/* internal definition */
static const size_t nbeam = 32;

/* populate the candidates. */
static bool populate_candidates(/* out */ GPtrArray * candidates,
                                /* in */ LookupStepContent step) {
    g_ptr_array_set_size(candidates, 0);

    if (0 == step->len)
        return false;

    for (size_t i = 0; i < step->len; ++i) {
        lookup_value_t * value = &g_array_index
            (step, lookup_value_t, i);

        g_ptr_array_add(candidates, value);
    }

    return true;
}

static bool lookup_value_less_than(lookup_value_t * lhs, lookup_value_t * rhs){
    return lhs->m_poss < rhs->m_poss;
}

/* use maximum heap to get the topest results. */
static bool get_top_results(/* out */ GPtrArray * topresults,
                            /* in */ GPtrArray * candidates) {
    g_ptr_array_set_size(topresults, 0);

    if (0 == candidates->len)
        return false;

    lookup_value_t ** begin =
        (lookup_value_t **) &g_ptr_array_index(candidates, 0);
    lookup_value_t ** end =
        (lookup_value_t **) &g_ptr_array_index(candidates, candidates->len);

    std_lite::make_heap(begin, end, lookup_value_less_than);

    while (end != begin) {
        lookup_value_t * one = *begin;
        g_ptr_array_add(topresults, one);

        std_lite::pop_heap(begin, end, lookup_value_less_than);
        --end;

        if (topresults->len >= nbeam)
            break;
    }

    return true;
}

