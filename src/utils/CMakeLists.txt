# ----------------------------------------
# Base sources (headers excluded)
# ----------------------------------------
set(COMMON_SOURCES
    objmap.c
    pooled_list.c
    unmap.c
    tag_table.c
    kdtree.c
    avltree.c
    string_map.c
    settings.c
    banner.c
)

# ----------------------------------------
# Optional: MEMLOG support
# ----------------------------------------
if(ENABLE_MEMLOG)
    list(APPEND COMMON_SOURCES memlog.c)
endif()

# ----------------------------------------
# Optional: SQLITE support (COMPILE_SQLITE)
# ----------------------------------------
set(SQLITE_LIB "")
if(ENABLE_MEMLOG)  # matches original logic
    # sqlite is only used when memlog is enabled
    add_library(mb_utils_sqlite3 STATIC sqlite3.c)

    target_compile_definitions(mb_utils_sqlite3 PRIVATE
        SQLITE_THREADSAFE=0
        SQLITE_OMIT_LOAD_EXTENSION
    )

    target_include_directories(mb_utils_sqlite3 PUBLIC ${PROJECT_SOURCE_DIR}/include)

    set(SQLITE_LIB mb_utils_sqlite3)
endif()

# ----------------------------------------
# Parallel dependencies
# ----------------------------------------
if(ENABLE_PARALLEL)
    find_package(MPI REQUIRED)
    find_package(Threads REQUIRED)

    set(PARALLEL_LIBS MPI::MPI_C Threads::Threads)
endif()

# ----------------------------------------
# Production utils (lib_utils)
# ----------------------------------------
add_library(mb_utils_serial STATIC ${COMMON_SOURCES})

add_library(libmboard::mb_utils_serial ALIAS mb_utils_serial)

target_compile_options(mb_utils_serial PRIVATE
    $<$<CONFIG:Debug>:${DEBUG_CFLAGS}>
    $<$<CONFIG:Release>:${PRODUCTION_CFLAGS}>
)

target_include_directories(mb_utils_serial
    PUBLIC
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
        $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/generated>
        $<INSTALL_INTERFACE:include>
)

target_link_libraries(mb_utils_serial
    ${SQLITE_LIB}
    m
)

target_compile_definitions(mb_utils_serial
    PUBLIC PACKAGE_VERSION="${PROJECT_VERSION}"
)

# ----------------------------------------
# Parallel production utils (lib_utils_p)
# ----------------------------------------
if(ENABLE_PARALLEL)

    add_library(mb_utils_parallel STATIC ${COMMON_SOURCES})

    add_library(libmboard::mb_utils_parallel ALIAS mb_utils_parallel)

    target_include_directories(mb_utils_parallel
        PUBLIC
            $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
            $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/generated>
            $<INSTALL_INTERFACE:include>
    )

    target_compile_options(mb_utils_parallel PRIVATE
        $<$<CONFIG:Debug>:${DEBUG_CFLAGS}>
        $<$<CONFIG:Release>:${PRODUCTION_CFLAGS}>
    )

    target_link_libraries(mb_utils_parallel
        ${SQLITE_LIB}
        ${PARALLEL_LIBS}
        m
    )

    target_compile_definitions(mb_utils_parallel
        PUBLIC PACKAGE_VERSION="${PROJECT_VERSION}" _PARALLEL
    )
endif()

