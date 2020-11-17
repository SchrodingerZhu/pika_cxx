cmake_minimum_required(VERSION 3.10)
project(pika CXX)
option(PIKA_FULL_OPTIMIZATION OFF "Enable optimizations for benchmarking.")
set(CMAKE_CXX_STANDARD 17)

if (PIKA_FULL_OPTIMIZATION)
    set(CMAKE_CXX_FLAGS "-O3 -march=native -flto")
endif()

add_subdirectory(abseil-cpp)
add_subdirectory(snmalloc)
add_subdirectory(googletest)

include_directories(include)
include_directories(abseil-cpp)
include_directories(googletest/googlemock/include)
include_directories(googletest/googletest/include)

file(GLOB PIKA_SRC src/*.cpp)

add_library(pika SHARED ${PIKA_SRC})

include(tests/tests.cmake)

find_program(CLANG_FORMAT NAMES
        clang-format)

if (${CLANG_FORMAT} STREQUAL "CLANG_FORMAT-NOTFOUND")
    message(WARNING "Not generating pika-clang-format target, no clang-format tool found")
else ()
    message(STATUS "Generating pika-clang-format target using ${CLANG_FORMAT}")
    file(GLOB ALL_SOURCE_FILES include/pika/* src/* tests/*.cpp tests/*.hpp)
    add_custom_target(
            pika-clang-format
            COMMAND ${CLANG_FORMAT}
            -i
            ${ALL_SOURCE_FILES})
endif()