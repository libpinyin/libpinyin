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

#ifndef MEMORY_CHUNK_H
#define MEMORY_CHUNK_H

#include <assert.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include "stl_lite.h"

namespace pinyin{

/*  for unmanaged mode
 *  m_free_func == free , when memory is allocated by malloc
 *  m_free_func == NULL,
 *  when memory is in small protion of allocated area
 *  m_free_func == other,
 *  malloc then free.
 */

class MemoryChunk{
    typedef   void (* free_func_t)(void *);
private:
    char * m_data_begin;
    char * m_data_end; //one data pass the end.
    char * m_allocated; //one data pass the end.
    free_func_t m_free_func;
    
private:
    void reset(){
	if ( m_free_func )
	    (*m_free_func)(m_data_begin);
	m_data_begin = NULL;
	m_data_end = NULL;
	m_allocated = NULL;
	m_free_func = NULL;
    }
    
    void ensure_has_space(size_t new_size){
	int delta_size = m_data_begin + new_size - m_data_end;
	if ( delta_size <= 0 ) return;
	ensure_has_more_space ( delta_size );
    }
    
    /* enlarge function */
    void ensure_has_more_space(size_t extra_size){
	if ( 0 == extra_size ) return;
	size_t newsize;
	size_t cursize = size();
	if ( m_free_func != free ) {
	    /* copy on resize */
	    newsize = cursize + extra_size;
	    /* do the copy */
	    char * tmp = (char *) malloc(newsize);
	    assert(tmp);
	    memset(tmp, 0, newsize);
	    memmove(tmp, m_data_begin, cursize);
	    /* free the origin memory */
	    if ( m_free_func){
		(*m_free_func)(m_data_begin);
	    }
	    
	    /* change varibles */
	    m_data_begin = tmp;
	    m_data_end = m_data_begin + cursize;
	    m_allocated = m_data_begin + newsize;
	    m_free_func = free;
	    return;
	}
	/* the memory area is managed by this memory chunk */
	if ( extra_size <= (size_t) (m_allocated - m_data_end))
	    return;
	newsize = std_lite::max( capacity()<<1, cursize + extra_size);
	m_data_begin = (char *) realloc(m_data_begin, newsize);
	assert(m_data_begin);
	memset(m_data_begin + cursize, 0, newsize - cursize);
	m_data_end = m_data_begin + cursize;
	m_allocated = m_data_begin + newsize;
	return;
    }
    
public:
    /* constructors */
    MemoryChunk(){
	m_data_begin = NULL;
	m_data_end = NULL;
	m_allocated = NULL;
	m_free_func = NULL;
    }
    
    /* destructors */
    ~MemoryChunk(){
	reset();
    }

    /* read access method */
    void* begin() const{
	return m_data_begin;
    }
    
    void* end() const{
        return m_data_end;
    }

    size_t size() const{
	return m_data_end - m_data_begin;
    }
    
    void set_size(size_t newsize){
	ensure_has_space(newsize);
	m_data_end = m_data_begin + newsize;
    }
    
    size_t capacity(){
	return m_allocated - m_data_begin;
    }
  
    /* 
     *  Transfer management of a memory chunk allocated by other part system
     *  to the memory chunk.
     */
    void set_chunk(void* begin, size_t length, free_func_t free_func){
	if ( m_free_func )
	    m_free_func( m_data_begin );
	
	m_data_begin = (char *) begin;
	m_data_end = (char *) m_data_begin + length;
	m_allocated = (char *) m_data_begin + length;
	m_free_func = free_func;
    }
  
    /* subchunk
     * use set_buffer internally.
     * new chunk need to be deleted.
     */
    MemoryChunk * get_sub_chunk(size_t offset, size_t length){
	MemoryChunk * retval = new MemoryChunk();
	char * begin_pos = m_data_begin + offset;
	retval->set_chunk(begin_pos, length, NULL);
	return retval;
    }
    /* write function
     * Data are written directly to the memory area.
     */
    bool set_content(size_t offset, const void * data, size_t len){
	size_t cursize = std_lite::max(size(), offset + len);
	ensure_has_space(offset + len);
	memmove(m_data_begin + offset, data, len);
	m_data_end = m_data_begin + cursize;
	return true;
    }
    /* append function
     * Data are appended at the end.
     */
    bool append_content(const void * data, size_t len){
        return set_content(size(), data, len);
    }
    /* insert function
     * Data are written to the memory area,
     * the original content are moved towards the rear.
     * parameter offset start from zero.
     */
    bool insert_content(size_t offset, const void * data, size_t length){
	ensure_has_more_space(length);
	size_t move_size = size() - offset;
	memmove(m_data_begin + offset + length, m_data_begin + offset, move_size);
	memmove(m_data_begin + offset, data, length);
	m_data_end += length;
	return true;
    }
    /* remove function
     * Data are removed directly,
     * the following content are moved towards the front.
     */
    bool remove_content(size_t offset, size_t length){
	size_t move_size = size() - offset - length;
	memmove(m_data_begin + offset, m_data_begin + offset + length, move_size);
	m_data_end -= length;
	return true;
    }

    /* get_content function
     * Get the binary data
     */
    bool get_content(size_t offset, void * buffer, size_t length){
	if ( size() < offset + length )
	    return false;
	memcpy( buffer, m_data_begin + offset, length);
	return true;
    }

    /* compact memory, reduce the size */
    void compact_memory(){
	if ( m_free_func != free )
	    return;
	size_t newsize = size();
	m_data_begin = (char *) realloc(m_data_begin, newsize);
	m_allocated = m_data_begin + newsize;
    }
  
    /* file storage  functions */
    bool load(const char * filename){
	/* free old data */
	reset();

        size_t file_size;
	
	FILE* file = fopen(filename, "r");
	if ( !file )
	    return false;

        fseek(file, 0, SEEK_END);
        file_size = ftell(file);
        fseek(file, 0, SEEK_SET);

	int data_len = file_size;
	void* data = malloc(data_len);
	if ( !data ){
	    fclose(file);
	    return false;      
	}
	
	data_len = fread(data, 1, data_len, file);
	set_chunk(data, data_len, free);

	fclose(file);
	return true;
    }

    bool save(const char * filename){
	FILE* file = fopen(filename, "w");
	if ( !file )
	    return false;

	size_t data_len = fwrite(begin(), 1, size(), file);
	if ( data_len != size()){
	    fclose(file);
	    return false;
	}

	fsync(fileno(file));
	fclose(file);
	return true;
    }
};

};

#endif
