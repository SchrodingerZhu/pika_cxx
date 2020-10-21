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