#
# Component Makefile
#

COMPONENT_OBJS := heap_caps_init.o heap_caps.o mem_block.o mem_block_group.o mem_block_list.o mem_block_map.o multi_heap.o

ifdef CONFIG_HEAP_TRACING

WRAP_FUNCTIONS = calloc malloc free realloc heap_caps_malloc heap_caps_free heap_caps_realloc heap_caps_malloc_default heap_caps_realloc_default
WRAP_ARGUMENT := -Wl,--wrap=

COMPONENT_ADD_LDFLAGS = -l$(COMPONENT_NAME) $(addprefix $(WRAP_ARGUMENT),$(WRAP_FUNCTIONS))

endif

COMPONENT_ADD_LDFRAGMENTS += linker.lf

CFLAGS += -DMULTI_HEAP_FREERTOS
