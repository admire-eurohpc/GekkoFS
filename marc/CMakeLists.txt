set(MARC_SRC
    main.cpp
)
set(MARC_HEADERS
)

add_executable(marc_test ${MARC_SRC} ${MARC_HEADERS})

target_link_libraries(marc_test
    PUBLIC
    #    gkfs_intercept
    # internal libs
    #    spdlog
    #    fmt::fmt
    # others
    #    Threads::Threads
    PRIVATE
    # open issue for std::filesystem https://gitlab.kitware.com/cmake/cmake/-/issues/17834
    stdc++fs
)

target_include_directories(marc_test
    PRIVATE
)
