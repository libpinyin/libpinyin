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

#include "pinyin.h"

bool print_k_mixture_model_magic_header(KMixtureModelBigram * bigram){
    KMixtureModelMagicHeader magic_header;
    if ( !bigram->get_magic_header(magic_header) ){
        fprintf(stderr, "no magic header in k mixture model.\n");
        exit(ENODATA);
    }
    printf("\\data model \"k mixture model\" count %d N %d\n",
           magic_header.m_WC, magic_header.m_N);
    return true;
}

bool print_k_mixture_model_array_header(KMixtureModelBigram * bigram){
    printf("\1-gram\n");
    GArray * items = g_array_new(FALSE, FALSE, sizeof(phrase_token_t));
    bigram->get_all_items(items);

    for (size_t i = 0; i < items->len; ++i) {
        phrase_token_t * token = &g_array_index(items, phrase_token_t, i);
        KMixtureModelArrayHeader array_header;
        bigram->get_array_header(*token, array_header);
        
    }

    return true;
}

int main(int argc, char * argv[]){
}
