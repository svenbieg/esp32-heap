// Copyright 2015-2016 Espressif Systems (Shanghai) PTE LTD
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at

//	 http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

// This file was modified
// Copyright 2021, Sven Bieg (svenbieg@web.de)
// http://github.com/svenbieg/esp32-heap


//=======
// Using
//=======

#include <multi_heap.h>
#include <string.h>
#include "mem_block.h"
#include "mem_block_list.h"
#include "multi_heap_internal.h"


//=========
// Private
//=========

// Remove offset from buffer or map
void multi_heap_remove_offset(multi_heap_handle_t heap, mem_block_info_t* info)
{
size_t* offsets=heap->free_offsets;
uint32_t count=heap->free_offset_count;
for(uint32_t pos=0; pos<count; pos++)
	{
	if(offsets[pos]!=info->pos)
		continue;
	for(uint32_t u=pos; u+1<count; u++)
		offsets[u]=offsets[u+1];
	heap->free_offset_count--;
	return;
	}
mem_block_map_remove_offset(heap, &heap->map_free, info->size, info->pos);
}

// Add free offset to buffer
void multi_heap_free_private(multi_heap_handle_t heap, size_t offset)
{
uint32_t count=heap->free_offset_count;
if(count==CONFIG_HEAP_MAX_OFFSETS)
	{
	MULTI_HEAP_PRINTF("CONFIG_HEAP_MAX_OFFSETS\n");
	heap->flags|=MULTI_HEAP_FLAG_DIRTY;
	return;
	}
size_t* offsets=heap->free_offsets;
uint32_t pos=0;
for(; pos<count; pos++)
	{
	if(offset>offsets[pos])
		{
		for(uint32_t u=count; u>pos; u--)
			offsets[u]=offsets[u-1];
		offsets[pos]=offset;
		break;
		}
	}
if(pos==count)
	offsets[count]=offset;
heap->free_offset_count++;
}

// Allocate block at the end of the heap
void* multi_heap_malloc_direct(multi_heap_handle_t heap, size_t block_size)
{
if(heap->total_size-heap->size<block_size)
	return NULL;
size_t heap_start=(size_t)heap+sizeof(multi_heap_t);
size_t heap_end=heap_start+heap->size;
void* p=mem_block_init(heap, heap_end, block_size, 0);
heap->size+=block_size;
heap->free_bytes-=block_size;
if(heap->free_bytes<heap->minimum_free_bytes)
	heap->minimum_free_bytes=heap->free_bytes;
heap->allocated_blocks++;
heap->total_blocks++;
return p;
}

// Allocate free block from map
void* multi_heap_malloc_fit(multi_heap_handle_t heap, size_t block_size)
{
mem_block_map_it_t it;
mem_block_map_it_init(&it, &heap->map_free);
if(mem_block_map_it_find(&it, block_size))
	{
	size_t free_pos=mem_block_map_item_get_offset(it.current);
	mem_block_map_remove_offset(heap, &heap->map_free, block_size, free_pos);
	void* p=mem_block_init(heap, free_pos, block_size, 0);
	heap->free_bytes-=block_size;
	if(heap->free_bytes<heap->minimum_free_bytes)
		heap->minimum_free_bytes=heap->free_bytes;
	heap->allocated_blocks++;
	heap->free_blocks--;
	return p;
	}
size_t over_size=block_size+mem_block_calc_size(1);
mem_block_map_it_find(&it, over_size);
if(!it.current)
	return NULL;
if(it.current->size<over_size)
	mem_block_map_it_move_next(&it);
if(!it.current)
	return NULL;
size_t free_pos=mem_block_map_item_get_offset(it.current);
size_t free_size=it.current->size;
mem_block_map_remove_offset(heap, &heap->map_free, free_size, free_pos);
void* p=mem_block_init(heap, free_pos, block_size, 0);
size_t rest_pos=free_pos+block_size;
size_t rest_size=free_size-block_size;
mem_block_init(heap, rest_pos, rest_size, MEM_BLOCK_FLAG_FREE);
multi_heap_free_private(heap, rest_pos);
heap->free_bytes-=block_size;
if(heap->free_bytes<heap->minimum_free_bytes)
	heap->minimum_free_bytes=heap->free_bytes;
heap->allocated_blocks++;
heap->total_blocks++;
return p;
}

// Allocate free block from buffer
void* multi_heap_malloc_private(multi_heap_handle_t heap, size_t block_size)
{
uint32_t count=heap->free_offset_count;
size_t* offsets=heap->free_offsets;
for(uint32_t pos=count; pos>0; pos--)
	{
	mem_block_info_t info;
	if(!mem_block_get_info(heap, offsets[pos-1], &info))
		continue;
	if(info.size!=block_size)
		continue;
	multi_heap_remove_offset(heap, &info);
	void* p=mem_block_init(heap, info.pos, info.size, 0);
	heap->free_bytes-=info.size;
	if(heap->free_bytes<heap->minimum_free_bytes)
		heap->minimum_free_bytes=heap->free_bytes;
	heap->allocated_blocks++;
	heap->free_blocks--;
	return p;
	}
return NULL;
}

// Add free offsets from buffer to map
void multi_heap_update_map(multi_heap_handle_t heap)
{
// Copy free offsets from buffer
size_t offsets[CONFIG_HEAP_MAX_OFFSETS];
size_t count=heap->free_offset_count;
for(uint32_t pos=0; pos<count; pos++)
	offsets[pos]=heap->free_offsets[pos];
heap->free_offset_count=0;
size_t heap_start=(size_t)heap+sizeof(multi_heap_t);
for(uint32_t pos=0; pos<count; pos++)
	{
	if(offsets[pos]==0)
		continue;
	mem_block_info_t cur;
	if(!mem_block_get_info(heap, offsets[pos], &cur))
		{
		heap->flags|=MULTI_HEAP_FLAG_DIRTY;
		continue;
		}
	// Combine with free blocks in buffer
	for(uint32_t u=pos+1; u<count; u++)
		{
		mem_block_info_t prev;
		if(!mem_block_get_info(heap, offsets[u], &prev))
			{
			offsets[u]=0;
			break;
			}
		if(prev.pos+prev.size<cur.pos)
			break;
		offsets[u]=0;
		cur.pos=prev.pos;
		cur.size+=prev.size;
		heap->free_blocks--;
		heap->total_blocks--;
		}
	mem_block_init(heap, cur.pos, cur.size, MEM_BLOCK_FLAG_FREE);
	// Combine free block with neighbours
	mem_block_neighbours_t info;
	if(!mem_block_get_neighbours(heap, cur.pos, &info))
		{
		heap->flags|=MULTI_HEAP_FLAG_DIRTY;
		continue;
		}
	if(info.prev.flags&MEM_BLOCK_FLAG_FREE)
		{
		multi_heap_remove_offset(heap, &info.prev);
		cur.pos=info.prev.pos;
		cur.size+=info.prev.size;
		heap->free_blocks--;
		heap->total_blocks--;
		}
	if(info.next.flags&MEM_BLOCK_FLAG_FREE)
		{
		multi_heap_remove_offset(heap, &info.next);
		cur.size+=info.next.size;
		heap->free_blocks--;
		heap->total_blocks--;
		}
	// Remove free block from end of heap
	if(cur.pos+cur.size==heap_start+heap->size)
		{
		heap->size-=cur.size;
		heap->free_blocks--;
		heap->total_blocks--;
		continue;
		}
	// Add free block to map
	mem_block_init(heap, cur.pos, cur.size, MEM_BLOCK_FLAG_FREE);
	if(!mem_block_map_add_offset(heap, &heap->map_free, cur.size, cur.pos))
		heap->flags|=MULTI_HEAP_FLAG_DIRTY;
	}
}


//==========
// Internal
//==========

bool multi_heap_check_internal(multi_heap_handle_t heap, bool print_errors)
{
bool success=true;
size_t heap_start=(size_t)heap+sizeof(multi_heap_t);
size_t heap_end=heap_start+heap->size;
size_t pos=heap_start;
bool prev_free=false;
bool free_collide=false;
while(pos<heap_end)
	{
	size_t* head=(size_t*)pos;
	size_t entry=*head;
	size_t size=entry&MEM_BLOCK_SIZE_MASK;
	size_t* foot=(size_t*)(pos+size);
	foot--;
	size_t foot_entry=*foot;
	if(foot_entry!=entry)
		{
		if(print_errors)
			{
			MULTI_HEAP_PRINTF("multi_heap_check_internal(0x%x): entry 0x%x mismatch %u - %u\n", heap, pos, entry, foot_entry);
			}
		success=false;
		break;
		}
	mem_block_info_t info;
	mem_block_get_info(heap, pos, &info);
	if(info.flags&MEM_BLOCK_FLAG_FREE)
		{
		if(prev_free)
			free_collide=true;
		prev_free=true;
		}
	else
		{
		prev_free=false;
		}
	pos+=size;
	}
if(print_errors&&free_collide)
	{
	MULTI_HEAP_PRINTF("multi_heap_check_internal(0x%x): free entries collide\n", heap);
	}
if(!mem_block_map_check(heap, &heap->map_free, print_errors))
	success=false;
return success;
}

void multi_heap_dump_internal(multi_heap_handle_t heap)
{
if(!heap->total_blocks)
	return;
MULTI_HEAP_PRINTF("heap 0x%x:\n", heap);
size_t heap_start=(size_t)heap+sizeof(multi_heap_t);
MULTI_HEAP_PRINTF("\tstart: 0x%x\tend: 0x%x\tsize: %u\ttotal: %u\n", heap_start, heap_start+heap->size, heap->size, heap->total_size);
MULTI_HEAP_PRINTF("\tfree bytes: %u", heap->free_bytes);
if(heap->flags&MULTI_HEAP_FLAG_DIRTY)
	MULTI_HEAP_PRINTF("\tDIRTY", heap->free_bytes);
MULTI_HEAP_PRINTF("\n\tblocks allocated: %u\tfree blocks: %u\ttotal blocks: %u\n", heap->allocated_blocks, heap->free_blocks, heap->total_blocks);
if(heap->free_blocks)
	{
	MULTI_HEAP_PRINTF("\nfree blocks:\n");
	mem_block_map_dump(heap, &heap->map_free);
	}
MULTI_HEAP_PRINTF("\n");
}

void multi_heap_free_internal(multi_heap_handle_t heap, void* p)
{
size_t offset=mem_block_get_offset(p);
mem_block_info_t info;
if(!mem_block_get_info(heap, offset, &info))
	return;
if(info.flags&MEM_BLOCK_FLAG_FREE)
	return;
size_t heap_start=(size_t)heap+sizeof(multi_heap_t);
if(info.pos+info.size==heap_start+heap->size)
	{
	heap->size-=info.size;
	heap->free_bytes+=info.size;
	heap->allocated_blocks--;
	heap->total_blocks--;
	return;
	}
mem_block_init(heap, info.pos, info.size, MEM_BLOCK_FLAG_FREE);
heap->free_bytes+=info.size;
heap->allocated_blocks--;
heap->free_blocks++;
multi_heap_free_private(heap, info.pos);
}

void* multi_heap_malloc_internal(multi_heap_handle_t heap, size_t size)
{
size_t block_size=mem_block_calc_size(size);
void* p=multi_heap_malloc_private(heap, block_size);
if(p)
	return p;
p=multi_heap_malloc_fit(heap, block_size);
if(p)
	return p;
return multi_heap_malloc_direct(heap, block_size);
}


//===========
// Protected
//===========

void multi_heap_free_protected(multi_heap_handle_t heap, void* p)
{
mem_block_neighbours_t info;
size_t offset=mem_block_get_offset(p);
if(!mem_block_get_neighbours(heap, offset, &info))
	return;
if(info.cur.flags&MEM_BLOCK_FLAG_FREE)
	return;
size_t free_pos=info.cur.pos;
size_t free_size=info.cur.size;
if(info.prev.flags&MEM_BLOCK_FLAG_FREE)
	{
	multi_heap_remove_offset(heap, &info.prev);
	free_pos=info.prev.pos;
	free_size+=info.prev.size;
	heap->free_blocks--;
	heap->total_blocks--;
	}
if(info.next.flags&MEM_BLOCK_FLAG_FREE)
	{
	multi_heap_remove_offset(heap, &info.next);
	free_size+=info.next.size;
	heap->free_blocks--;
	heap->total_blocks--;
	}
size_t heap_start=(size_t)heap+sizeof(multi_heap_t);
if(free_pos+free_size==heap_start+heap->size)
	{
	heap->size-=free_size;
	heap->free_bytes+=info.cur.size;
	heap->allocated_blocks--;
	heap->total_blocks--;
	return;
	}
mem_block_init(heap, free_pos, free_size, MEM_BLOCK_FLAG_FREE);
heap->free_bytes+=info.cur.size;
heap->allocated_blocks--;
heap->free_blocks++;
if(!mem_block_map_add_offset(heap, &heap->map_free, free_size, free_pos))
	heap->flags|=MULTI_HEAP_FLAG_DIRTY;
}

void* multi_heap_malloc_protected(multi_heap_handle_t heap, size_t size)
{
size_t block_size=mem_block_calc_size(size);
if(heap->free_bytes<block_size)
	return NULL;
void* p=multi_heap_malloc_fit(heap, block_size);
if(p)
	return p;
if(heap->total_size-heap->size<block_size+512)
	return NULL;
return multi_heap_malloc_direct(heap, block_size);
}


//========
// Public
//========

void *multi_heap_aligned_alloc(multi_heap_handle_t heap, size_t size, size_t alignment)
{
if(alignment!=sizeof(size_t))
	return NULL; // TODO
return multi_heap_malloc(heap, size);
}

void* multi_heap_malloc(multi_heap_handle_t heap, size_t size)
{
if(heap==NULL||size==0)
	return NULL;
MULTI_HEAP_LOCK(heap->lock);
void* p=multi_heap_malloc_protected(heap, size);
multi_heap_update_map(heap);
MULTI_HEAP_UNLOCK(heap->lock);
return p;
}

void multi_heap_aligned_free(multi_heap_handle_t heap, void* p)
{
multi_heap_free(heap, p);
}

void multi_heap_free(multi_heap_handle_t heap, void* p)
{
MULTI_HEAP_LOCK(heap->lock);
multi_heap_free_protected(heap, p);
multi_heap_update_map(heap);
MULTI_HEAP_UNLOCK(heap->lock);
}

void* multi_heap_realloc(multi_heap_handle_t heap, void* p, size_t size)
{
if(p==NULL)
	return multi_heap_malloc(heap, size);
return NULL; // TODO
}

size_t multi_heap_get_allocated_size(multi_heap_handle_t heap, void* p)
{
MULTI_HEAP_LOCK(heap->lock);
size_t offset=mem_block_get_offset(p);
mem_block_info_t info;
if(mem_block_get_info(heap, offset, &info))
	{
	if(info.flags&MEM_BLOCK_FLAG_FREE)
		info.size=0;
	}
MULTI_HEAP_UNLOCK(heap->lock);
return info.size;
}

multi_heap_handle_t multi_heap_register(void* head, size_t size)
{
size_t offset=multi_heap_align_up((size_t)head, 8);
multi_heap_t* heap=(multi_heap_t*)offset;
size_t start=offset+sizeof(multi_heap_t);
size_t end=multi_heap_align_down(offset+size, 16);
heap->lock=NULL;
heap->total_size=end-start;
heap->size=0;
heap->free_bytes=heap->total_size;
heap->minimum_free_bytes=heap->total_size;
heap->allocated_blocks=0;
heap->free_blocks=0;
heap->total_blocks=0;
heap->flags=0;
heap->free_offset_count=0;
mem_block_map_init(&heap->map_free);
return heap;
}

void multi_heap_set_lock(multi_heap_handle_t heap, void* lock)
{
heap->lock=lock;
}

void multi_heap_dump(multi_heap_handle_t heap)
{
MULTI_HEAP_LOCK(heap->lock);
multi_heap_dump_internal(heap);
MULTI_HEAP_UNLOCK(heap->lock);
}

bool multi_heap_check(multi_heap_handle_t heap, bool print_errors)
{
MULTI_HEAP_LOCK(heap->lock);
bool b=multi_heap_check_internal(heap, print_errors);
MULTI_HEAP_UNLOCK(heap->lock);
return b;
}

size_t multi_heap_free_size(multi_heap_handle_t heap)
{
if(heap==NULL)
	return 0;
MULTI_HEAP_LOCK(heap->lock);
size_t size=heap->free_bytes;
MULTI_HEAP_UNLOCK(heap->lock);
return size;
}

size_t multi_heap_minimum_free_size(multi_heap_handle_t heap)
{
if(heap==NULL)
	return 0;
MULTI_HEAP_LOCK(heap->lock);
size_t size=heap->minimum_free_bytes;
MULTI_HEAP_UNLOCK(heap->lock);
return size;
}

void multi_heap_get_info(multi_heap_handle_t heap, multi_heap_info_t *info)
{
memset(info, 0, sizeof(multi_heap_info_t));
if(heap==NULL)
	return;
MULTI_HEAP_LOCK(heap->lock);
info->total_free_bytes=heap->free_bytes;
info->total_allocated_bytes=heap->size;
size_t largest=heap->total_size-heap->size;
size_t item_count=mem_block_map_get_item_count(&heap->map_free);
mem_block_map_item_t* last=mem_block_map_get_item_at(&heap->map_free, item_count-1);
if(last&&last->size>largest)
	largest=last->size;
info->largest_free_block=largest;
info->minimum_free_bytes=heap->minimum_free_bytes;
info->allocated_blocks=heap->allocated_blocks;
info->free_blocks=heap->free_blocks;
info->total_blocks=heap->total_blocks;
MULTI_HEAP_UNLOCK(heap->lock);
}
