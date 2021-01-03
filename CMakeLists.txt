set(srcs 
    "heap_caps.c"
    "heap_caps_init.c"
    "mem_block.c"
    "mem_block_group.c"
    "mem_block_list.c"
    "mem_block_map.c"
    "multi_heap.c")

idf_component_register(SRCS "${srcs}"
                    INCLUDE_DIRS include
                    LDFRAGMENTS linker.lf
                    PRIV_REQUIRES soc)

if(NOT CMAKE_BUILD_EARLY_EXPANSION)
    idf_build_get_property(build_components BUILD_COMPONENTS)
    if(freertos IN_LIST build_components)
        target_compile_options(${COMPONENT_TARGET} PRIVATE "-DMULTI_HEAP_FREERTOS")
    endif()
endif()
