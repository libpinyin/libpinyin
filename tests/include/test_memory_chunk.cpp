#include <stdio.h>
#include <iostream>
#include "memory_chunk.h"
// Test Memory Chunk Functionality

int main(int argc, char * argv[]){
  MemoryChunk* chunk;
  chunk = new MemoryChunk();
  int i = 12;
  chunk->set_content(0, &i, sizeof(int));

  int * p = (int *)chunk->begin();
  assert(chunk->size() == sizeof(int));
  std::cout<<*p<<std::endl;
  std::cout<<chunk->capacity()<<std::endl;
  p = & i;
  chunk->set_chunk(p, sizeof(int), NULL);
  short t = 5;
  chunk->set_content(sizeof(int), &t, sizeof(short));
  assert( sizeof(int) + sizeof(short) == chunk->size());
  std::cout<<chunk->capacity()<<std::endl;

  p = (int *)chunk->begin();
  short * p2 =(short *)(((char *) (chunk->begin())) + sizeof(int));
  std::cout<<*p<<'\t'<<*p2<<std::endl;

  chunk->set_content(sizeof(int) + sizeof(short), &t, sizeof(short));
  
  assert( sizeof(int) + (sizeof(short) << 1) == chunk->size());
  std::cout<<chunk->capacity()<<std::endl;
  p = (int *)chunk->begin();
  p2 =(short *)(((char *) (chunk->begin())) + sizeof(int));
  std::cout<<*p<<'\t'<<*p2<<'\t'<<*(p2 + 1)<<std::endl;

  chunk->set_size(sizeof(int) + sizeof(short) *3);
  p = (int *)chunk->begin();
  p2 =(short *)(((char *) (chunk->begin())) + sizeof(int));

  chunk->set_content(0, &i, sizeof(int));

  *(p2+2) = 3;
  std::cout<<*p<<'\t'<<*p2<<'\t'<<*(p2 + 1)<<'\t'<<*(p2+2)<<std::endl;

  int m = 10;
  chunk->set_chunk(&m, sizeof(int), NULL);
  int n = 12;
  chunk->insert_content(sizeof(int), &n, sizeof(int));
  n = 11;
  chunk->insert_content(sizeof(int), &n, sizeof(int));

  int * p3 = (int *)chunk->begin();
  std::cout<<*p3<<'\t'<<*(p3+1)<<'\t'<<*(p3+2)<<std::endl;
	
  chunk->remove_content(sizeof(int), sizeof(int));
  std::cout<<*p3<<'\t'<<*(p3+1)<<std::endl;

  int tmp;
  assert(chunk->get_content(sizeof(int), &tmp, sizeof(int)));
  std::cout<<tmp<<std::endl;
  
  
  delete chunk;

  const char * filename =  "/tmp/version";
  const char * version = "0.2.0";

  chunk =  new MemoryChunk;
  bool retval = chunk->load(filename);
  if ( !retval ){
      std::cerr<<"can't find chunk"<<std::endl;
  }else{
      if ( memcmp(version, chunk->begin(), strlen(version) + 1) == 0){
	  std::cout<<"match"<<std::endl;
      }

  }

  chunk->set_content(0, version, strlen(version) + 1);
  chunk->save(filename);

  retval = chunk->load(filename);
  if ( !retval ){
      std::cerr<<"can't find chunk"<<std::endl;
  }
  if ( memcmp(version, chunk->begin(), strlen(version) + 1) == 0){
      std::cout<<"match"<<std::endl;
  }

  return 0;
}
