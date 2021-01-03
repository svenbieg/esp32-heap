//=============
// mem_block.c
//=============

// Copyright 2021, Sven Bieg (svenbieg@web.de)
// http://github.com/svenbieg/esp32-heap


//=======
// Using
//=======

#include <string.h>
#include "mem_block.h"
#include "multi_heap_internal.h"
#include "multi_heap_platform.h"


//========
// Common
//========

size_t mem_block_calc_size(size_t size)
{
return multi_heap_align_up(size, 4)+2*sizeof(size_t);
}

void* mem_block_init(multi_heap_handle_t heap, size_t offset, size_t size, size_t flags)
{
size_t heap_start=(size_t)heap+sizeof(multi_heap_t);
size_t heap_end=heap_start+heap->size;
if(offset<heap_start||offset>heap_end)
	return NULL;
if(offset+size>heap_end+size)
	return NULL;
size_t entry=size&MEM_BLOCK_SIZE_MASK;
entry|=flags;
size_t* head=(size_t*)offset;
*head=entry;
head++;
size_t* foot=(size_t*)(offset+size);
foot--;
*foot=entry;
return head;
}

bool mem_block_get_neighbours(multi_heap_handle_t heap, size_t offset, mem_block_neighbours_t* info)
{
memset(info, 0, sizeof(mem_block_neighbours_t));
if(!mem_block_get_info(heap, offset, &info->cur))
	return false;
size_t heap_start=(size_t)heap+sizeof(multi_heap_t);
if(offset>heap_start)
	{
	size_t* foot=(size_t*)offset;
	foot--;
	size_t foot_entry=*foot;
	size_t size=foot_entry&MEM_BLOCK_SIZE_MASK;
	size_t prev=offset-size;
	if(!mem_block_get_info(heap, prev, &info->prev))
		return false;
	}
size_t next=offset+info->cur.size;
if(next<heap_start+heap->size)
	{
	if(!mem_block_get_info(heap, next, &info->next))
		return false;
	}
return true;
}

bool mem_block_get_info(multi_heap_handle_t heap, size_t offset, mem_block_info_t* info)
{
memset(info, 0, sizeof(mem_block_info_t));
size_t heap_start=(size_t)heap+sizeof(multi_heap_t);
if(offset<heap_start||offset>=heap_start+heap->size)
	return false;
size_t* head=(size_t*)offset;
size_t entry=*head;
size_t flags=entry&MEM_BLOCK_FLAGS_MASK;
size_t size=entry&MEM_BLOCK_SIZE_MASK;
if(size<3*sizeof(size_t)||size>heap->size)
	return false;
size_t* foot=(size_t*)(offset+size);
foot--;
size_t foot_entry=*foot;
if(entry!=foot_entry)
	return false;
info->flags=flags;
info->pos=offset;
info->size=size;
return true;
}

void* mem_block_get_pointer(size_t offset)
{
return (void*)(offset+sizeof(size_t));
}

size_t mem_block_get_offset(void* p)
{
return (size_t)p-sizeof(size_t);
}
