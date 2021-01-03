//==================
// mem_block_list.c
//==================

// Sorted offsets of free memory-blocks with the same size

// Copyright 2021, Sven Bieg (svenbieg@web.de)
// http://github.com/svenbieg/esp32-heap


//=======
// Using
//=======

#include "mem_block.h"
#include "mem_block_list.h"
#include "multi_heap_internal.h"
#include "multi_heap_platform.h"


//=======
// Group
//=======

// Con-/Destructors

void mem_block_list_group_destroy(multi_heap_handle_t heap, mem_block_list_group_t* group)
{
if(mem_block_group_get_level(group)==0)
	{
	multi_heap_free_internal(heap, group);
	return;
	}
mem_block_list_parent_group_destroy(heap, (mem_block_list_parent_group_t*)group);
}


// Access

bool mem_block_list_group_check(multi_heap_handle_t heap, mem_block_list_group_t* group, bool print_errors)
{
if(mem_block_group_get_level(group)==0)
	return mem_block_list_item_group_check(heap, (mem_block_list_item_group_t*)group, print_errors);
return mem_block_list_parent_group_check(heap, (mem_block_list_parent_group_t*)group, print_errors);
}

void mem_block_list_group_dump(mem_block_list_group_t* group)
{
if(mem_block_group_get_level(group)==0)
	{
	mem_block_list_item_group_dump((mem_block_list_item_group_t*)group);
	return;
	}
mem_block_list_parent_group_dump((mem_block_list_parent_group_t*)group);
}

size_t* mem_block_list_group_get_first_item(mem_block_list_group_t* group)
{
if(mem_block_group_get_level(group)==0)
	return mem_block_list_item_group_get_first_item((mem_block_list_item_group_t*)group);
mem_block_list_parent_group_t* pgroup=(mem_block_list_parent_group_t*)group;
return pgroup->first;
}

size_t* mem_block_list_group_get_item(mem_block_list_group_t* group, size_t offset)
{
if(mem_block_group_get_level(group)==0)
	return mem_block_list_item_group_get_item((mem_block_list_item_group_t*)group, offset);
return mem_block_list_parent_group_get_item((mem_block_list_parent_group_t*)group, offset);
}

size_t* mem_block_list_group_get_item_at(mem_block_list_group_t* group, size_t pos)
{
if(mem_block_group_get_level(group)==0)
	return mem_block_list_item_group_get_item_at((mem_block_list_item_group_t*)group, pos);
return mem_block_list_parent_group_get_item_at((mem_block_list_parent_group_t*)group, pos);
}

size_t mem_block_list_group_get_item_count(mem_block_list_group_t* group)
{
if(mem_block_group_get_level(group)==0)
	return group->child_count;
mem_block_list_parent_group_t* pgroup=(mem_block_list_parent_group_t*)group;
return pgroup->item_count;
}

size_t* mem_block_list_group_get_last_item(mem_block_list_group_t* group)
{
if(mem_block_group_get_level(group)==0)
	return mem_block_list_item_group_get_last_item((mem_block_list_item_group_t*)group);
mem_block_list_parent_group_t* pgroup=(mem_block_list_parent_group_t*)group;
return pgroup->last;
}


// Modification

bool mem_block_list_group_add_item(multi_heap_handle_t heap, mem_block_list_group_t* group, size_t offset, bool again, bool* exists)
{
if(mem_block_group_get_level(group)==0)
	return mem_block_list_item_group_add_item((mem_block_list_item_group_t*)group, offset, exists);
return mem_block_list_parent_group_add_item(heap, (mem_block_list_parent_group_t*)group, offset, again, exists);
}

bool mem_block_list_group_remove_item(multi_heap_handle_t heap, mem_block_list_group_t* group, size_t offset)
{
if(mem_block_group_get_level(group)==0)
	return mem_block_list_item_group_remove_item((mem_block_list_item_group_t*)group, offset);
return mem_block_list_parent_group_remove_item(heap, (mem_block_list_parent_group_t*)group, offset);
}


//============
// Item-group
//============

// Con-/Destructors

mem_block_list_item_group_t* mem_block_list_item_group_create(multi_heap_handle_t heap)
{
mem_block_list_item_group_t* group=(mem_block_list_item_group_t*)multi_heap_malloc_internal(heap, sizeof(mem_block_list_item_group_t));
if(group==NULL)
	return NULL;
mem_block_group_init((mem_block_group_t*)group, 0, 0);
return group;
}


// Access

bool mem_block_list_item_group_check(multi_heap_handle_t heap, mem_block_list_item_group_t* group, bool print_errors)
{
uint16_t child_count=mem_block_group_get_child_count((mem_block_group_t*)group);
if(!child_count)
	{
	if(print_errors)
		{
		MULTI_HEAP_PRINTF("multi_heap_check(0x%x): empty list-item-group 0x%x\n", heap, group);
		}
	return false;
	}
bool success=true;
size_t heap_start=(size_t)heap+sizeof(multi_heap_t);
size_t heap_end=heap_start+heap->size;
size_t last_offset=0;
for(uint16_t pos=0; pos<child_count; pos++)
	{
	size_t offset=group->items[pos];
	if(offset<heap_start||offset>=heap_end)
		{
		if(print_errors)
			{
			MULTI_HEAP_PRINTF("multi_heap_check(0x%x): offset outside heap 0x%x\n", heap, offset);
			}
		success=false;
		continue;
		}
	if(offset<=last_offset)
		{
		if(print_errors)
			{
			MULTI_HEAP_PRINTF("multi_heap_check(0x%x): offset 0x%x<=0x%x\n", heap, offset, last_offset);
			}
		success=false;
		continue;
		}
	last_offset=offset;
	}
return success;
}

void mem_block_list_item_group_dump(mem_block_list_item_group_t* group)
{
uint16_t child_count=mem_block_group_get_child_count((mem_block_group_t*)group);
for(uint16_t pos=0; pos<child_count; pos++)
	{
	MULTI_HEAP_PRINTF(" 0x%x", group->items[pos]);
	}
}

size_t* mem_block_list_item_group_get_first_item(mem_block_list_item_group_t* group)
{
if(mem_block_group_get_child_count((mem_block_group_t*)group)==0)
	return NULL;
return group->items;
}

uint16_t mem_block_list_item_group_get_insert_pos(mem_block_list_item_group_t* group, size_t offset, bool* exists)
{
uint16_t child_count=mem_block_group_get_child_count((mem_block_group_t*)group);
uint16_t start=0;
uint16_t end=child_count;
while(start<end)
	{
	uint16_t pos=start+(end-start)/2;
	if(group->items[pos]>offset)
		{
		end=pos;
		continue;
		}
	if(group->items[pos]<offset)
		{
		start=pos+1;
		continue;
		}
	*exists=true;
	return pos;
	}
return start;
}

size_t* mem_block_list_item_group_get_item(mem_block_list_item_group_t* group, size_t offset)
{
int16_t pos=mem_block_list_item_group_get_item_pos(group, offset);
if(pos<0)
	return NULL;
return &group->items[pos];
}

size_t* mem_block_list_item_group_get_item_at(mem_block_list_item_group_t* group, size_t pos)
{
if(pos>=mem_block_group_get_child_count((mem_block_group_t*)group))
	return NULL;
return &group->items[pos];
}

int16_t mem_block_list_item_group_get_item_pos(mem_block_list_item_group_t* group, size_t offset)
{
uint16_t start=0;
uint16_t end=mem_block_group_get_child_count((mem_block_group_t*)group);
uint16_t pos=0;
while(start<end)
	{
	pos=start+(end-start)/2;
	size_t item=group->items[pos];
	if(item>offset)
		{
		end=pos;
		continue;
		}
	if(item<offset)
		{
		start=pos+1;
		continue;
		}
	return pos;
	}
return -(int16_t)pos-1;
}

size_t* mem_block_list_item_group_get_last_item(mem_block_list_item_group_t* group)
{
uint16_t count=mem_block_group_get_child_count((mem_block_group_t*)group);
if(count==0)
	return NULL;
return &group->items[count-1];
}


// Modification

bool mem_block_list_item_group_add_item(mem_block_list_item_group_t* group, size_t offset, bool* exists)
{
uint16_t pos=mem_block_list_item_group_get_insert_pos(group, offset, exists);
if(*exists)
	return false;
return mem_block_list_item_group_add_item_internal(group, offset, pos);
}

bool mem_block_list_item_group_add_item_internal(mem_block_list_item_group_t* group, size_t offset, uint16_t pos)
{
uint16_t child_count=mem_block_group_get_child_count((mem_block_group_t*)group);
if(child_count==CONFIG_HEAP_GROUP_SIZE)
	return false;
for(uint16_t u=child_count; u>pos; u--)
	group->items[u]=group->items[u-1];
group->items[pos]=offset;
mem_block_group_set_child_count((mem_block_group_t*)group, child_count+1);
return true;
}

void mem_block_list_item_group_append_items(mem_block_list_item_group_t* group, size_t const* items, uint16_t count)
{
uint16_t child_count=mem_block_group_get_child_count((mem_block_group_t*)group);
for(uint16_t u=0; u<count; u++)
	group->items[child_count+u]=items[u];
mem_block_group_set_child_count((mem_block_group_t*)group, child_count+count);
}

void mem_block_list_item_group_insert_items(mem_block_list_item_group_t* group, uint16_t pos, size_t const* items, uint16_t count)
{
uint16_t child_count=mem_block_group_get_child_count((mem_block_group_t*)group);
for(uint16_t u=child_count+count-1; u>=pos+count; u--)
	group->items[u]=group->items[u-count];
for(uint16_t u=0; u<count; u++)
	group->items[pos+u]=items[u];
mem_block_group_set_child_count((mem_block_group_t*)group, child_count+count);
}

bool mem_block_list_item_group_remove_item(mem_block_list_item_group_t* group, size_t offset)
{
int16_t pos=mem_block_list_item_group_get_item_pos(group, offset);
if(pos<0)
	return false;
return mem_block_list_item_group_remove_item_at(group, pos);
}

bool mem_block_list_item_group_remove_item_at(mem_block_list_item_group_t* group, size_t at)
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

void mem_block_list_item_group_remove_items(mem_block_list_item_group_t* group, uint16_t pos, uint16_t count)
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

mem_block_list_parent_group_t* mem_block_list_parent_group_create(multi_heap_handle_t heap, uint16_t level)
{
mem_block_list_parent_group_t* group=(mem_block_list_parent_group_t*)multi_heap_malloc_internal(heap, sizeof(mem_block_list_parent_group_t));
if(group==NULL)
	return NULL;
mem_block_group_init((mem_block_group_t*)group, level, 0);
group->first=NULL;
group->last=NULL;
group->item_count=0;
return group;
}

mem_block_list_parent_group_t* mem_block_list_parent_group_create_with_child(multi_heap_handle_t heap, mem_block_list_group_t* child)
{
mem_block_list_parent_group_t* group=(mem_block_list_parent_group_t*)multi_heap_malloc_internal(heap, sizeof(mem_block_list_parent_group_t));
if(group==NULL)
	return NULL;
uint16_t child_level=mem_block_group_get_level(child);
mem_block_group_init((mem_block_group_t*)group, child_level+1, 1);
group->first=mem_block_list_group_get_first_item(child);
group->last=mem_block_list_group_get_last_item(child);
group->item_count=mem_block_list_group_get_item_count(child);
group->children[0]=child;
return group;
}

void mem_block_list_parent_group_destroy(multi_heap_handle_t heap, mem_block_list_parent_group_t* group)
{
uint16_t child_count=mem_block_group_get_child_count((mem_block_group_t*)group);
for(uint16_t u=0; u<child_count; u++)
	mem_block_list_group_destroy(heap, group->children[u]);
multi_heap_free_internal(heap, group);
}


// Access

bool mem_block_list_parent_group_check(multi_heap_handle_t heap, mem_block_list_parent_group_t* group, bool print_errors)
{
uint16_t child_count=mem_block_group_get_child_count((mem_block_group_t*)group);
if(!child_count)
	{
	if(print_errors)
		{
		MULTI_HEAP_PRINTF("multi_heap_check(0x%x): empty list-parent-group\n", heap);
		}
	return false;
	}
bool success=true;
for(uint16_t pos=0; pos<child_count; pos++)
	{
	if(!mem_block_list_group_check(heap, group->children[pos], print_errors))
		success=false;
	}
return success;
}

void mem_block_list_parent_group_dump(mem_block_list_parent_group_t* group)
{
uint16_t child_count=mem_block_group_get_child_count((mem_block_group_t*)group);
for(uint16_t pos=0; pos<child_count; pos++)
	mem_block_list_group_dump(group->children[pos]);
}

int16_t mem_block_list_parent_group_get_group(mem_block_list_parent_group_t* group, size_t* pos)
{
uint16_t child_count=mem_block_group_get_child_count((mem_block_group_t*)group);
for(uint16_t u=0; u<child_count; u++)
	{
	size_t item_count=mem_block_list_group_get_item_count(group->children[u]);
	if(*pos<item_count)
		return u;
	*pos-=item_count;
	}
return -1;
}

uint16_t mem_block_list_parent_group_get_insert_pos(mem_block_list_parent_group_t* group, size_t offset, uint16_t* insert_pos, bool* exists)
{
uint16_t child_count=mem_block_group_get_child_count((mem_block_group_t*)group);
if(!child_count)
	return 0;
uint16_t start=0;
uint16_t end=child_count;
size_t* first=NULL;
size_t* last=NULL;
int16_t empty=0;
while(start<end)
	{
	uint16_t pos=start+(end-start)/2+empty;
	first=mem_block_list_group_get_first_item(group->children[pos]);
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
	last=mem_block_list_group_get_last_item(group->children[pos]);
	if(*first==offset||*last==offset)
		{
		*exists=true;
		*insert_pos=pos;
		return 1;
		}
	if(*first>offset)
		{
		end=pos;
		continue;
		}
	if(*last<offset)
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
	first=mem_block_list_group_get_first_item(group->children[start]);
	if(first==NULL||*first>=offset)
		{
		*insert_pos=start-1;
		return 2;
		}
	}
if(start+1<child_count)
	{
	last=mem_block_list_group_get_last_item(group->children[start]);
	if(last==0||*last<=offset)
		return 2;
	}
return 1;
}

size_t* mem_block_list_parent_group_get_item(mem_block_list_parent_group_t* group, size_t offset)
{
int16_t child=mem_block_list_parent_group_get_item_pos(group, offset);
if(child<0)
	return NULL;
return mem_block_list_group_get_item(group->children[child], offset);
}

size_t* mem_block_list_parent_group_get_item_at(mem_block_list_parent_group_t* group, size_t pos)
{
int16_t child=mem_block_list_parent_group_get_group(group, &pos);
if(child<0)
	return NULL;
return mem_block_list_group_get_item_at(group->children[child], pos);
}

int16_t mem_block_list_parent_group_get_item_pos(mem_block_list_parent_group_t* group, size_t offset)
{
uint16_t start=0;
uint16_t end=mem_block_group_get_child_count((mem_block_group_t*)group);
uint16_t pos=0;
size_t* first=NULL;
size_t* last=NULL;
int16_t empty=0;
while(start<end)
	{
	pos=start+(end-start)/2+empty;
	first=mem_block_list_group_get_first_item(group->children[pos]);
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
	if(*first>offset)
		{
		end=pos;
		continue;
		}
	last=mem_block_list_group_get_last_item(group->children[pos]);
	if(*last<offset)
		{
		start=pos+1;
		continue;
		}
	return pos;
	}
return -(int16_t)pos-1;
}

int16_t mem_block_list_parent_group_get_nearest_space(mem_block_list_parent_group_t* group, int16_t pos)
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

bool mem_block_list_parent_group_add_item(multi_heap_handle_t heap, mem_block_list_parent_group_t* group, size_t offset, bool again, bool* exists)
{
mem_block_group_lock((mem_block_group_t*)group);
bool added=mem_block_list_parent_group_add_item_internal(heap, group, offset, again, exists);
if(added)
	{
	group->item_count++;
	mem_block_list_parent_group_update_bounds(group);
	}
if(mem_block_group_is_dirty((mem_block_group_t*)group))
	{
	mem_block_list_parent_group_combine_children(heap, group);
	mem_block_group_set_clean((mem_block_group_t*)group);
	}
mem_block_group_unlock((mem_block_group_t*)group);
return added;
}

bool mem_block_list_parent_group_add_item_internal(multi_heap_handle_t heap, mem_block_list_parent_group_t* group, size_t offset, bool again, bool* exists)
{
uint16_t pos=0;
uint16_t count=mem_block_list_parent_group_get_insert_pos(group, offset, &pos, exists);
if(*exists)
	return false;
if(!again)
	{
	for(uint16_t u=0; u<count; u++)
		{
		if(mem_block_list_group_add_item(heap, group->children[pos+u], offset, false, exists))
			return true;
		if(*exists)
			return false;
		}
	if(mem_block_list_parent_group_shift_children(group, pos, count))
		{
		count=mem_block_list_parent_group_get_insert_pos(group, offset, &pos, exists);
		for(uint16_t u=0; u<count; u++)
			{
			if(mem_block_list_group_add_item(heap, group->children[pos+u], offset, false, exists))
				return true;
			}
		}
	}
if(!mem_block_list_parent_group_split_child(heap, group, pos))
	{
	mem_block_group_unlock((mem_block_group_t*)group);
	return false;
	}
count=mem_block_list_parent_group_get_insert_pos(group, offset, &pos, exists);
for(uint16_t u=0; u<count; u++)
	{
	if(mem_block_list_group_add_item(heap, group->children[pos+u], offset, true, exists))
		return true;
	}
return false;
}

void mem_block_list_parent_group_append_groups(mem_block_list_parent_group_t* group, mem_block_list_group_t* const* groups, uint16_t count)
{
uint16_t child_count=mem_block_group_get_child_count((mem_block_group_t*)group);
for(uint16_t u=0; u<count; u++)
	{
	group->children[child_count+u]=groups[u];
	group->item_count+=mem_block_list_group_get_item_count(groups[u]);
	}
mem_block_group_set_child_count((mem_block_group_t*)group, child_count+count);
mem_block_list_parent_group_update_bounds(group);
}

bool mem_block_list_parent_group_combine_child(multi_heap_handle_t heap, mem_block_list_parent_group_t* group, uint16_t pos)
{
uint16_t count=mem_block_group_get_child_count(group->children[pos]);
if(count==0)
	{
	mem_block_list_parent_group_remove_group(heap, group, pos);
	return true;
	}
if(pos>0)
	{
	uint16_t before=mem_block_group_get_child_count(group->children[pos-1]);
	if(count+before<=CONFIG_HEAP_GROUP_SIZE)
		{
		mem_block_list_parent_group_move_children(group, pos, pos-1, count);
		mem_block_list_parent_group_remove_group(heap, group, pos);
		return true;
		}
	}
uint16_t child_count=mem_block_group_get_child_count((mem_block_group_t*)group);
if(pos+1<child_count)
	{
	uint16_t after=mem_block_group_get_child_count(group->children[pos+1]);
	if(count+after<=CONFIG_HEAP_GROUP_SIZE)
		{
		mem_block_list_parent_group_move_children(group, pos+1, pos, after);
		mem_block_list_parent_group_remove_group(heap, group, pos+1);
		return true;
		}
	}
return false;
}

bool mem_block_list_parent_group_combine_children(multi_heap_handle_t heap, mem_block_list_parent_group_t* group)
{
bool combined=false;
uint16_t child_count=mem_block_group_get_child_count((mem_block_group_t*)group);
for(uint16_t pos=0; pos<child_count; )
	{
	if(mem_block_list_parent_group_combine_child(heap, group, pos))
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

void mem_block_list_parent_group_insert_groups(mem_block_list_parent_group_t* group, uint16_t pos, mem_block_list_group_t* const* groups, uint16_t count)
{
uint16_t child_count=mem_block_group_get_child_count((mem_block_group_t*)group);
for(uint16_t u=child_count+count-1; u>=pos+count; u--)
	group->children[u]=group->children[u-count];
for(uint16_t u=0; u<count; u++)
	{
	group->children[pos+u]=groups[u];
	group->item_count+=mem_block_list_group_get_item_count(groups[u]);
	}
mem_block_group_set_child_count((mem_block_group_t*)group, child_count+count);
mem_block_list_parent_group_update_bounds(group);
}

void mem_block_list_parent_group_move_children(mem_block_list_parent_group_t* group, uint16_t from, uint16_t to, uint16_t count)
{
uint16_t level=mem_block_group_get_level((mem_block_group_t*)group);
if(level>1)
	{
	mem_block_list_parent_group_t* src=(mem_block_list_parent_group_t*)group->children[from];
	mem_block_list_parent_group_t* dst=(mem_block_list_parent_group_t*)group->children[to];
	if(from>to)
		{
		mem_block_list_parent_group_append_groups(dst, src->children, count);
		mem_block_list_parent_group_remove_groups(src, 0, count);
		}
	else
		{
		uint16_t src_count=mem_block_group_get_child_count((mem_block_group_t*)src);
		mem_block_list_parent_group_insert_groups(dst, 0, &src->children[src_count-count], count);
		mem_block_list_parent_group_remove_groups(src, src_count-count, count);
		}
	}
else
	{
	mem_block_list_item_group_t* src=(mem_block_list_item_group_t*)group->children[from];
	mem_block_list_item_group_t* dst=(mem_block_list_item_group_t*)group->children[to];
	if(from>to)
		{
		mem_block_list_item_group_append_items(dst, src->items, count);
		mem_block_list_item_group_remove_items(src, 0, count);
		}
	else
		{
		uint16_t src_count=mem_block_group_get_child_count((mem_block_group_t*)src);
		mem_block_list_item_group_insert_items(dst, 0, &src->items[src_count-count], count);
		mem_block_list_item_group_remove_items(src, src_count-count, count);
		}
	}
}

void mem_block_list_parent_group_move_space(mem_block_list_parent_group_t* group, uint16_t from, uint16_t to)
{
if(from<to)
	{
	for(uint16_t u=from; u<to; u++)
		mem_block_list_parent_group_move_children(group, u+1, u, 1);
	}
else
	{
	for(uint16_t u=from; u>to; u--)
		mem_block_list_parent_group_move_children(group, u-1, u, 1);
	}
}

void mem_block_list_parent_group_remove_group(multi_heap_handle_t heap, mem_block_list_parent_group_t* group, uint16_t pos)
{
uint16_t child_count=mem_block_group_get_child_count((mem_block_group_t*)group);
mem_block_list_group_t* child=group->children[pos];
for(uint16_t u=pos; u+1<child_count; u++)
	group->children[u]=group->children[u+1];
mem_block_group_set_child_count((mem_block_group_t*)group, child_count-1);
multi_heap_free_internal(heap, child);
}

void mem_block_list_parent_group_remove_groups(mem_block_list_parent_group_t* group, uint16_t pos, uint16_t count)
{
for(uint16_t u=0; u<count; u++)
	group->item_count-=mem_block_list_group_get_item_count(group->children[pos+u]);
uint16_t child_count=mem_block_group_get_child_count((mem_block_group_t*)group);
for(uint16_t u=pos; u+count<child_count; u++)
	group->children[u]=group->children[u+count];
mem_block_group_set_child_count((mem_block_group_t*)group, child_count-count);
mem_block_list_parent_group_update_bounds(group);
}

bool mem_block_list_parent_group_remove_item(multi_heap_handle_t heap, mem_block_list_parent_group_t* group, size_t offset)
{
int16_t pos=mem_block_list_parent_group_get_item_pos(group, offset);
if(pos<0)
	return false;
if(!mem_block_list_group_remove_item(heap, group->children[pos], offset))
	return false;
group->item_count--;
mem_block_list_parent_group_update_bounds(group);
if(mem_block_group_is_locked((mem_block_group_t*)group))
	{
	mem_block_group_set_dirty((mem_block_group_t*)group);
	}
else
	{
	mem_block_list_parent_group_combine_child(heap, group, pos);
	}
return true;
}

bool mem_block_list_parent_group_shift_children(mem_block_list_parent_group_t* group, uint16_t pos, uint16_t count)
{
int16_t space=mem_block_list_parent_group_get_nearest_space(group, pos);
if(space<0)
	return false;
if(count>1&&space>pos)
	pos++;
mem_block_list_parent_group_move_space(group, space, pos);
return true;
}

bool mem_block_list_parent_group_split_child(multi_heap_handle_t heap, mem_block_list_parent_group_t* group, uint16_t pos)
{
uint16_t child_count=mem_block_group_get_child_count((mem_block_group_t*)group);
if(child_count==CONFIG_HEAP_GROUP_SIZE)
	return false;
mem_block_list_group_t* child=NULL;
uint16_t level=mem_block_group_get_level((mem_block_group_t*)group);
if(level>1)
	{
	child=(mem_block_list_group_t*)mem_block_list_parent_group_create(heap, level-1);
	}
else
	{
	child=(mem_block_list_group_t*)mem_block_list_item_group_create(heap);
	}
if(!child)
	return false;
for(uint16_t u=child_count; u>pos+1; u--)
	group->children[u]=group->children[u-1];
group->children[pos+1]=child;
mem_block_group_set_child_count((mem_block_group_t*)group, child_count+1);
mem_block_list_parent_group_move_children(group, pos, pos+1, 1);
return true;
}

void mem_block_list_parent_group_update_bounds(mem_block_list_parent_group_t* group)
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
	group->first=mem_block_list_group_get_first_item(group->children[pos]);
	if(group->first!=NULL)
		break;
	}
for(uint16_t pos=child_count; pos>0; pos--)
	{
	group->last=mem_block_list_group_get_last_item(group->children[pos-1]);
	if(group->last!=NULL)
		break;
	}
}


//======
// List
//======

// Con-/Destructors

void mem_block_list_destroy(multi_heap_handle_t heap, mem_block_list_t* list)
{
mem_block_list_group_t* root=list->root;
if(!root)
	return;
list->root=NULL;
mem_block_list_group_destroy(heap, root);
}

void mem_block_list_init(mem_block_list_t* list)
{
list->root=NULL;
}

void mem_block_list_open(mem_block_list_t* list, size_t offset)
{
list->root=(mem_block_group_t*)offset;
}


// Access

bool mem_block_list_check(multi_heap_handle_t heap, mem_block_list_t* list, bool print_errors)
{
if(!list->root)
	{
	if(print_errors)
		{
		MULTI_HEAP_PRINTF("multi_heap_check(0x%x): empty list\n", heap);
		}
	return false;
	}
return mem_block_list_group_check(heap, list->root, print_errors);
}

void mem_block_list_dump(mem_block_list_t* list)
{
if(list->root)
	mem_block_list_group_dump(list->root);
}

size_t mem_block_list_get_item(mem_block_list_t* list, size_t offset)
{
if(!list->root)
	return 0;
size_t* item=mem_block_list_group_get_item(list->root, offset);
if(!item)
	return 0;
return *item;
}

size_t mem_block_list_get_item_at(mem_block_list_t* list, size_t pos)
{
size_t* item=mem_block_list_group_get_item_at(list->root, pos);
if(!item)
	return 0;
return *item;
}

size_t mem_block_list_get_item_count(mem_block_list_t* list)
{
if(!list->root)
	return 0;
return mem_block_list_group_get_item_count(list->root);
}


// Modification

bool mem_block_list_add_offset(multi_heap_handle_t heap, mem_block_list_t* list, size_t offset)
{
if(!list->root)
	{
	list->root=(mem_block_list_group_t*)mem_block_list_item_group_create(heap);
	if(!list->root)
		return false;
	}
bool exists=false;
bool added=mem_block_list_group_add_item(heap, list->root, offset, false, &exists);
mem_block_list_update_root(heap, list);
if(added)
	return true;
if(exists)
	return false;
mem_block_group_lock(list->root);
mem_block_list_parent_group_t* root=mem_block_list_parent_group_create_with_child(heap, list->root);
mem_block_group_unlock(list->root);
if(!root)
	{
	mem_block_list_update_root(heap, list);
	return false;
	}
list->root=(mem_block_list_group_t*)root;
added=mem_block_list_parent_group_add_item(heap, root, offset, true, &exists);
mem_block_list_update_root(heap, list);
return added;
}

bool mem_block_list_remove_offset(multi_heap_handle_t heap, mem_block_list_t* list, size_t offset)
{
if(!list->root)
	return false;
if(!mem_block_list_group_remove_item(heap, list->root, offset))
	return false;
mem_block_list_update_root(heap, list);
return true;
}

void mem_block_list_update_root(multi_heap_handle_t heap, mem_block_list_t* list)
{
mem_block_list_group_t* root=list->root;
if(mem_block_group_is_locked(root))
	return;
uint16_t level=mem_block_group_get_level(root);
uint16_t child_count=mem_block_group_get_child_count(root);
if(level==0)
	{
	if(child_count==0)
		{
		list->root=NULL;
		multi_heap_free_internal(heap, root);
		}
	return;
	}
if(child_count>1)
	return;
mem_block_list_parent_group_t* proot=(mem_block_list_parent_group_t*)root;
list->root=proot->children[0];
multi_heap_free_internal(heap, root);
}
