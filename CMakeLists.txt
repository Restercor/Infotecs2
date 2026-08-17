cmake_minimum_required(VERSION 3.16)

project(LogProject VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_EXTENSIONS OFF)

# Logging library

option(BUILD_SHARED_LIBS "Building logging libruary as shared" ON) # Type selection

add_library(logging
    src/logger.cpp
    src/async_logger.cpp
    src/log_level.cpp
)

target_include_directories(logging PUBLIC ${PROJECT_SOURCE_DIR}/include)
target_compile_options(logging PRIVATE -Wall -Wextra -Wpedantic)

# Part 3 application

add_executable(statistics_server
    part3Server/main.cpp
)

target_include_directories(statistics_server PRIVATE ${PROJECT_SOURCE_DIR}/include ${PROJECT_SOURCE_DIR}/part3Server)

target_link_libraries(statistics_server PRIVATE logging)

target_compile_options(statistics_server PRIVATE -Wall -Wextra -Wpedantic)

# Part 2 application

add_executable(async_logger
    src/main.cpp
)

target_include_directories(async_logger PRIVATE $(PROJECT_SOURCE_DIR)/include)
target_link_libraries(async_logger PRIVATE logging)
target_compile_options(async_logger PRIVATE -Wall -Wextra -Wpedantic)

# tests

include(CTest)
enable_testing()

add_executable(test_logger
    tests/logger_tests.cpp
)

target_include_directories(test_logger PRIVATE ${PROJECT_SOURCE_DIR}/include)

target_link_libraries(test_logger PRIVATE logging)
target_compile_options(test_logger PRIVATE -Wall -Wextra -Wpedantic)

add_test(NAME logger_tests COMMAND test_logger)