#
# Component Makefile
#

COMPONENT_OBJS := heap_caps_init.o heap_caps.o mem_block.o mem_block_group.o mem_block_list.o mem_block_map.o multi_heap.o


COMPONENT_ADD_LDFRAGMENTS += linker.lf

CFLAGS += -DMULTI_HEAP_FREERTOS
