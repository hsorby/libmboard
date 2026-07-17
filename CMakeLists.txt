cmake_minimum_required(VERSION 3.21)

project(libmboard VERSION 0.3.1 LANGUAGES C)

# ----------------------------------------
# Options (from AC_ARG_ENABLE)
# ----------------------------------------
option(ENABLE_MEMLOG "Enable instrumentation of memory usage" OFF)
option(ENABLE_UNSAFE_CHECKS "Enable unsafe developer checks" OFF)
option(ENABLE_VERBOSE "Enable verbose output" OFF)
option(ENABLE_WARNINGS "Enable warning printing in debug" OFF)
option(ENABLE_PARALLEL "Enable parallel build" ON)
option(ENABLE_TESTS "Enable unit tests" OFF)

# ----------------------------------------
# Compiler settings (converted from configure.ac)
# ----------------------------------------
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

# GCC-style flags
message(STATUS "CMake compiler ID: ${CMAKE_C_COMPILER_ID}")
if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(
        -fno-common
        -Wall -pedantic
        -Wextra -Wstrict-prototypes -Wmissing-prototypes -Wpointer-arith
        -Wreturn-type -Wswitch -Wshadow -Wcast-align -Wunused
    )
endif()

# Production / Debug flags
set(PRODUCTION_CFLAGS -DNDEBUG -O3)
set(DEBUG_CFLAGS -g -D_EXTRA_CHECKS)

if(ENABLE_MEMLOG)
    add_definitions(-D_LOG_MEMORY_USAGE)
endif()

if(ENABLE_UNSAFE_CHECKS)
    list(APPEND DEBUG_CFLAGS -D_UNSAFE_CHECKS)
endif()

if(ENABLE_VERBOSE)
    list(APPEND DEBUG_CFLAGS -D_EXTRA_INFO)
    list(APPEND PRODUCTION_CFLAGS -D_EXTRA_INFO)
endif()

if(ENABLE_WARNINGS OR ENABLE_VERBOSE)
    list(APPEND DEBUG_CFLAGS -D_PRINT_WARNINGS)
endif()

# ----------------------------------------
# Required headers / libraries
# ----------------------------------------
include(CheckIncludeFile)
check_include_file(limits.h HAVE_LIMITS_H)
check_include_file(stddef.h HAVE_STDDEF_H)
check_include_file(unistd.h HAVE_UNISTD_H)
check_include_file(float.h HAVE_FLOAT_H)

if(ENABLE_MEMLOG)
    set(WANT_MEMLOG TRUE)
endif()

set(CONFIG_ARGS "${CMAKE_ARGS}")
string(TIMESTAMP CONFIG_DATE "%Y-%m-%d")

configure_file(
    ${PROJECT_SOURCE_DIR}/include/mb_config.h.in
    ${PROJECT_BINARY_DIR}/generated/mb_config.h
)

find_library(M_LIB m)
if(M_LIB)
    message(STATUS "Found libm")
endif()

# ----------------------------------------
# Include directories
# ----------------------------------------
include_directories(${PROJECT_SOURCE_DIR}/include)
include_directories(${PROJECT_BINARY_DIR}/generated)

# ----------------------------------------
# Subdirectories (SUBDIRS)
# ----------------------------------------
add_subdirectory(src/utils)
add_subdirectory(src/serial)

if(ENABLE_PARALLEL)
    add_subdirectory(src/parallel)
endif()

add_subdirectory(src/progs)

if(ENABLE_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

# ----------------------------------------
# Install headers
# ----------------------------------------
install(FILES
    ${PROJECT_BINARY_DIR}/generated/mb_config.h
    include/mboard.h
    include/mb_memlog.h
    include/mb_objmap.h
    DESTINATION include
)

# ----------------------------------------
# Extra targets (doc, test, etc.)
# ----------------------------------------
add_custom_target(doc
    COMMAND doxygen Doxyfile_user
    COMMAND doxygen Doxyfile_developer_serial
    COMMAND doxygen Doxyfile_developer_parallel
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
)

add_custom_target(clean-all
    COMMAND ${CMAKE_BUILD_TOOL} clean
)

# ----------------------------------------
# Consumer facing targets
# ----------------------------------------
add_library(mboard_s INTERFACE)

add_library(libmboard::mboard_s ALIAS mboard_s)

target_link_libraries(mboard_s
    INTERFACE
        mb_serial
        mb_utils_serial
)

if(ENABLE_PARALLEL)
    add_library(mboard_p INTERFACE)

    add_library(libmboard::mboard_p ALIAS mboard_p)

    target_link_libraries(mboard_p
        INTERFACE
            mb_parallel
            mb_utils_parallel
    )
endif()

# ----------------------------------------
# Install targets
# ----------------------------------------
install(
    TARGETS
        mboard_s
        mb_serial
        mb_utils_serial
    EXPORT libmboardTargets
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
)

if(ENABLE_PARALLEL)
    install(
        TARGETS
            mboard_p
            mb_parallel
            mb_utils_parallel
        EXPORT libmboardTargets
        ARCHIVE DESTINATION lib
        LIBRARY DESTINATION lib
        RUNTIME DESTINATION bin
    )
endif()

install(
    EXPORT libmboardTargets
    FILE libmboard-targets.cmake
    NAMESPACE libmboard::
    DESTINATION lib/cmake/libmboard
)

include(CMakePackageConfigHelpers)

write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/libmboard-config-version.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/libmboard-config.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/libmboard-config.cmake"
    INSTALL_DESTINATION lib/cmake/libmboard
)

install(
    FILES
        "${CMAKE_CURRENT_BINARY_DIR}/libmboard-config.cmake"
        "${CMAKE_CURRENT_BINARY_DIR}/libmboard-config-version.cmake"
    DESTINATION lib/cmake/libmboard
)

# ----------------------------------------
# Build tree package config
# ----------------------------------------

export(
    EXPORT libmboardTargets
    FILE "${CMAKE_CURRENT_BINARY_DIR}/libmboard-targets.cmake"
    NAMESPACE libmboard::
)

configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/libmboard-config.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/libmboard-config.cmake"
    INSTALL_DESTINATION lib/cmake/libmboard
)

# ----------------------------------------
# Configuration Summary Report
# ----------------------------------------
message(STATUS "----------------------------------------")
message(STATUS "libmboard ${PROJECT_VERSION}")
message(STATUS "Compiler: ${CMAKE_C_COMPILER}")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "Parallel enabled: ${ENABLE_PARALLEL}")
message(STATUS "Verbose enabled: ${ENABLE_VERBOSE}")
message(STATUS "Tests enabled: ${ENABLE_TESTS}")
message(STATUS "Memlog enabled: ${ENABLE_MEMLOG}")
message(STATUS "----------------------------------------")

