//=================
// mem_block_map.h
//=================

// Free memory-blocks sorted by size

// Copyright 2021, Sven Bieg (svenbieg@web.de)
// http://github.com/svenbieg/esp32-heap

#pragma once


//=======
// Using
//=======

#include <multi_heap.h>
#include "mem_block_group.h"
#include "multi_heap_platform.h"


//=======
// Flags
//=======

#define MEM_BLOCK_MAP_FLAG_LIST (size_t)1
#define MEM_BLOCK_MAP_FLAGS_MASK (size_t)3
#define MEM_BLOCK_MAP_OFFSET_MASK ((size_t)~3)


//======
// Item
//======

typedef struct
{
size_t size;
size_t offset;
}mem_block_map_item_t;

size_t mem_block_map_item_get_offset(mem_block_map_item_t* item);


//=======
// Group
//=======

typedef mem_block_group_t mem_block_map_group_t;

// Con-/Destructors
void mem_block_map_group_destroy(multi_heap_handle_t heap, mem_block_map_group_t* group);

// Access
bool mem_block_map_group_check(multi_heap_handle_t heap, mem_block_map_group_t* group, bool print_errors);
void mem_block_map_group_dump(mem_block_map_group_t* group);
mem_block_map_item_t* mem_block_map_group_get_first_item(mem_block_map_group_t* group);
mem_block_map_item_t* mem_block_map_group_get_item(mem_block_map_group_t* group, size_t size);
mem_block_map_item_t* mem_block_map_group_get_item_at(mem_block_map_group_t* group, size_t pos);
size_t mem_block_map_group_get_item_count(mem_block_map_group_t* group);
mem_block_map_item_t* mem_block_map_group_get_last_item(mem_block_map_group_t* group);

// Modification
bool mem_block_map_group_add_offset(multi_heap_handle_t heap, mem_block_map_group_t* group, size_t size, size_t offset, bool again, bool* exists);
bool mem_block_map_group_remove_offset(multi_heap_handle_t heap, mem_block_map_group_t* group, size_t size, size_t offset, bool* removed);


//============
// Item-group
//============

typedef struct mem_block_map_item_group_t
{
uint16_t level;
uint16_t child_count;
mem_block_map_item_t items[CONFIG_HEAP_GROUP_SIZE];
}mem_block_map_item_group_t;

// Con-/Destructors
mem_block_map_item_group_t* mem_block_map_item_group_create(multi_heap_handle_t heap);

// Access
bool mem_block_map_item_group_check(multi_heap_handle_t heap, mem_block_map_item_group_t* group, bool print_errors);
void mem_block_map_item_group_dump(mem_block_map_item_group_t* group);
mem_block_map_item_t* mem_block_map_item_group_get_first_item(mem_block_map_item_group_t* group);
uint16_t mem_block_map_item_group_get_insert_pos(mem_block_map_item_group_t* group, size_t size, bool* exists);
mem_block_map_item_t* mem_block_map_item_group_get_item(mem_block_map_item_group_t* group, size_t size);
mem_block_map_item_t* mem_block_map_item_group_get_item_at(mem_block_map_item_group_t* group, size_t pos);
int16_t mem_block_map_item_group_get_item_pos(mem_block_map_item_group_t* group, size_t size);
mem_block_map_item_t* mem_block_map_item_group_get_last_item(mem_block_map_item_group_t* group);

// Modification
bool mem_block_map_item_group_add_offset(multi_heap_handle_t heap, mem_block_map_item_group_t* group, size_t size, size_t offset, bool* exists);
bool mem_block_map_item_group_add_offset_internal(mem_block_map_item_group_t* group, size_t size, size_t offset, uint16_t pos);
void mem_block_map_item_group_append_items(mem_block_map_item_group_t* group, mem_block_map_item_t const* items, uint16_t count);
void mem_block_map_item_group_insert_items(mem_block_map_item_group_t* group, uint16_t pos, mem_block_map_item_t const* items, uint16_t count);
bool mem_block_map_item_group_remove_offset(multi_heap_handle_t heap, mem_block_map_item_group_t* group, size_t size, size_t offset, bool* removed);
bool mem_block_map_item_group_remove_item_at(mem_block_map_item_group_t* group, size_t pos);
void mem_block_map_item_group_remove_items(mem_block_map_item_group_t* group, uint16_t pos, uint16_t count);


//==============
// Parent-group
//==============

typedef struct
{
uint16_t level;
uint16_t child_count;
mem_block_map_item_t* first;
mem_block_map_item_t* last;
size_t item_count;
mem_block_map_group_t* children[CONFIG_HEAP_GROUP_SIZE];
}mem_block_map_parent_group_t;


// Con-/Destructors
mem_block_map_parent_group_t* mem_block_map_parent_group_create(multi_heap_handle_t heap, uint16_t level);
mem_block_map_parent_group_t* mem_block_map_parent_group_create_with_child(multi_heap_handle_t heap, mem_block_map_group_t* child);
void mem_block_map_parent_group_destroy(multi_heap_handle_t heap, mem_block_map_parent_group_t* group);

// Access
bool mem_block_map_parent_group_check(multi_heap_handle_t heap, mem_block_map_parent_group_t* group, bool print_errors);
void mem_block_map_parent_group_dump(mem_block_map_parent_group_t* group);
int16_t mem_block_map_parent_group_get_group(mem_block_map_parent_group_t* group, size_t* pos);
uint16_t mem_block_map_parent_group_get_insert_pos(mem_block_map_parent_group_t* group, size_t size, uint16_t* insert_pos, bool* exists);
mem_block_map_item_t* mem_block_map_parent_group_get_item(mem_block_map_parent_group_t* group, size_t size);
mem_block_map_item_t* mem_block_map_parent_group_get_item_at(mem_block_map_parent_group_t* group, size_t pos);
int16_t mem_block_map_parent_group_get_item_pos(mem_block_map_parent_group_t* group, size_t size);
int16_t mem_block_map_parent_group_get_nearest_space(mem_block_map_parent_group_t* group, int16_t pos);

// Modification
bool mem_block_map_parent_group_add_offset(multi_heap_handle_t heap, mem_block_map_parent_group_t* group, size_t size, size_t offset, bool again, bool* exists);
bool mem_block_map_parent_group_add_offset_internal(multi_heap_handle_t heap, mem_block_map_parent_group_t* group, size_t size, size_t offset, bool again, bool* exists);
void mem_block_map_parent_group_append_groups(mem_block_map_parent_group_t* group, mem_block_map_group_t* const* groups, uint16_t count);
bool mem_block_map_parent_group_combine_child(multi_heap_handle_t heap, mem_block_map_parent_group_t* group, uint16_t pos);
bool mem_block_map_parent_group_combine_children(multi_heap_handle_t heap, mem_block_map_parent_group_t* group);
void mem_block_map_parent_group_insert_groups(mem_block_map_parent_group_t* group, uint16_t pos, mem_block_map_group_t* const* groups, uint16_t count);
void mem_block_map_parent_group_move_children(mem_block_map_parent_group_t* group, uint16_t from, uint16_t to, uint16_t count);
void mem_block_map_parent_group_move_space(mem_block_map_parent_group_t* group, uint16_t from, uint16_t to);
void mem_block_map_parent_group_remove_group(multi_heap_handle_t heap, mem_block_map_parent_group_t* group, uint16_t pos);
void mem_block_map_parent_group_remove_groups(mem_block_map_parent_group_t* group, uint16_t pos, uint16_t count);
bool mem_block_map_parent_group_remove_offset(multi_heap_handle_t heap, mem_block_map_parent_group_t* group, size_t size, size_t offset, bool* removed);
bool mem_block_map_parent_group_shift_children(mem_block_map_parent_group_t* group, uint16_t pos, uint16_t count);
bool mem_block_map_parent_group_split_child(multi_heap_handle_t heap, mem_block_map_parent_group_t* group, uint16_t pos);
void mem_block_map_parent_group_update_bounds(mem_block_map_parent_group_t* group);


//=====
// Map
//=====

typedef struct
{
mem_block_map_group_t* root;
}mem_block_map_t;

// Con-/Destructors
void mem_block_map_destroy(multi_heap_handle_t heap, mem_block_map_t* map);
void mem_block_map_init(mem_block_map_t* map);

// Access
bool mem_block_map_check(multi_heap_handle_t heap, mem_block_map_t* map, bool print_errors);
void mem_block_map_dump(multi_heap_handle_t heap, mem_block_map_t* map);
mem_block_map_item_t* mem_block_map_get_item_at(mem_block_map_t* map, size_t pos);
size_t mem_block_map_get_item_count(mem_block_map_t* map);
size_t mem_block_map_get_offset(mem_block_map_t* map, size_t size);

// Modification
bool mem_block_map_add_offset(multi_heap_handle_t heap, mem_block_map_t* map, size_t size, size_t offset);
bool mem_block_map_remove_offset(multi_heap_handle_t heap, mem_block_map_t* map, size_t size, size_t offset);
void mem_block_map_update_root(multi_heap_handle_t heap, mem_block_map_t* map);


//==========
// Iterator
//==========

typedef struct
{
mem_block_map_group_t* group;
uint16_t pos;
}mem_block_map_it_ptr_t;

typedef struct
{
mem_block_map_item_t* current;
mem_block_map_t* map;
uint16_t level_count;
mem_block_map_it_ptr_t pointers[CONFIG_HEAP_MAP_MAX_LEVELS];
}mem_block_map_it_t;


// Con-/Destructors
void mem_block_map_it_init(mem_block_map_it_t* it, mem_block_map_t* mem_block_map);

// Modification
bool mem_block_map_it_find(mem_block_map_it_t* it, size_t size);
bool mem_block_map_it_move_next(mem_block_map_it_t* it);
