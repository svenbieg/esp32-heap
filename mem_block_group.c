//===================
// mem_block_group.c
//===================

// Copyright 2021, Sven Bieg (svenbieg@web.de)
// http://github.com/svenbieg/esp32-heap


//=======
// Using
//=======

#include "mem_block_group.h"
#include "multi_heap_platform.h"


//================
// Initialization
//================

void mem_block_group_init(mem_block_group_t* group, uint16_t level, uint16_t child_count)
{
// 16bit operations are not allowed in IRAM
mem_block_group_t set;
set.level=level;
set.child_count=child_count;
uint32_t* src=(uint32_t*)&set;
uint32_t* dst=(uint32_t*)group;
dst[0]=src[0];
}


//========
// Access
//========

uint16_t mem_block_group_get_child_count(mem_block_group_t* group)
{
mem_block_group_t get={ 0, 0 };
uint32_t* src=(uint32_t*)group;
uint32_t* dst=(uint32_t*)&get;
dst[0]=src[0];
return get.child_count;
}

uint16_t mem_block_group_get_level(mem_block_group_t* group)
{
mem_block_group_t get={ 0, 0 };
uint32_t* src=(uint32_t*)group;
uint32_t* dst=(uint32_t*)&get;
dst[0]=src[0];
return get.level&MEM_BLOCK_GROUP_LEVEL_MASK;
}

bool mem_block_group_is_dirty(mem_block_group_t* group)
{
mem_block_group_t get={ 0, 0 };
uint32_t* src=(uint32_t*)group;
uint32_t* dst=(uint32_t*)&get;
dst[0]=src[0];
return (get.level&MEM_BLOCK_GROUP_FLAG_DIRTY)>0;
}

bool mem_block_group_is_locked(mem_block_group_t* group)
{
mem_block_group_t get={ 0, 0 };
uint32_t* src=(uint32_t*)group;
uint32_t* dst=(uint32_t*)&get;
dst[0]=src[0];
return (get.level&MEM_BLOCK_GROUP_FLAG_LOCKED)>0;
}


//==============
// Modification
//==============

void mem_block_group_lock(mem_block_group_t* group)
{
mem_block_group_t set={ 0, 0 };
uint32_t* src=(uint32_t*)&set;
uint32_t* dst=(uint32_t*)group;
src[0]=dst[0];
set.level|=MEM_BLOCK_GROUP_FLAG_LOCKED;
dst[0]=src[0];
}

void mem_block_group_set_child_count(mem_block_group_t* group, uint16_t child_count)
{
mem_block_group_t set={ 0, 0 };
uint32_t* src=(uint32_t*)&set;
uint32_t* dst=(uint32_t*)group;
src[0]=dst[0];
set.child_count=child_count;
dst[0]=src[0];
}

void mem_block_group_set_clean(mem_block_group_t* group)
{
mem_block_group_t set={ 0, 0 };
uint32_t* src=(uint32_t*)&set;
uint32_t* dst=(uint32_t*)group;
src[0]=dst[0];
set.level&=~MEM_BLOCK_GROUP_FLAG_DIRTY;
dst[0]=src[0];
}

void mem_block_group_set_dirty(mem_block_group_t* group)
{
mem_block_group_t set={ 0, 0 };
uint32_t* src=(uint32_t*)&set;
uint32_t* dst=(uint32_t*)group;
src[0]=dst[0];
set.level|=MEM_BLOCK_GROUP_FLAG_DIRTY;
dst[0]=src[0];
}

void mem_block_group_unlock(mem_block_group_t* group)
{
mem_block_group_t set={ 0, 0 };
uint32_t* src=(uint32_t*)&set;
uint32_t* dst=(uint32_t*)group;
src[0]=dst[0];
set.level&=~MEM_BLOCK_GROUP_FLAG_LOCKED;
dst[0]=src[0];
}
