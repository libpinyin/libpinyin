/* 
 *  libpinyin
 *  Library to deal with pinyin.
 *  
 *  Copyright (C) 2006-2007 Peng Wu
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

#include <float.h>
#include <limits.h>
#include <stdio.h>
#include "memory_chunk.h"
#include "phrase_index.h"
#include "pinyin_lookup.h"
#include "winner_tree.h"

using namespace pinyin;

WinnerTreeBranchIterator::WinnerTreeBranchIterator(WinnerTree & tree)
    :m_tree(tree), m_counter(0){
    m_max_value = m_tree.m_items[m_tree.get_winner()];
    m_counter = 0;
}

bool WinnerTreeBranchIterator::has_next(){
    if ( m_counter >= m_tree.m_tree_size)
	return false;
    return m_counter < nbranch;
}

lookup_value_t WinnerTreeBranchIterator::next(){
    int winner = m_tree.get_winner();
    lookup_value_t tmp = m_tree.m_items[winner];
    m_tree.m_items[winner].m_poss = 
	- FLT_MAX;
    m_tree.replay(winner);
    ++m_counter;
    return tmp;
}

void WinnerTree::play(int p, int lc, int rc){
    m_tree[p] = winner(lc, rc);
    //continue competition
    while( p > 1 && p % 2) {
	m_tree[p/2] = winner( m_tree[p - 1], m_tree[p]);
	p/=2;
  }
}


bool WinnerTree::initialize(LookupStepContent cur_step){
    size_t size = cur_step->len;
    if ( size > m_max_tree_size ){
	init(size);
    }
    assert(size > nbranch);
    m_tree_size = size;
    
    //initialize array tree
    int nindex = 1;
    
    for( size_t i = 0; i < cur_step->len ; ++i){
	lookup_value_t * cur_value = &g_array_index(cur_step, lookup_value_t, i);
	m_items[nindex++] = *cur_value;
    }
    
    //compute s = 2 ^ log(n -1)
    int i, s;
    for( s = 1; 2 * s <= m_tree_size - 1; s += s);
  
    m_low_ext = 2 * (m_tree_size - s);
    m_offset = 2 * s - 1;
  
    //compute outside nodes
    for( i = 2; i <= m_low_ext; i += 2)
	play((m_offset + i)/2, i - 1, i);
    //compute other nodes
    if ( m_tree_size % 2){
	play( m_tree_size / 2, m_tree[m_tree_size - 1], m_low_ext +1);
	i = m_low_ext + 3;
    }else i = m_low_ext + 2;
  
    //compute others 
    for( ; i <= m_tree_size; i += 2)
    play( (i - m_low_ext + m_tree_size - 1) / 2, i - 1, i);
    return true;
}

void WinnerTree::replay(int i){
    assert( 1 <= i && i <= m_tree_size);
    
    int p; //compete node
    int lc; //p's left child
    int rc; //p's right child
    
    //first compete
    if ( i <= m_low_ext){
	p = (m_offset + i) / 2;
	lc = 2 * p - m_offset;
	rc = lc + 1;
    }else{
	p = (i - m_low_ext + m_tree_size -1) / 2;
	if ( 2 * p == m_tree_size - 1 ){
	    lc = m_tree[2*p];
	    rc = i;
	}else{
	    lc = 2 * p - m_tree_size + 1 + m_low_ext;
	    rc = lc + 1;
	}
    }
    
    m_tree[p] = winner(lc, rc);
    
    //added by wupeng
    if ( ( p | 0x01 )  == m_tree_size ){
        p /= 2;
	m_tree[p] = winner( m_tree[2 * p], m_low_ext + 1 );
    }
    
    //compute others
    p /= 2;
    for( ; p >= 1 ; p /= 2)
	m_tree[p] = winner( m_tree[2 * p], m_tree[2 * p + 1]);
}

int WinnerTree::winner(int lc, int rc){
    return m_items[lc].m_poss > m_items[rc].m_poss ? 
    lc : rc;
}
