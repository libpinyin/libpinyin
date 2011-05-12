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
#include "k_mixture_model.h"

void print_help(){
    printf("merge_k_mixture_model <RESULT_FILENAME> {<SOURCE_FILENAME>}+\n");
}


bool merge_two_phrase_array( /* in */  FlexibleBigramPhraseArray first,
                             /* in */  FlexibleBigramPhraseArray second,
                             /* out */ FlexibleBigramPhraseArray & merged ){
    merged = NULL;
    /* both arrays are empty. */
    if ( !(first || second) )
        return false;

    /* only one array contains items. */
    if ( !first || !second ){
        if ( first )
            merged = first;
        if (second )
            merged = second;
        return true;
    }

    merged = g_array_new(FALSE, FALSE, sizeof(KMixtureModelArrayItemWithToken));

    /* merge two arrays. */
    guint first_index, second_index = first_index = 0;
    KMixtureModelArrayItemWithToken * first_item,
        * second_item = first_item = NULL;
    while ( first_index < first->len && second_index < second->len ){
        first_item = &g_array_index(first, KMixtureModelArrayItemWithToken,
                                    first_index);
        second_item = &g_array_index(second, KMixtureModelArrayItemWithToken,
                                     second_index);
        if ( first_item->m_token > second_item->m_token ) {
            g_array_append_val(merged, *second_item);
            second_index ++;
        } else if ( first_item->m_token < second_item->m_token ) {
            g_array_append_val(merged, *first_item);
            first_index ++;
        } else /* first_item->m_token == second_item->m_token */ {
            KMixtureModelArrayItemWithToken merged_item;
            memset(&merged_item, 0, sizeof(KMixtureModelArrayItemWithToken));
            merged_item.m_token = first_item->m_token;/* same as second_item */
            merged_item.m_item.m_WC = first_item->m_item.m_WC +
                second_item->m_item.m_WC;
            /* merged_item.m_item.m_T = first_item->m_item.m_T +
                   second_item->m_item.m_T; */
            merged_item.m_item.m_N_n_0 = first_item->m_item.m_N_n_0 +
                second_item->m_item.m_N_n_0;
            merged_item.m_item.m_n_1 = first_item->m_item.m_n_1 +
                second_item->m_item.m_n_1;
            merged_item.m_item.m_Mr = std_lite::max(first_item->m_item.m_Mr,
                                                    second_item->m_item.m_Mr);
            g_array_append_val(merged, merged_item);
        }
    }

    /* add remained items. */
    while ( first_index < first->len ){
        first_item = &g_array_index(first, KMixtureModelArrayItemWithToken,
                                    first_index);
        g_array_append_val(merged, *first_item);
        first_index++;
    }

    while ( second_index < second->len ){
        second_item = &g_array_index(second, KMixtureModelArrayItemWithToken,
                                     second_index);
        g_array_append_val(merged, *second_item);
        second_index++;
    }

    return true;
}


int main(int argc, char * argv[]){
    const char * result_filename = NULL;
    return 0;
}
