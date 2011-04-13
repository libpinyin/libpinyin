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
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */


#ifndef K_MIXTURE_MODEL
#define K_MIXTURE_MODEL

#include "novel_types.h"

namespace pinyin{

typedef guint32 corpus_count_t;

/* Note: storage parameters: N, T, n_r.
 * N: the total number of documents.
 * T: the total number of instances of the word or phrase.
 * n_r: the number of documents having exactly <b>r</b> occurrences.
 *      only n_0, n_1 are used here.
 */

static inline parameter_t compute_alpha(corpus_count_t N, corpus_count_t n_0){
    parameter_t alpha = 1 - n_0 / (parameter_t) N;
    return alpha;
}

static inline parameter_t compute_gamma(corpus_count_t N,
                                        corpus_count_t n_0,
                                        corpus_count_t n_1){
    parameter_t gamma = 1 - n_1 / (parameter_t) (N - n_0);
    return gamma;
}

static inline parameter_t compute_B(corpus_count_t N,
                                    corpus_count_t T,
                                    corpus_count_t n_0,
                                    corpus_count_t n_1){
    parameter_t B = (T - n_1 ) / (parameter_t) (N - n_0 - n_1);
    return B;
}

/* three parameters model */
static inline parameter_t compute_Pr_G_3(corpus_count_t k,
                                         parameter_t alpha,
                                         parameter_t gamma,
                                         parameter_t B){
    if ( k == 0 )
        return 1 - alpha;

    if ( k == 1 )
        return alpha * (1 - gamma);

    if ( k > 1 ) {
        return alpha * gamma / (B - 1) * pow((1 - 1 / (B - 1)) , k - 2);
    }

    assert(false);
}

static inline parameter_t compute_Pr_G_3_with_count(corpus_count_t k,
                                                    corpus_count_t N,
                                                    corpus_count_t T,
                                                    corpus_count_t n_0,
                                                    corpus_count_t n_1){
    parameter_t alpha = compute_alpha(N, n_0);
    parameter_t gamma = compute_gamma(N, n_0, n_1);
    parameter_t B = compute_B(N, T, n_0, n_1);

    return compute_Pr_G_3(k, alpha, gamma, B);
}

/* two parameters model */
static inline parameter_t compute_Pr_G_2(corpus_count_t k,
                                         parameter_t alpha,
                                         parameter_t B){
    parameter_t gamma = 1 - 1 / (B - 1);
    return compute_Pr_G_3(k, alpha, gamma, B);
}

static inline parameter_t compute_Pr_G_2_with_count(corpus_count_t k,
                                                    corpus_count_t N,
                                                    corpus_count_t T,
                                                    corpus_count_t n_0,
                                                    corpus_count_t n_1){
    parameter_t alpha = compute_alpha(N, n_0);
    parameter_t B = compute_B(N, T, n_0, n_1);
    return compute_Pr_G_2(k, alpha, B);
}

};


#endif
