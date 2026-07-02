# ----------------------------------------
# Include directory
# ----------------------------------------
set(INCLUDEDIR ${PROJECT_SOURCE_DIR}/include)

# ----------------------------------------
# Install scripts (dist_bin_SCRIPTS)
# ----------------------------------------
install(PROGRAMS
    ${CMAKE_CURRENT_SOURCE_DIR}/mboard-config
    ${CMAKE_CURRENT_SOURCE_DIR}/mboard-check-minversion
    DESTINATION bin
)

# ----------------------------------------
# Optional: memlog tool
# ----------------------------------------
if(ENABLE_MEMLOG)

    add_executable(mboard-parse-memlogdb
        mboard-parse-memlogdb.c
    )

    target_include_directories(mboard-parse-memlogdb
        PRIVATE ${INCLUDEDIR}
    )

    # Link against sqlite helper lib created in utils
    target_link_libraries(mboard-parse-memlogdb
        utils_sqlite3
        m
    )

    install(TARGETS mboard-parse-memlogdb
        RUNTIME DESTINATION bin
    )

endif()

