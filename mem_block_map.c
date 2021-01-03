//=================
// mem_block_map.c
//=================

// Free memory-blocks sorted by size

// Copyright 2021, Sven Bieg (svenbieg@web.de)
// http://github.com/svenbieg/esp32-heap


//=======
// Using
//=======

#include "mem_block.h"
#include "mem_block_map.h"
#include "mem_block_list.h"
#include "multi_heap_internal.h"
#include "multi_heap_platform.h"


//======
// Item
//======

size_t mem_block_map_item_get_offset(mem_block_map_item_t* item)
{
if(!item)
	return 0;
size_t flags=item->offset&MEM_BLOCK_MAP_FLAGS_MASK;
size_t offset=item->offset&MEM_BLOCK_MAP_OFFSET_MASK;
if(!flags&MEM_BLOCK_MAP_FLAG_LIST)
	return offset;
mem_block_list_t list;
mem_block_list_open(&list, offset);
return mem_block_list_get_item_at(&list, 0);
}


//=======
// Group
//=======

// Con-/Destructors
void mem_block_map_group_destroy(multi_heap_handle_t heap, mem_block_map_group_t* group)
{
if(mem_block_group_get_level(group)==0)
	{
	multi_heap_free_internal(heap, group);
	return;
	}
mem_block_map_parent_group_destroy(heap, (mem_block_map_parent_group_t*)group);
}


// Access

bool mem_block_map_group_check(multi_heap_handle_t heap, mem_block_map_group_t* group, bool print_errors)
{
if(mem_block_group_get_level(group)==0)
	return mem_block_map_item_group_check(heap, (mem_block_map_item_group_t*)group, print_errors);
return mem_block_map_parent_group_check(heap, (mem_block_map_parent_group_t*)group, print_errors);
}

void mem_block_map_group_dump(mem_block_map_group_t* group)
{
if(mem_block_group_get_level(group)==0)
	{
	mem_block_map_item_group_dump((mem_block_map_item_group_t*)group);
	return;
	}
mem_block_map_parent_group_dump((mem_block_map_parent_group_t*)group);
}

mem_block_map_item_t* mem_block_map_group_get_first_item(mem_block_map_group_t* group)
{
if(mem_block_group_get_level(group)==0)
	return mem_block_map_item_group_get_first_item((mem_block_map_item_group_t*)group);
mem_block_map_parent_group_t* pgroup=(mem_block_map_parent_group_t*)group;
return pgroup->first;
}

mem_block_map_item_t* mem_block_map_group_get_item(mem_block_map_group_t* group, size_t size)
{
if(mem_block_group_get_level(group)==0)
	return mem_block_map_item_group_get_item((mem_block_map_item_group_t*)group, size);
return mem_block_map_parent_group_get_item((mem_block_map_parent_group_t*)group, size);
}

mem_block_map_item_t* mem_block_map_group_get_item_at(mem_block_map_group_t* group, size_t pos)
{
if(mem_block_group_get_level(group)==0)
	return mem_block_map_item_group_get_item_at((mem_block_map_item_group_t*)group, pos);
return mem_block_map_parent_group_get_item_at((mem_block_map_parent_group_t*)group, pos);
}

size_t mem_block_map_group_get_item_count(mem_block_map_group_t* group)
{
if(mem_block_group_get_level(group)==0)
	return group->child_count;
mem_block_map_parent_group_t* pgroup=(mem_block_map_parent_group_t*)group;
return pgroup->item_count;
}

mem_block_map_item_t* mem_block_map_group_get_last_item(mem_block_map_group_t* group)
{
if(mem_block_group_get_level(group)==0)
	return mem_block_map_item_group_get_last_item((mem_block_map_item_group_t*)group);
mem_block_map_parent_group_t* pgroup=(mem_block_map_parent_group_t*)group;
return pgroup->last;
}


// Modification

bool mem_block_map_group_add_offset(multi_heap_handle_t heap, mem_block_map_group_t* group, size_t size, size_t offset, bool again, bool* exists)
{
if(mem_block_group_get_level(group)==0)
	return mem_block_map_item_group_add_offset(heap, (mem_block_map_item_group_t*)group, size, offset, exists);
return mem_block_map_parent_group_add_offset(heap, (mem_block_map_parent_group_t*)group, size, offset, again, exists);
}

bool mem_block_map_group_remove_offset(multi_heap_handle_t heap, mem_block_map_group_t* group, size_t size, size_t offset, bool* removed)
{
if(mem_block_group_get_level(group)==0)
	return mem_block_map_item_group_remove_offset(heap, (mem_block_map_item_group_t*)group, size, offset, removed);
return mem_block_map_parent_group_remove_offset(heap, (mem_block_map_parent_group_t*)group, size, offset, removed);
}


//============
// Item-group
//============

// Con-/Destructors

mem_block_map_item_group_t* mem_block_map_item_group_create(multi_heap_handle_t heap)
{
mem_block_map_item_group_t* group=(mem_block_map_item_group_t*)multi_heap_malloc_internal(heap, sizeof(mem_block_map_item_group_t));
if(group==NULL)
	return NULL;
mem_block_group_init((mem_block_group_t*)group, 0, 0);
return group;
}


// Access

bool mem_block_map_item_group_check(multi_heap_handle_t heap, mem_block_map_item_group_t* group, bool print_errors)
{
uint16_t child_count=mem_block_group_get_child_count((mem_block_group_t*)group);
if(!child_count)
	{
	if(print_errors)
		{
		MULTI_HEAP_PRINTF("multi_heap_check(0x%x): empty map-item-group\n", heap);
		}
	return false;
	}
bool success=true;
size_t heap_start=(size_t)heap+sizeof(multi_heap_t);
size_t heap_end=heap_start+heap->size;
size_t last_size=0;
for(uint16_t pos=0; pos<child_count; pos++)
	{
	size_t size=group->items[pos].size;
	if(size<=last_size)
		{
		if(print_errors)
			{
			MULTI_HEAP_PRINTF("multi_heap_check(0x%x): size %u<=%u\n", heap, size, last_size);
			}
		success=false;
		continue;
		}
	last_size=size;
	size_t entry=group->items[pos].offset;
	size_t flags=entry&MEM_BLOCK_MAP_FLAGS_MASK;
	size_t offset=entry&MEM_BLOCK_MAP_OFFSET_MASK;
	if(offset<heap_start||offset>=heap_end)
		{
		if(print_errors)
			{
			MULTI_HEAP_PRINTF("multi_heap_check(0x%x, 0x%x): map offset outside heap\n", heap, offset);
			}
		success=false;
		continue;
		}
	if(flags&MEM_BLOCK_MAP_FLAG_LIST)
		{
		mem_block_list_t list;
		mem_block_list_open(&list, offset);
		if(!mem_block_list_check(heap, &list, print_errors))
			success=false;
		}
	}
return success;
}

void mem_block_map_item_group_dump(mem_block_map_item_group_t* group)
{
uint16_t child_count=mem_block_group_get_child_count((mem_block_group_t*)group);
for(uint16_t pos=0; pos<child_count; pos++)
	{
	size_t entry=group->items[pos].offset;
	size_t size=group->items[pos].size;
	size_t offset=entry&MEM_BLOCK_MAP_OFFSET_MASK;
	MULTI_HEAP_PRINTF("\t[%u]", size);
	if(entry&MEM_BLOCK_MAP_FLAG_LIST)
		{
		mem_block_list_t list;
		mem_block_list_open(&list, offset);
		mem_block_list_dump(&list);
		}
	else
		{
		MULTI_HEAP_PRINTF(" 0x%x", offset);
		}
	MULTI_HEAP_PRINTF("\n");
	}
}

mem_block_map_item_t* mem_block_map_item_group_get_first_item(mem_block_map_item_group_t* group)
{
if(mem_block_group_get_child_count((mem_block_group_t*)group)==0)
	return NULL;
return group->items;
}

uint16_t mem_block_map_item_group_get_insert_pos(mem_block_map_item_group_t* group, size_t size, bool* exists)
{
uint16_t child_count=mem_block_group_get_child_count((mem_block_group_t*)group);
uint16_t start=0;
uint16_t end=child_count;
while(start<end)
	{
	uint16_t pos=start+(end-start)/2;
	if(group->items[pos].size>size)
		{
		end=pos;
		continue;
		}
	if(group->items[pos].size<size)
		{
		start=pos+1;
		continue;
		}
	*exists=true;
	return pos;
	}
return start;
}

mem_block_map_item_t* mem_block_map_item_group_get_item(mem_block_map_item_group_t* group, size_t size)
{
int16_t pos=mem_block_map_item_group_get_item_pos(group, size);
if(pos<0)
	return NULL;
return &group->items[pos];
}

mem_block_map_item_t* mem_block_map_item_group_get_item_at(mem_block_map_item_group_t* group, size_t pos)
{
if(pos>=mem_block_group_get_child_count((mem_block_group_t*)group))
	return NULL;
return &group->items[pos];
}

int16_t mem_block_map_item_group_get_item_pos(mem_block_map_item_group_t* group, size_t size)
{
uint16_t start=0;
uint16_t end=mem_block_group_get_child_count((mem_block_group_t*)group);
uint16_t pos=0;
mem_block_map_item_t* item=NULL;
while(start<end)
	{
	pos=start+(end-start)/2;
	item=&group->items[pos];
	if(item->size>size)
		{
		end=pos;
		continue;
		}
	if(item->size<size)
		{
		start=pos+1;
		continue;
		}
	return pos;
	}
return -(int16_t)pos-1;
}

mem_block_map_item_t* mem_block_map_item_group_get_last_item(mem_block_map_item_group_t* group)
{
uint16_t count=mem_block_group_get_child_count((mem_block_group_t*)group);
if(count==0)
	return NULL;
return &group->items[count-1];
}


// Modification

bool mem_block_map_item_group_add_offset(multi_heap_handle_t heap, mem_block_map_item_group_t* group, size_t size, size_t offset, bool* exists)
{
uint16_t pos=mem_block_map_item_group_get_insert_pos(group, size, exists);
if(*exists)
	{
	mem_block_group_lock((mem_block_group_t*)group);
	mem_block_map_item_t* item=mem_block_map_item_group_get_item_at(group, pos);
	size_t flags=item->offset&MEM_BLOCK_MAP_FLAGS_MASK;
	size_t entry=item->offset&MEM_BLOCK_MAP_OFFSET_MASK;
	mem_block_list_t list;
	if(flags&MEM_BLOCK_MAP_FLAG_LIST)
		{
		mem_block_list_open(&list, entry);
		if(!mem_block_list_add_offset(heap, &list, offset))
			{
			mem_block_group_unlock((mem_block_group_t*)group);
			return false;
			}
		}
	else
		{
		mem_block_list_init(&list);
		if(!mem_block_list_add_offset(heap, &list, entry))
			{
			mem_block_list_destroy(heap, &list);
			mem_block_group_unlock((mem_block_group_t*)group);
			return false;
			}
		if(!mem_block_list_add_offset(heap, &list, offset))
			{
			mem_block_list_destroy(heap, &list);
			mem_block_group_unlock((mem_block_group_t*)group);
			return false;
			}
		}
	*exists=false;
	pos=mem_block_map_item_group_get_insert_pos(group, size, exists);
	if(*exists)
		{
		item=mem_block_map_item_group_get_item_at(group, pos);
		size_t item_count=mem_block_list_get_item_count(&list);
		if(item_count>1)
			{
			entry=(size_t)list.root;
			entry|=MEM_BLOCK_MAP_FLAG_LIST;
			item->offset=entry;
			}
		else
			{
			item->offset=offset;
			mem_block_list_destroy(heap, &list);
			}
		}
	else
		{
		mem_block_map_item_group_add_offset_internal(group, size, offset, pos);
		mem_block_list_destroy(heap, &list);
		}
	mem_block_group_unlock((mem_block_group_t*)group);
	return true;
	}
return mem_block_map_item_group_add_offset_internal(group, size, offset, pos);
}

bool mem_block_map_item_group_add_offset_internal(mem_block_map_item_group_t* group, size_t size, size_t offset, uint16_t pos)
{
uint16_t child_count=mem_block_group_get_child_count((mem_block_group_t*)group);
if(child_count==CONFIG_HEAP_GROUP_SIZE)
	return false;
for(uint16_t u=child_count; u>pos; u--)
	group->items[u]=group->items[u-1];
group->items[pos].size=size;
group->items[pos].offset=offset;
mem_block_group_set_child_count((mem_block_group_t*)group, child_count+1);
return true;
}

void mem_block_map_item_group_append_items(mem_block_map_item_group_t* group, mem_block_map_item_t const* items, uint16_t count)
{
uint16_t child_count=mem_block_group_get_child_count((mem_block_group_t*)group);
for(uint16_t u=0; u<count; u++)
	group->items[child_count+u]=items[u];
mem_block_group_set_child_count((mem_block_group_t*)group, child_count+count);
}

void mem_block_map_item_group_insert_items(mem_block_map_item_group_t* group, uint16_t pos, mem_block_map_item_t const* items, uint16_t count)
{
uint16_t child_count=mem_block_group_get_child_count((mem_block_group_t*)group);
for(uint16_t u=child_count+count-1; u>=pos+count; u--)
	group->items[u]=group->items[u-count];
for(uint16_t u=0; u<count; u++)
	group->items[pos+u]=items[u];
mem_block_group_set_child_count((mem_block_group_t*)group, child_count+count);
}

bool mem_block_map_item_group_remove_offset(multi_heap_handle_t heap, mem_block_map_item_group_t* group, size_t size, size_t offset, bool* removed)
{
int16_t pos=mem_block_map_item_group_get_item_pos(group, size);
if(pos<0)
	return false;
mem_block_map_item_t* item=mem_block_map_item_group_get_item_at(group, pos);
size_t flags=item->offset&MEM_BLOCK_MAP_FLAGS_MASK;
size_t entry=item->offset&MEM_BLOCK_MAP_OFFSET_MASK;
if(flags&MEM_BLOCK_MAP_FLAG_LIST)
	{
	mem_block_list_t list;
	mem_block_list_open(&list, entry);
	if(!mem_block_list_remove_offset(heap, &list, offset))
		return false;
	size_t item_count=mem_block_list_get_item_count(&list);
	if(item_count>1)
		{
		entry=(size_t)list.root;
		entry|=MEM_BLOCK_MAP_FLAG_LIST;
		item->offset=entry;
		}
	else if(item_count==1)
		{
		entry=mem_block_list_get_item_at(&list, 0);
		item->offset=entry;
		mem_block_list_destroy(heap, &list);
		}
	else
		{
		if(!mem_block_map_item_group_remove_item_at(group, pos))
			return false;
		*removed=true;
		}
	return true;
	}
if(!mem_block_map_item_group_remove_item_at(group, pos))
	return false;
*removed=true;
return true;
}

bool mem_block_map_item_group_remove_item_at(mem_block_map_item_group_t* group, size_t at)
{
uint16_t pos=(uint16_t)at;
uint16_t child_count=mem_block_group_get_child_count((mem_block_group_t*)group);
if(pos>=child_count)
	return false;
for(uint16_t u=pos; u+1<child_count; u++)
	group->items[u]=group->items[u+1];
mem_block_group_set_child_count((mem_block_group_t*)group, child_count-1);
return true;
}

void mem_block_map_item_group_remove_items(mem_block_map_item_group_t* group, uint16_t pos, uint16_t count)
{
uint16_t child_count=mem_block_group_get_child_count((mem_block_group_t*)group);
for(uint16_t u=pos; u+count<child_count; u++)
	group->items[u]=group->items[u+count];
mem_block_group_set_child_count((mem_block_group_t*)group, child_count-count);
}


//==============
// Parent-group
//==============

// Con-/Destructors

mem_block_map_parent_group_t* mem_block_map_parent_group_create(multi_heap_handle_t heap, uint16_t level)
{
mem_block_map_parent_group_t* group=(mem_block_map_parent_group_t*)multi_heap_malloc_internal(heap, sizeof(mem_block_map_parent_group_t));
if(group==NULL)
	return NULL;
mem_block_group_init((mem_block_group_t*)group, level, 0);
group->first=NULL;
group->last=NULL;
group->item_count=0;
return group;
}

mem_block_map_parent_group_t* mem_block_map_parent_group_create_with_child(multi_heap_handle_t heap, mem_block_map_group_t* child)
{
mem_block_map_parent_group_t* group=(mem_block_map_parent_group_t*)multi_heap_malloc_internal(heap, sizeof(mem_block_map_parent_group_t));
if(group==NULL)
	return NULL;
uint16_t child_level=mem_block_group_get_level(child);
mem_block_group_init((mem_block_group_t*)group, child_level+1, 1);
group->first=mem_block_map_group_get_first_item(child);
group->last=mem_block_map_group_get_last_item(child);
group->item_count=mem_block_map_group_get_item_count(child);
group->children[0]=child;
return group;
}

void mem_block_map_parent_group_destroy(multi_heap_handle_t heap, mem_block_map_parent_group_t* group)
{
uint16_t child_count=mem_block_group_get_child_count((mem_block_group_t*)group);
for(uint16_t u=0; u<child_count; u++)
	mem_block_map_group_destroy(heap, group->children[u]);
multi_heap_free_internal(heap, group);
}


// Access

bool mem_block_map_parent_group_check(multi_heap_handle_t heap, mem_block_map_parent_group_t* group, bool print_errors)
{
uint16_t child_count=mem_block_group_get_child_count((mem_block_group_t*)group);
if(!child_count)
	{
	if(print_errors)
		{
		MULTI_HEAP_PRINTF("multi_heap_check(0x%x): empty map-parent-group\n", heap);
		}
	return false;
	}
bool success=true;
for(uint16_t pos=0; pos<child_count; pos++)
	{
	if(!mem_block_map_group_check(heap, group->children[pos], print_errors))
		success=false;
	}
return success;
}

void mem_block_map_parent_group_dump(mem_block_map_parent_group_t* group)
{
uint16_t child_count=mem_block_group_get_child_count((mem_block_group_t*)group);
for(uint16_t pos=0; pos<child_count; pos++)
	mem_block_map_group_dump(group->children[pos]);
}

int16_t mem_block_map_parent_group_get_group(mem_block_map_parent_group_t* group, size_t* pos)
{
uint16_t child_count=mem_block_group_get_child_count((mem_block_group_t*)group);
for(uint16_t u=0; u<child_count; u++)
	{
	size_t item_count=mem_block_map_group_get_item_count(group->children[u]);
	if(*pos<item_count)
		return u;
	*pos-=item_count;
	}
return -1;
}

uint16_t mem_block_map_parent_group_get_insert_pos(mem_block_map_parent_group_t* group, size_t size, uint16_t* insert_pos, bool* exists)
{
uint16_t child_count=mem_block_group_get_child_count((mem_block_group_t*)group);
if(!child_count)
	return 0;
uint16_t start=0;
uint16_t end=child_count;
mem_block_map_item_t* first=NULL;
mem_block_map_item_t* last=NULL;
int16_t empty=0;
while(start<end)
	{
	uint16_t pos=start+(end-start)/2+empty;
	first=mem_block_map_group_get_first_item(group->children[pos]);
	if(first==NULL)
		{
		if(empty<0)
			{
			empty--;
			if((end-start)/2+empty<start)
				break;
			continue;
			}
		empty++;
		if((end-start)/2+empty>=end)
			{
			empty=-1;
			if((end-start)/2+empty<start)
				break;
			}
		continue;
		}
	empty=0;
	last=mem_block_map_group_get_last_item(group->children[pos]);
	if(first->size==size||last->size==size)
		{
		*exists=true;
		*insert_pos=pos;
		return 1;
		}
	if(first->size>size)
		{
		end=pos;
		continue;
		}
	if(last->size<size)
		{
		start=pos+1;
		continue;
		}
	start=pos;
	break;
	}
if(start>child_count-1)
	start=child_count-1;
*insert_pos=start;
if(start>0)
	{
	first=mem_block_map_group_get_first_item(group->children[start]);
	if(first==NULL||first->size>=size)
		{
		*insert_pos=start-1;
		return 2;
		}
	}
if(start+1<child_count)
	{
	last=mem_block_map_group_get_last_item(group->children[start]);
	if(last==NULL||last->size<=size)
		return 2;
	}
return 1;
}

mem_block_map_item_t* mem_block_map_parent_group_get_item(mem_block_map_parent_group_t* group, size_t size)
{
int16_t child=mem_block_map_parent_group_get_item_pos(group, size);
if(child<0)
	return NULL;
return mem_block_map_group_get_item(group->children[child], size);
}

mem_block_map_item_t* mem_block_map_parent_group_get_item_at(mem_block_map_parent_group_t* group, size_t pos)
{
int16_t child=mem_block_map_parent_group_get_group(group, &pos);
if(child<0)
	return NULL;
return mem_block_map_group_get_item_at(group->children[child], pos);
}

int16_t mem_block_map_parent_group_get_item_pos(mem_block_map_parent_group_t* group, size_t size)
{
uint16_t start=0;
uint16_t end=mem_block_group_get_child_count((mem_block_group_t*)group);
uint16_t pos=0;
mem_block_map_item_t* first=NULL;
mem_block_map_item_t* last=NULL;
int16_t empty=0;
while(start<end)
	{
	pos=start+(end-start)/2+empty;
	first=mem_block_map_group_get_first_item(group->children[pos]);
	if(first==NULL)
		{
		if(empty<0)
			{
			empty--;
			if((end-start)/2+empty<start)
				break;
			continue;
			}
		empty++;
		if((end-start)/2+empty>=end)
			{
			empty=-1;
			if((end-start)/2+empty<start)
				break;
			}
		continue;
		}
	empty=0;
	if(first->size>size)
		{
		end=pos;
		continue;
		}
	last=mem_block_map_group_get_last_item(group->children[pos]);
	if(last->size<size)
		{
		start=pos+1;
		continue;
		}
	return pos;
	}
return -(int16_t)pos-1;
}

int16_t mem_block_map_parent_group_get_nearest_space(mem_block_map_parent_group_t* group, int16_t pos)
{
int16_t child_count=(int16_t)mem_block_group_get_child_count((mem_block_group_t*)group);
int16_t before=pos-1;
int16_t after=pos+1;
while(before>=0||after<child_count)
	{
	if(before>=0)
		{
		uint16_t count=mem_block_group_get_child_count(group->children[before]);
		if(count<CONFIG_HEAP_GROUP_SIZE)
			return before;
		before--;
		}
	if(after<child_count)
		{
		uint16_t count=mem_block_group_get_child_count(group->children[after]);
		if(count<CONFIG_HEAP_GROUP_SIZE)
			return after;
		after++;
		}
	}
return -1;
}


// Modification

bool mem_block_map_parent_group_add_offset(multi_heap_handle_t heap, mem_block_map_parent_group_t* group, size_t size, size_t offset, bool again, bool* exists)
{
mem_block_group_lock((mem_block_group_t*)group);
bool added=mem_block_map_parent_group_add_offset_internal(heap, group, size, offset, again, exists);
if(added)
	{
	group->item_count++;
	mem_block_map_parent_group_update_bounds(group);
	}
if(mem_block_group_is_dirty((mem_block_group_t*)group))
	{
	mem_block_map_parent_group_combine_children(heap, group);
	mem_block_group_set_clean((mem_block_group_t*)group);
	}
mem_block_group_unlock((mem_block_group_t*)group);
return added;
}

bool mem_block_map_parent_group_add_offset_internal(multi_heap_handle_t heap, mem_block_map_parent_group_t* group, size_t size, size_t offset, bool again, bool* exists)
{
uint16_t pos=0;
uint16_t count=mem_block_map_parent_group_get_insert_pos(group, size, &pos, exists);
if(!again)
	{
	for(uint16_t u=0; u<count; u++)
		{
		if(mem_block_map_group_add_offset(heap, group->children[pos+u], size, offset, false, exists))
			return true;
		if(*exists)
			return false;
		}
	if(mem_block_map_parent_group_shift_children(group, pos, count))
		{
		count=mem_block_map_parent_group_get_insert_pos(group, size, &pos, exists);
		for(uint16_t u=0; u<count; u++)
			{
			if(mem_block_map_group_add_offset(heap, group->children[pos+u], size, offset, false, exists))
				return true;
			}
		}
	}
if(!mem_block_map_parent_group_split_child(heap, group, pos))
	return false;
count=mem_block_map_parent_group_get_insert_pos(group, size, &pos, exists);
for(uint16_t u=0; u<count; u++)
	{
	if(mem_block_map_group_add_offset(heap, group->children[pos+u], size, offset, true, exists))
		return true;
	}
return false;
}

void mem_block_map_parent_group_append_groups(mem_block_map_parent_group_t* group, mem_block_map_group_t* const* groups, uint16_t count)
{
uint16_t child_count=mem_block_group_get_child_count((mem_block_group_t*)group);
for(uint16_t u=0; u<count; u++)
	{
	group->children[child_count+u]=groups[u];
	group->item_count+=mem_block_map_group_get_item_count(groups[u]);
	}
mem_block_group_set_child_count((mem_block_group_t*)group, child_count+count);
mem_block_map_parent_group_update_bounds(group);
}

bool mem_block_map_parent_group_combine_child(multi_heap_handle_t heap, mem_block_map_parent_group_t* group, uint16_t pos)
{
uint16_t count=mem_block_group_get_child_count(group->children[pos]);
if(count==0)
	{
	mem_block_map_parent_group_remove_group(heap, group, pos);
	return true;
	}
if(pos>0)
	{
	uint16_t before=mem_block_group_get_child_count(group->children[pos-1]);
	if(count+before<=CONFIG_HEAP_GROUP_SIZE)
		{
		mem_block_map_parent_group_move_children(group, pos, pos-1, count);
		mem_block_map_parent_group_remove_group(heap, group, pos);
		return true;
		}
	}
uint16_t child_count=mem_block_group_get_child_count((mem_block_group_t*)group);
if(pos+1<child_count)
	{
	uint16_t after=mem_block_group_get_child_count(group->children[pos+1]);
	if(count+after<=CONFIG_HEAP_GROUP_SIZE)
		{
		mem_block_map_parent_group_move_children(group, pos+1, pos, after);
		mem_block_map_parent_group_remove_group(heap, group, pos+1);
		return true;
		}
	}
return false;
}

bool mem_block_map_parent_group_combine_children(multi_heap_handle_t heap, mem_block_map_parent_group_t* group)
{
bool combined=false;
uint16_t child_count=mem_block_group_get_child_count((mem_block_group_t*)group);
for(uint16_t pos=0; pos<child_count; )
	{
	if(mem_block_map_parent_group_combine_child(heap, group, pos))
		{
		child_count--;
		combined=true;
		}
	else
		{
		pos++;
		}
	}
return combined;
}

void mem_block_map_parent_group_insert_groups(mem_block_map_parent_group_t* group, uint16_t pos, mem_block_map_group_t* const* groups, uint16_t count)
{
uint16_t child_count=mem_block_group_get_child_count((mem_block_group_t*)group);
for(uint16_t u=child_count+count-1; u>=pos+count; u--)
	group->children[u]=group->children[u-count];
for(uint16_t u=0; u<count; u++)
	{
	group->children[pos+u]=groups[u];
	group->item_count+=mem_block_map_group_get_item_count(groups[u]);
	}
mem_block_group_set_child_count((mem_block_group_t*)group, child_count+count);
mem_block_map_parent_group_update_bounds(group);
}

void mem_block_map_parent_group_move_children(mem_block_map_parent_group_t* group, uint16_t from, uint16_t to, uint16_t count)
{
uint16_t level=mem_block_group_get_level((mem_block_group_t*)group);
if(level>1)
	{
	mem_block_map_parent_group_t* src=(mem_block_map_parent_group_t*)group->children[from];
	mem_block_map_parent_group_t* dst=(mem_block_map_parent_group_t*)group->children[to];
	if(from>to)
		{
		mem_block_map_parent_group_append_groups(dst, src->children, count);
		mem_block_map_parent_group_remove_groups(src, 0, count);
		}
	else
		{
		uint16_t src_count=mem_block_group_get_child_count((mem_block_group_t*)src);
		mem_block_map_parent_group_insert_groups(dst, 0, &src->children[src_count-count], count);
		mem_block_map_parent_group_remove_groups(src, src_count-count, count);
		}
	}
else
	{
	mem_block_map_item_group_t* src=(mem_block_map_item_group_t*)group->children[from];
	mem_block_map_item_group_t* dst=(mem_block_map_item_group_t*)group->children[to];
	if(from>to)
		{
		mem_block_map_item_group_append_items(dst, src->items, count);
		mem_block_map_item_group_remove_items(src, 0, count);
		}
	else
		{
		uint16_t src_count=mem_block_group_get_child_count((mem_block_group_t*)src);
		mem_block_map_item_group_insert_items(dst, 0, &src->items[src_count-count], count);
		mem_block_map_item_group_remove_items(src, src_count-count, count);
		}
	}
}

void mem_block_map_parent_group_move_space(mem_block_map_parent_group_t* group, uint16_t from, uint16_t to)
{
if(from<to)
	{
	for(uint16_t u=from; u<to; u++)
		mem_block_map_parent_group_move_children(group, u+1, u, 1);
	}
else
	{
	for(uint16_t u=from; u>to; u--)
		mem_block_map_parent_group_move_children(group, u-1, u, 1);
	}
}

void mem_block_map_parent_group_remove_group(multi_heap_handle_t heap, mem_block_map_parent_group_t* group, uint16_t pos)
{
uint16_t child_count=mem_block_group_get_child_count((mem_block_group_t*)group);
mem_block_map_group_t* child=group->children[pos];
for(uint16_t u=pos; u+1<child_count; u++)
	group->children[u]=group->children[u+1];
mem_block_group_set_child_count((mem_block_group_t*)group, child_count-1);
multi_heap_free_internal(heap, child);
}

void mem_block_map_parent_group_remove_groups(mem_block_map_parent_group_t* group, uint16_t pos, uint16_t count)
{
for(uint16_t u=0; u<count; u++)
	group->item_count-=mem_block_map_group_get_item_count(group->children[pos+u]);
uint16_t child_count=mem_block_group_get_child_count((mem_block_group_t*)group);
for(uint16_t u=pos; u+count<child_count; u++)
	group->children[u]=group->children[u+count];
mem_block_group_set_child_count((mem_block_group_t*)group, child_count-count);
mem_block_map_parent_group_update_bounds(group);
}

bool mem_block_map_parent_group_remove_offset(multi_heap_handle_t heap, mem_block_map_parent_group_t* group, size_t size, size_t offset, bool* removed)
{
int16_t pos=mem_block_map_parent_group_get_item_pos(group, size);
if(pos<0)
	return false;
if(!mem_block_map_group_remove_offset(heap, group->children[pos], size, offset, removed))
	return false;
if(*removed)
	{
	group->item_count--;
	mem_block_map_parent_group_update_bounds(group);
	if(mem_block_group_is_locked((mem_block_group_t*)group))
		{
		mem_block_group_set_dirty((mem_block_group_t*)group);
		}
	else
		{
		mem_block_map_parent_group_combine_child(heap, group, pos);
		}
	}
return true;
}

bool mem_block_map_parent_group_shift_children(mem_block_map_parent_group_t* group, uint16_t pos, uint16_t count)
{
int16_t space=mem_block_map_parent_group_get_nearest_space(group, pos);
if(space<0)
	return false;
if(count>1&&space>pos)
	pos++;
mem_block_map_parent_group_move_space(group, space, pos);
return true;
}

bool mem_block_map_parent_group_split_child(multi_heap_handle_t heap, mem_block_map_parent_group_t* group, uint16_t pos)
{
uint16_t child_count=mem_block_group_get_child_count((mem_block_group_t*)group);
if(child_count==CONFIG_HEAP_GROUP_SIZE)
	return false;
mem_block_map_group_t* child=NULL;
uint16_t level=mem_block_group_get_level((mem_block_group_t*)group);
if(level>1)
	{
	child=(mem_block_map_group_t*)mem_block_map_parent_group_create(heap, level-1);
	}
else
	{
	child=(mem_block_map_group_t*)mem_block_map_item_group_create(heap);
	}
if(!child)
	return false;
for(uint16_t u=child_count; u>pos+1; u--)
	group->children[u]=group->children[u-1];
group->children[pos+1]=child;
mem_block_group_set_child_count((mem_block_group_t*)group, child_count+1);
mem_block_map_parent_group_move_children(group, pos, pos+1, 1);
return true;
}

void mem_block_map_parent_group_update_bounds(mem_block_map_parent_group_t* group)
{
uint16_t child_count=mem_block_group_get_child_count((mem_block_group_t*)group);
if(child_count==0)
	{
	group->first=NULL;
	group->last=NULL;
	return;
	}
for(uint16_t pos=0; pos<child_count; pos++)
	{
	group->first=mem_block_map_group_get_first_item(group->children[pos]);
	if(group->first!=NULL)
		break;
	}
for(uint16_t pos=child_count; pos>0; pos--)
	{
	group->last=mem_block_map_group_get_last_item(group->children[pos-1]);
	if(group->last!=NULL)
		break;
	}
}


//=====
// Map
//=====

// Con-/Destructors

void mem_block_map_destroy(multi_heap_handle_t heap, mem_block_map_t* map)
{
mem_block_map_group_t* root=map->root;
if(!root)
	return;
map->root=NULL;
mem_block_map_group_destroy(heap, root);
}

void mem_block_map_init(mem_block_map_t* map)
{
map->root=NULL;
}


// Access

bool mem_block_map_check(multi_heap_handle_t heap, mem_block_map_t* map, bool print_errors)
{
if(!map->root)
	return true;
return mem_block_map_group_check(heap, map->root, print_errors);
}

void mem_block_map_dump(multi_heap_handle_t heap, mem_block_map_t* map)
{
if(!map->root)
	return;
mem_block_map_group_dump(map->root);
}

mem_block_map_item_t* mem_block_map_get_item_at(mem_block_map_t* map, size_t pos)
{
if(!map->root)
	return NULL;
return mem_block_map_group_get_item_at(map->root, pos);
}

size_t mem_block_map_get_item_count(mem_block_map_t* map)
{
if(!map->root)
	return 0;
return mem_block_map_group_get_item_count(map->root);
}

size_t mem_block_map_get_offset(mem_block_map_t* map, size_t size)
{
if(!map->root)
	return 0;
mem_block_map_item_t* item=mem_block_map_group_get_item(map->root, size);
if(!item)
	return 0;
size_t entry=item->offset;
size_t flags=entry&MEM_BLOCK_MAP_FLAGS_MASK;
size_t offset=entry&MEM_BLOCK_MAP_OFFSET_MASK;
if(flags&MEM_BLOCK_MAP_FLAG_LIST)
	{
	mem_block_list_t list;
	mem_block_list_open(&list, offset);
	offset=mem_block_list_get_item_at(&list, 0);
	}
return offset;
}


// Modification

bool mem_block_map_add_offset(multi_heap_handle_t heap, mem_block_map_t* map, size_t size, size_t offset)
{
size_t heap_start=(size_t)heap+sizeof(multi_heap_t);
size_t heap_end=heap_start+heap->size;
if(offset<heap_start||offset>=heap_end)
	return false;
if(size==0||offset+size>heap_end)
	return false;
if(!map->root)
	{
	map->root=(mem_block_map_group_t*)mem_block_map_item_group_create(heap);
	if(!map->root)
		return false;
	}
bool exists=false;
bool added=mem_block_map_group_add_offset(heap, map->root, size, offset, false, &exists);
mem_block_map_update_root(heap, map);
if(added)
	return true;
if(exists)
	return false;
mem_block_group_lock(map->root);
mem_block_map_parent_group_t* root=mem_block_map_parent_group_create_with_child(heap, map->root);
mem_block_group_unlock(map->root);
if(!root)
	{
	mem_block_map_update_root(heap, map);
	return false;
	}
map->root=(mem_block_map_group_t*)root;
added=mem_block_map_parent_group_add_offset(heap, root, size, offset, true, &exists);
mem_block_map_update_root(heap, map);
return added;
}

bool mem_block_map_remove_offset(multi_heap_handle_t heap, mem_block_map_t* map, size_t size, size_t offset)
{
if(!map->root)
	return false;
bool removed=false;
if(!mem_block_map_group_remove_offset(heap, map->root, size, offset, &removed))
	return false;
if(removed)
	mem_block_map_update_root(heap, map);
return true;
}

void mem_block_map_update_root(multi_heap_handle_t heap, mem_block_map_t* map)
{
mem_block_map_group_t* root=map->root;
if(mem_block_group_is_locked(root))
	return;
uint16_t level=mem_block_group_get_level(root);
uint16_t child_count=mem_block_group_get_child_count(root);
if(level==0)
	{
	if(child_count==0)
		{
		map->root=NULL;
		multi_heap_free_internal(heap, root);
		}
	return;
	}
if(child_count>1)
	return;
mem_block_map_parent_group_t* proot=(mem_block_map_parent_group_t*)root;
map->root=proot->children[0];
multi_heap_free_internal(heap, root);
}


//==========
// Iterator
//==========

// Con-/Destructors

void mem_block_map_it_init(mem_block_map_it_t* it, mem_block_map_t* mem_block_map)
{
it->current=NULL;
it->map=mem_block_map;
it->level_count=0;
}


// Modification

bool mem_block_map_it_find(mem_block_map_it_t* it, size_t size)
{
it->current=NULL;
bool found=true;
mem_block_map_group_t* group=it->map->root;
if(!group)
	return false;
uint16_t level_count=mem_block_group_get_level(group)+1;
if(level_count>CONFIG_HEAP_MAP_MAX_LEVELS)
	{
	MULTI_HEAP_PRINTF("CONFIG_HEAP_MAP_MAX_LEVELS\n");
	return false;
	}
it->level_count=level_count;
for(uint16_t level=0; level<level_count-1; level++)
	{
	mem_block_map_parent_group_t* pgroup=(mem_block_map_parent_group_t*)group;
	int16_t child=mem_block_map_parent_group_get_item_pos(pgroup, size);
	if(child<0)
		{
		found=false;
		child++;
		child*=-1;
		}
	uint16_t pos=(uint16_t)child;
	it->pointers[level].group=group;
	it->pointers[level].pos=pos;
	group=pgroup->children[pos];
	}
mem_block_map_item_group_t* igroup=(mem_block_map_item_group_t*)group;
int16_t child=mem_block_map_item_group_get_item_pos(igroup, size);
if(child<0)
	{
	found=false;
	child++;
	child*=-1;
	}
uint16_t pos=(uint16_t)child;
it->pointers[level_count-1].group=group;
it->pointers[level_count-1].pos=pos;
it->current=mem_block_map_item_group_get_item_at(igroup, pos);
return found;
}

bool mem_block_map_it_move_next(mem_block_map_it_t* it)
{
if(!it->current)
	return false;
uint16_t level_count=it->level_count;
mem_block_map_it_ptr_t* ptr=&it->pointers[level_count-1];
mem_block_map_item_group_t* igroup=(mem_block_map_item_group_t*)ptr->group;
uint16_t child_count=mem_block_group_get_child_count(ptr->group);
if(ptr->pos+1<child_count)
	{
	ptr->pos++;
	it->current=mem_block_map_item_group_get_item_at(igroup, ptr->pos);
	return true;
	}
for(uint16_t level=level_count-1; level>0; level--)
	{
	ptr=&it->pointers[level-1];
	mem_block_map_parent_group_t* pgroup=(mem_block_map_parent_group_t*)ptr->group;
	child_count=mem_block_group_get_child_count(ptr->group);
	if(ptr->pos+1>=child_count)
		continue;
	ptr->pos++;
	mem_block_map_group_t* group=ptr->group;
	for(; level<level_count; level++)
		{
		pgroup=(mem_block_map_parent_group_t*)group;
		group=pgroup->children[ptr->pos];
		ptr=&it->pointers[level];
		ptr->group=group;
		ptr->pos=0;
		}
	igroup=(mem_block_map_item_group_t*)group;
	it->current=mem_block_map_item_group_get_item_at(igroup, 0);
	return true;
	}
it->current=NULL;
return false;
}
