//=============
// mem_block.h
//=============

// Copyright 2021, Sven Bieg (svenbieg@web.de)
// http://github.com/svenbieg/esp32-heap

#pragma once


//=======
// Using
//=======

#include <multi_heap.h>


//=======
// Flags
//=======

#define MEM_BLOCK_FLAG_FREE (size_t)1
#define MEM_BLOCK_FLAG_ALIGNED (size_t)2
#define MEM_BLOCK_FLAGS_MASK (size_t)3
#define MEM_BLOCK_SIZE_MASK ((size_t)~3)


//======
// Info
//======

typedef struct
{
size_t flags;
size_t pos;
size_t size;
}mem_block_info_t;

typedef struct
{
mem_block_info_t cur;
mem_block_info_t next;
mem_block_info_t prev;
}mem_block_neighbours_t;


//========
// Common
//========

size_t mem_block_calc_size(size_t size);
void* mem_block_init(multi_heap_handle_t heap, size_t offset, size_t size, size_t flags);
bool mem_block_get_neighbours(multi_heap_handle_t heap, size_t offset, mem_block_neighbours_t* info);
bool mem_block_get_info(multi_heap_handle_t heap, size_t offset, mem_block_info_t* info);
void* mem_block_get_pointer(size_t offset);
size_t mem_block_get_offset(void* p);
