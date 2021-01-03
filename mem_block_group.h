//===================
// mem_block_group.h
//===================

// Copyright 2021, Sven Bieg (svenbieg@web.de)
// http://github.com/svenbieg/esp32-heap

#pragma once


//=======
// Using
//=======

#include <stdbool.h>
#include <stdint.h>


//=======
// Flags
//=======

#define MEM_BLOCK_GROUP_FLAG_LOCKED ((uint16_t)0x8000)
#define MEM_BLOCK_GROUP_FLAG_DIRTY ((uint16_t)0x4000)
#define MEM_BLOCK_GROUP_LEVEL_MASK ((uint16_t)0x3FFF)


//=======
// Group
//=======

typedef struct
{
uint16_t level;
uint16_t child_count;
}mem_block_group_t;

// Initialization
void mem_block_group_init(mem_block_group_t* group, uint16_t level, uint16_t child_count);

// Access
uint16_t mem_block_group_get_child_count(mem_block_group_t* group);
uint16_t mem_block_group_get_level(mem_block_group_t* group);
bool mem_block_group_is_dirty(mem_block_group_t* group);
bool mem_block_group_is_locked(mem_block_group_t* group);

// Modification
void mem_block_group_lock(mem_block_group_t* group);
void mem_block_group_set_child_count(mem_block_group_t* group, uint16_t child_count);
void mem_block_group_set_clean(mem_block_group_t* group);
void mem_block_group_set_dirty(mem_block_group_t* group);
void mem_block_group_unlock(mem_block_group_t* group);
