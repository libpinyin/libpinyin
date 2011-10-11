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

#ifndef LOOKUP_WINNER_TREE_H
#define LOOKUP_WINNER_TREE_H

#include <assert.h>

namespace pinyin{

const int nbranch = 32;

class DirectBranchIterator: public IBranchIterator{//for nitem <= nbranch
    LookupStepContent m_step_content;
    size_t m_iter_pos;
public:
    //Constructor
    DirectBranchIterator(LookupStepContent step_content)
	:m_step_content(step_content)
    { m_iter_pos = 0; }
    
    //Destructor
    virtual ~DirectBranchIterator(){}
    
    //Member Function
    bool has_next(){
	return m_iter_pos != m_step_content->len;
    }
    
    lookup_value_t next(){
	lookup_value_t * tmp = &g_array_index(m_step_content, 
					      lookup_value_t, m_iter_pos);
	++m_iter_pos;
	return *tmp;
    }
    
    lookup_value_t max(){
	lookup_value_t * max_value = &g_array_index(m_step_content, lookup_value_t, 0);
	for ( size_t i = 1 ; i < m_step_content->len; ++i){
	    lookup_value_t * cur_value = &g_array_index(m_step_content, lookup_value_t, i);
	    if ( cur_value->m_poss > max_value->m_poss )
		max_value = cur_value;
	}
	return *max_value;
    }
};

class WinnerTree;

class WinnerTreeBranchIterator: public IBranchIterator{//for nitem <= nbranch
    WinnerTree& m_tree;
    int m_counter;
    lookup_value_t m_max_value;
public:
    //Constructor
    WinnerTreeBranchIterator(WinnerTree & tree);
    
    //Destructor
    virtual ~WinnerTreeBranchIterator(){}
  
    //Member Function
    bool has_next();
    
    lookup_value_t next();
    
    lookup_value_t max(){
	return m_max_value;
    }
    
};

class WinnerTree{
    friend class WinnerTreeBranchIterator;
private:
    size_t m_max_tree_size; // maxsize
    int m_tree_size; // n
    int m_low_ext;
    int m_offset;
    int * m_tree; 
    MemoryChunk m_buffer;
    MemoryChunk m_tree_buffer;
    lookup_value_t * m_items;

    int winner(int lc, int rc);
    
    void play(int p, int lc, int rc);
    
    void init(int tree_size){
	m_max_tree_size = tree_size;
	//data buffer
	m_buffer.set_size( sizeof(lookup_value_t) * (tree_size + 1) );
	m_items = (lookup_value_t *) m_buffer.begin();
	
	//tree item buffer
	m_tree_buffer.set_size( sizeof(int) * m_max_tree_size);
	m_tree = (int * ) m_tree_buffer.begin();
	m_tree_size = 0;
    }
    
public:
    
    //Constructor
    WinnerTree(int tree_size = 10){
	init(tree_size);
    }
    
    //Destructor
    ~WinnerTree() { }

    //need delete this
    IBranchIterator* get_iterator(LookupStepContent step){
	if ( step->len <= nbranch )
	    return new DirectBranchIterator(step);
	//TODO:another situation > nbranch
	assert(initialize(step));
	return new WinnerTreeBranchIterator(*this);
    }
    
protected:
    
    int get_winner() const {
	return (m_tree_size)? m_tree[1] : 0;
    }
    
    //Member Function
    bool initialize(LookupStepContent cur_step);
    void replay(int i);
};

};
#endif
