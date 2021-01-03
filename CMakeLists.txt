set(srcs 
    "heap_caps.c"
    "heap_caps_init.c"
    "mem_block.c"
    "mem_block_group.c"
    "mem_block_list.c"
    "mem_block_map.c"
    "multi_heap.c")

if(CONFIG_HEAP_TRACING_STANDALONE)
    list(APPEND srcs "heap_trace_standalone.c")
    set_source_files_properties(heap_trace_standalone.c
        PROPERTIES COMPILE_FLAGS
        -Wno-frame-address)
endif()

idf_component_register(SRCS "${srcs}"
                    INCLUDE_DIRS include
                    LDFRAGMENTS linker.lf
                    PRIV_REQUIRES soc)

if(CONFIG_HEAP_TRACING)
    set(WRAP_FUNCTIONS
        calloc
        malloc
        free
        realloc
        heap_caps_malloc
        heap_caps_free
        heap_caps_realloc
        heap_caps_malloc_default
        heap_caps_realloc_default)

    foreach(wrap ${WRAP_FUNCTIONS})
        target_link_libraries(${COMPONENT_LIB} INTERFACE "-Wl,--wrap=${wrap}")
    endforeach()
endif()

if(NOT CMAKE_BUILD_EARLY_EXPANSION)
    idf_build_get_property(build_components BUILD_COMPONENTS)
    if(freertos IN_LIST build_components)
        target_compile_options(${COMPONENT_TARGET} PRIVATE "-DMULTI_HEAP_FREERTOS")
    endif()
endif()
