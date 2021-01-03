//==================
// mem_block_list.h
//==================

// Sorted offsets of free memory-blocks with the same size

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
// Group
//=======

typedef mem_block_group_t mem_block_list_group_t;

// Con-/Destructors
void mem_block_list_group_destroy(multi_heap_handle_t heap, mem_block_list_group_t* group);

// Access
bool mem_block_list_group_check(multi_heap_handle_t heap, mem_block_list_group_t* group, bool print_errors);
void mem_block_list_group_dump(mem_block_list_group_t* group);
size_t* mem_block_list_group_get_first_item(mem_block_list_group_t* group);
size_t* mem_block_list_group_get_item(mem_block_list_group_t* group, size_t offset);
size_t* mem_block_list_group_get_item_at(mem_block_list_group_t* group, size_t pos);
size_t mem_block_list_group_get_item_count(mem_block_list_group_t* group);
size_t* mem_block_list_group_get_last_item(mem_block_list_group_t* group);

// Modification
bool mem_block_list_group_add_item(multi_heap_handle_t heap, mem_block_list_group_t* group, size_t value, bool again, bool* exists);
bool mem_block_list_group_remove_item(multi_heap_handle_t heap, mem_block_list_group_t* group, size_t value);


//============
// Item-group
//============

typedef struct mem_block_list_item_group_t
{
uint16_t level;
uint16_t child_count;
size_t items[CONFIG_HEAP_GROUP_SIZE];
}mem_block_list_item_group_t;

// Con-/Destructors
mem_block_list_item_group_t* mem_block_list_item_group_create(multi_heap_handle_t heap);

// Access
bool mem_block_list_item_group_check(multi_heap_handle_t heap, mem_block_list_item_group_t* group, bool print_errors);
void mem_block_list_item_group_dump(mem_block_list_item_group_t* group);
size_t* mem_block_list_item_group_get_first_item(mem_block_list_item_group_t* group);
uint16_t mem_block_list_item_group_get_insert_pos(mem_block_list_item_group_t* group, size_t offset, bool* exists);
size_t* mem_block_list_item_group_get_item(mem_block_list_item_group_t* group, size_t offset);
size_t* mem_block_list_item_group_get_item_at(mem_block_list_item_group_t* group, size_t pos);
int16_t mem_block_list_item_group_get_item_pos(mem_block_list_item_group_t* group, size_t offset);
size_t* mem_block_list_item_group_get_last_item(mem_block_list_item_group_t* group);

// Modification
bool mem_block_list_item_group_add_item(mem_block_list_item_group_t* group, size_t value, bool* exists);
bool mem_block_list_item_group_add_item_internal(mem_block_list_item_group_t* group, size_t value, uint16_t pos);
void mem_block_list_item_group_append_items(mem_block_list_item_group_t* group, size_t const* items, uint16_t count);
void mem_block_list_item_group_insert_items(mem_block_list_item_group_t* group, uint16_t pos, size_t const* items, uint16_t count);
bool mem_block_list_item_group_remove_item(mem_block_list_item_group_t* group, size_t value);
bool mem_block_list_item_group_remove_item_at(mem_block_list_item_group_t* group, size_t pos);
void mem_block_list_item_group_remove_items(mem_block_list_item_group_t* group, uint16_t pos, uint16_t count);


//==============
// Parent-group
//==============

typedef struct
{
uint16_t level;
uint16_t child_count;
size_t* first;
size_t* last;
size_t item_count;
mem_block_list_group_t* children[CONFIG_HEAP_GROUP_SIZE];
}mem_block_list_parent_group_t;


// Con-/Destructors
mem_block_list_parent_group_t* mem_block_list_parent_group_create(multi_heap_handle_t heap, uint16_t level);
mem_block_list_parent_group_t* mem_block_list_parent_group_create_with_child(multi_heap_handle_t heap, mem_block_list_group_t* child);
void mem_block_list_parent_group_destroy(multi_heap_handle_t heap, mem_block_list_parent_group_t* group);

// Access
bool mem_block_list_parent_group_check(multi_heap_handle_t heap, mem_block_list_parent_group_t* group, bool print_errors);
void mem_block_list_parent_group_dump(mem_block_list_parent_group_t* group);
int16_t mem_block_list_parent_group_get_group(mem_block_list_parent_group_t* group, size_t* pos);
uint16_t mem_block_list_parent_group_get_insert_pos(mem_block_list_parent_group_t* group, size_t value, uint16_t* insert_pos, bool* exists);
size_t* mem_block_list_parent_group_get_item(mem_block_list_parent_group_t* group, size_t value);
size_t* mem_block_list_parent_group_get_item_at(mem_block_list_parent_group_t* group, size_t pos);
int16_t mem_block_list_parent_group_get_item_pos(mem_block_list_parent_group_t* group, size_t value);
int16_t mem_block_list_parent_group_get_nearest_space(mem_block_list_parent_group_t* group, int16_t pos);

// Modification
bool mem_block_list_parent_group_add_item(multi_heap_handle_t heap, mem_block_list_parent_group_t* group, size_t value, bool again, bool* exists);
bool mem_block_list_parent_group_add_item_internal(multi_heap_handle_t heap, mem_block_list_parent_group_t* group, size_t value, bool again, bool* exists);
void mem_block_list_parent_group_append_groups(mem_block_list_parent_group_t* group, mem_block_list_group_t* const* groups, uint16_t count);
bool mem_block_list_parent_group_combine_child(multi_heap_handle_t heap, mem_block_list_parent_group_t* group, uint16_t pos);
bool mem_block_list_parent_group_combine_children(multi_heap_handle_t heap, mem_block_list_parent_group_t* group);
void mem_block_list_parent_group_insert_groups(mem_block_list_parent_group_t* group, uint16_t pos, mem_block_list_group_t* const* groups, uint16_t count);
void mem_block_list_parent_group_move_children(mem_block_list_parent_group_t* group, uint16_t from, uint16_t to, uint16_t count);
void mem_block_list_parent_group_move_space(mem_block_list_parent_group_t* group, uint16_t from, uint16_t to);
void mem_block_list_parent_group_remove_group(multi_heap_handle_t heap, mem_block_list_parent_group_t* group, uint16_t pos);
void mem_block_list_parent_group_remove_groups(mem_block_list_parent_group_t* group, uint16_t pos, uint16_t count);
bool mem_block_list_parent_group_remove_item(multi_heap_handle_t heap, mem_block_list_parent_group_t* group, size_t value);
bool mem_block_list_parent_group_shift_children(mem_block_list_parent_group_t* group, uint16_t pos, uint16_t count);
bool mem_block_list_parent_group_split_child(multi_heap_handle_t heap, mem_block_list_parent_group_t* group, uint16_t pos);
void mem_block_list_parent_group_update_bounds(mem_block_list_parent_group_t* group);


//======
// List
//======

typedef struct
{
mem_block_list_group_t* root;
}mem_block_list_t;

// Con-/Destructors
void mem_block_list_destroy(multi_heap_handle_t heap, mem_block_list_t* list);
void mem_block_list_init(mem_block_list_t* list);
void mem_block_list_open(mem_block_list_t* list, size_t offset);

// Access
bool mem_block_list_check(multi_heap_handle_t heap, mem_block_list_t* list, bool print_errors);
void mem_block_list_dump(mem_block_list_t* list);
size_t mem_block_list_get_item(mem_block_list_t* list, size_t value);
size_t mem_block_list_get_item_at(mem_block_list_t* list, size_t pos);
size_t mem_block_list_get_item_count(mem_block_list_t* list);

// Modification
bool mem_block_list_add_offset(multi_heap_handle_t heap, mem_block_list_t* list, size_t value);
bool mem_block_list_remove_offset(multi_heap_handle_t heap, mem_block_list_t* list, size_t value);
void mem_block_list_update_root(multi_heap_handle_t heap, mem_block_list_t* list);
