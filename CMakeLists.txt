cmake_minimum_required(VERSION 3.14)
project(rust-compiler LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Main executable
add_executable(rustc
    src/main.cc
    src/lexer.cc
    src/parser.cc
)

# Google Test
include(FetchContent)
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG v1.14.0
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()

# Test executable
add_executable(lexer_test
    tests/lexer_test.cc
    src/lexer.cc
)
target_link_libraries(lexer_test GTest::gtest_main)
target_include_directories(lexer_test PRIVATE ${CMAKE_SOURCE_DIR}/src)

include(GoogleTest)
gtest_discover_tests(lexer_test)

add_executable(parser_test
    tests/parser_test.cc
    src/lexer.cc
    src/parser.cc
)
target_link_libraries(parser_test GTest::gtest_main)
target_include_directories(parser_test PRIVATE ${CMAKE_SOURCE_DIR}/src)

gtest_discover_tests(parser_test)
