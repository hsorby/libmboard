# Only build if parallel is enabled (maps COMPILE_PARALLEL)
if(NOT ENABLE_PARALLEL)
    return()
endif()

# ----------------------------------------
# Common sources (headers NOT added as sources)
# ----------------------------------------
set(COMMON_SOURCES
    addmessage.c
    clear.c
    comm_routines_OLD.c
    comm_routines_HANDSHAKE.c
    comm_utils.c
    commthread.c
    create.c
    delete.c
    env_finalise.c
    env_finalised.c
    env_init.c
    env_initialised.c
    filter_create.c
    filter_delete.c
    filter_assign.c
    function_assign.c
    function_free.c
    function_register.c
    iterator_create.c
    iterator_createfiltered.c
    iterator_createsorted.c
    iterator_createfilteredsorted.c
    iterator_delete.c
    iterator_getmessage.c
    iterator_randomise.c
    iterator_rewind.c
    indexmap_create.c
    indexmap_delete.c
    indexmap_sync.c
    indexmap_memberof.c
    indexmap_addentry.c
    searchtree_create.c
    searchtree_search.c
    searchtree_delete.c
    setaccessmode.c
    setsyncpattern.c
    synccomplete.c
    syncstart.c
    synctest.c
    util_commqueue.c
    util_syncqueue.c
)

# ----------------------------------------
# Parallel dependencies (replacement for @MB_PARALLEL_*)
# ----------------------------------------
find_package(MPI REQUIRED)

# Threads (often needed in your code)
find_package(Threads REQUIRED)

set(PARALLEL_LIBS MPI::MPI_C Threads::Threads)
set(PARALLEL_CFLAGS "")
set(PARALLEL_LDFLAGS "")

# ----------------------------------------
# Production library (lib_mb_p)
# ----------------------------------------
add_library(mb_parallel STATIC ${COMMON_SOURCES})

add_library(mboard::mb_parallel ALIAS mb_parallel)

target_include_directories(mb_parallel
    PUBLIC
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
        $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/generated>
        $<INSTALL_INTERFACE:include>
)

target_compile_options(mb_parallel PRIVATE
    $<$<CONFIG:Debug>:${DEBUG_CFLAGS}>
    $<$<CONFIG:Release>:${PRODUCTION_CFLAGS}>
    ${PARALLEL_CFLAGS}
)

target_link_libraries(mb_parallel
    ${PARALLEL_LIBS}
    m
)

target_compile_definitions(mb_parallel PUBLIC _PARALLEL)
