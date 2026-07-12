# SPDX-License-Identifier: MIT

if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/../vendor/googletest/CMakeLists.txt")
    message(FATAL_ERROR
        "vendor/googletest is missing.\n"
        "Run: git submodule update --init --recursive")
endif()
add_subdirectory(../vendor/googletest ${CMAKE_BINARY_DIR}/_googletest)

file(GLOB_RECURSE CNA_EXTENDED_TEST_SOURCES CONFIGURE_DEPENDS
    "*.cpp"
)

add_executable(CnaExtendedTests
    ${CNA_EXTENDED_TEST_SOURCES}
)

target_link_libraries(CnaExtendedTests
    PRIVATE
        CNA_EXTENDED
        gtest_main
)

if(MSVC)
    target_compile_options(CnaExtendedTests PRIVATE /W4 /WX)
else()
    target_compile_options(CnaExtendedTests PRIVATE -Wall -Wextra -Werror)
endif()

include(GoogleTest)
gtest_discover_tests(CnaExtendedTests)
