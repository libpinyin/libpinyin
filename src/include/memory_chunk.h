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

#include <config.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#ifdef HAVE_MMAP
#include <sys/mman.h>
#endif
#include "stl_lite.h"

namespace pinyin{

/*  for unmanaged mode
 *  m_free_func == free, when memory is allocated by malloc
 *  m_free_func == munmap, when memory is allocated by mmap
 *  m_free_func == NULL,
 *  when memory is in small protion of allocated area
 *  m_free_func == other,
 *  malloc then free.
 */

/**
 * MemoryChunk:
 *
 * The utility to manage the memory chunks.
 *
 */

class MemoryChunk{
    typedef void (* free_func_t)(...);
private:
    char * m_data_begin;
    char * m_data_end; //one data pass the end.
    char * m_allocated; //one data pass the end.
    free_func_t m_free_func;
    
private:
    void freemem(){
        if ((free_func_t)free == m_free_func)
            free(m_data_begin);
#ifdef HAVE_MMAP
        else if ((free_func_t)munmap == m_free_func)
            munmap(m_data_begin, capacity());
#endif
        else
            assert(FALSE);
    }


    void reset(){
        if (m_free_func)
            freemem();

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
	if ( m_free_func != (free_func_t)free ) {
	    /* copy on resize */
	    newsize = cursize + extra_size;
	    /* do the copy */
	    char * tmp = (char *) malloc(newsize);
	    assert(tmp);
	    memset(tmp, 0, newsize);
	    memmove(tmp, m_data_begin, cursize);
	    /* free the origin memory */
            if (m_free_func)
                freemem();
	    /* change varibles */
	    m_data_begin = tmp;
	    m_data_end = m_data_begin + cursize;
	    m_allocated = m_data_begin + newsize;
	    m_free_func = (free_func_t)free;
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
    /**
     * MemoryChunk::MemoryChunk:
     *
     * The constructor of the MemoryChunk.
     *
     */
    MemoryChunk(){
	m_data_begin = NULL;
	m_data_end = NULL;
	m_allocated = NULL;
	m_free_func = NULL;
    }
    
    /**
     * MemoryChunk::~MemoryChunk:
     *
     * The destructor of the MemoryChunk.
     *
     */
    ~MemoryChunk(){
	reset();
    }

    /**
     * MemoryChunk::begin:
     *
     * Read access method, to get the begin of the MemoryChunk.
     *
     */
    void* begin() const{
	return m_data_begin;
    }

    /**
     * MemoryChunk::end:
     *
     * Write access method, to get the end of the MemoryChunk.
     *
     */
    void* end() const{
        return m_data_end;
    }

    /**
     * MemoryChunk::size:
     *
     * Get the size of the content in the MemoryChunk.
     *
     */
    size_t size() const{
	return m_data_end - m_data_begin;
    }

    /**
     * MemoryChunk::set_size:
     *
     * Set the size of the content in the MemoryChunk.
     *
     */
    void set_size(size_t newsize){
	ensure_has_space(newsize);
	m_data_end = m_data_begin + newsize;
    }

    /**
     * MemoryChunk::capacity:
     *
     * Get the capacity of the MemoryChunk.
     *
     */
    size_t capacity(){
	return m_allocated - m_data_begin;
    }
  
    /**
     * MemoryChunk::set_chunk:
     * @begin: the begin of the data
     * @length: the length of the data
     * @free_func: the function to free the data
     *
     * Transfer management of a memory chunk allocated by other part of the
     * system to the memory chunk.
     *
     */
    void set_chunk(void* begin, size_t length, free_func_t free_func){
	if (m_free_func)
            freemem();
	
	m_data_begin = (char *) begin;
	m_data_end = (char *) m_data_begin + length;
	m_allocated = (char *) m_data_begin + length;
	m_free_func = free_func;
    }
  
    /**
     * MemoryChunk::get_sub_chunk:
     * @offset: the offset in this MemoryChunk.
     * @length: the data length to be retrieved.
     * @returns: the newly allocated MemoryChunk.
     *
     * Get a sub MemoryChunk from this MemoryChunk.
     *
     * Note: use set_chunk internally.
     * the returned new chunk need to be deleted.
     *
     */
    MemoryChunk * get_sub_chunk(size_t offset, size_t length){
	MemoryChunk * retval = new MemoryChunk();
	char * begin_pos = m_data_begin + offset;
	retval->set_chunk(begin_pos, length, NULL);
	return retval;
    }

    /**
     * MemoryChunk::set_content:
     * @offset: the offset in this MemoryChunk.
     * @data: the begin of the data to be copied.
     * @len: the length of the data to be copied.
     * @returns: whether the data is copied successfully.
     *
     * Data are written directly to the memory area in this MemoryChunk.
     *
     */
    bool set_content(size_t offset, const void * data, size_t len){
	size_t cursize = std_lite::max(size(), offset + len);
	ensure_has_space(offset + len);
	memmove(m_data_begin + offset, data, len);
	m_data_end = m_data_begin + cursize;
	return true;
    }

    /**
     * MemoryChunk::append_content:
     * @data: the begin of the data to be copied.
     * @len: the length of the data to be copied.
     * @returns: whether the data is appended successfully.
     *
     * Data are appended at the end of the MemoryChunk.
     *
     */
    bool append_content(const void * data, size_t len){
        return set_content(size(), data, len);
    }

    /**
     * MemoryChunk::insert_content:
     * @offset: the offset in this MemoryChunk, which starts from zero.
     * @data: the begin of the data to be copied.
     * @length: the length of the data to be copied.
     * @returns: whether the data is inserted successfully.
     *
     * Data are written to the memory area,
     * the original content are moved towards the rear.
     *
     */
    bool insert_content(size_t offset, const void * data, size_t length){
	ensure_has_more_space(length);
	size_t move_size = size() - offset;
	memmove(m_data_begin + offset + length, m_data_begin + offset, move_size);
	memmove(m_data_begin + offset, data, length);
	m_data_end += length;
	return true;
    }

    /**
     * MemoryChunk::remove_content:
     * @offset: the offset in this MemoryChunk.
     * @length: the length of the removed content.
     * @returns: whether the content is removed successfully.
     *
     * Data are removed directly,
     * the following content are moved towards the front.
     *
     */
    bool remove_content(size_t offset, size_t length){
	size_t move_size = size() - offset - length;
	memmove(m_data_begin + offset, m_data_begin + offset + length, move_size);
	m_data_end -= length;
	return true;
    }

    /**
     * MemoryChunk::get_content:
     * @offset: the offset in this MemoryChunk.
     * @buffer: the buffer to retrieve the content.
     * @length: the length of content to be retrieved.
     * @returns: whether the content is retrieved.
     *
     * Get the content in this MemoryChunk.
     *
     */
    bool get_content(size_t offset, void * buffer, size_t length){
	if ( size() < offset + length )
	    return false;
	memcpy( buffer, m_data_begin + offset, length);
	return true;
    }

    /**
     * MemoryChunk::compact_memory:
     *
     * Compact memory, reduce the size.
     *
     */
    void compact_memory(){
	if ( m_free_func != (free_func_t)free )
	    return;
	size_t newsize = size();
	m_data_begin = (char *) realloc(m_data_begin, newsize);
	m_allocated = m_data_begin + newsize;
    }
  
    /**
     * MemoryChunk::load:
     * @filename: load the MemoryChunk from the filename.
     * @returns: whether the load is successful.
     *
     * Load the content from the filename.
     *
     */
    bool load(const char * filename){
	/* free old data */
	reset();

	int fd = open(filename, O_RDONLY);
	if (-1 == fd)
	    return false;

        off_t file_size = lseek(fd, 0, SEEK_END);
        lseek(fd, 0, SEEK_SET);

	int data_len = file_size;

#ifdef HAVE_MMAP
        void* data = mmap(NULL, data_len, PROT_READ|PROT_WRITE, MAP_PRIVATE,
                          fd, 0);

        if (MAP_FAILED == data) {
            close(fd);
            return false;
        }

        set_chunk(data, data_len, (free_func_t)munmap);
#else
	void* data = malloc(data_len);
	if ( !data ){
	    close(fd);
	    return false;
	}

	data_len = read(fd, data, data_len);
	set_chunk(data, data_len, (free_func_t)free);
#endif

	close(fd);
	return true;
    }

    /**
     * MemoryChunk::save:
     * @filename: save this MemoryChunk to the filename.
     * @returns: whether the save is successful.
     *
     * Save the content to the filename.
     *
     */
    bool save(const char * filename){
	int fd = open(filename, O_CREAT|O_WRONLY|O_TRUNC, 0644);
	if ( -1 == fd )
	    return false;

	size_t data_len = write(fd, begin(), size());
	if ( data_len != size()){
	    close(fd);
	    return false;
	}

	fsync(fd);
	close(fd);
	return true;
    }
};

};

#endif
