# ----------------------------------------
# Common sources (headers excluded)
# ----------------------------------------
set(COMMON_SOURCES
    addmessage.c
    clear.c
    create.c
    delete.c
    env_finalise.c
    env_finalised.c
    env_init.c
    env_initialised.c
    filter_assign.c
    filter_delete.c
    filter_create.c
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
    serial_utils.c
    synccomplete.c
    syncstart.c
    synctest.c
)

# ----------------------------------------
# Library mb_serial
# ----------------------------------------
add_library(mb_serial ${COMMON_SOURCES})

add_library(mboard::mb_serial ALIAS mb_serial)

target_compile_options(mb_serial PRIVATE
    $<$<CONFIG:Debug>:${DEBUG_CFLAGS}>
    $<$<CONFIG:Release>:${PRODUCTION_CFLAGS}>
)

target_include_directories(mb_serial
    PUBLIC
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
        $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/generated>
        $<INSTALL_INTERFACE:include>
)

target_link_libraries(mb_serial
    m
)
