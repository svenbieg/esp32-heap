// Copyright 2015-2016 Espressif Systems (Shanghai) PTE LTD
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at

//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

// This file was modified
// Copyright 2021, Sven Bieg (svenbieg@web.de)
// http://github.com/svenbieg/esp32-heap

#pragma once


//=======
// Using
//=======

#include <stdbool.h>
#include <stdint.h>
#include "mem_block_map.h"
#include "multi_heap_platform.h"


//=======
// Flags
//=======

#define MULTI_HEAP_FLAG_DIRTY ((uint32_t)1)


//======
// Info
//======

typedef struct multi_heap_info
{
void* lock;
size_t total_size;
size_t size;
size_t free_bytes;
size_t minimum_free_bytes;
size_t allocated_blocks;
size_t free_blocks;
size_t total_blocks;
uint32_t flags;
uint32_t free_offset_count;
size_t free_offsets[CONFIG_HEAP_MAX_OFFSETS];
mem_block_map_t map_free;
}multi_heap_t;


//===========
// Alignment
//===========

inline static size_t multi_heap_align_down(size_t offset, size_t align)
{
if(offset%align)
	offset-=offset%align;
return offset;
}

inline static size_t multi_heap_align_up(size_t offset, size_t align)
{
if(offset%align)
	offset+=(align-(offset%align));
return offset;
}


//=======
// Tasks
//=======

static inline void multi_heap_internal_lock(multi_heap_handle_t heap)
{
MULTI_HEAP_LOCK(heap->lock);
}

static inline void multi_heap_internal_unlock(multi_heap_handle_t heap)
{
MULTI_HEAP_UNLOCK(heap->lock);
}


//==========
// Internal
//==========

bool multi_heap_check_internal(multi_heap_handle_t heap, bool print_errors);
void multi_heap_dump_internal(multi_heap_handle_t heap);
void multi_heap_free_internal(multi_heap_handle_t heap, void* ptr);
void* multi_heap_malloc_internal(multi_heap_handle_t heap, size_t size);
