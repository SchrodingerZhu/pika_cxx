cmake_minimum_required(VERSION 3.10)
project(pika CXX)
set(CMAKE_CXX_STANDARD 17)

add_subdirectory(abseil-cpp)
add_subdirectory(snmalloc)
add_subdirectory(googletest)

include_directories(include)
include_directories(abseil-cpp)
include_directories(googletest/googlemock/include)
include_directories(googletest/googletest/include)

include(tests/tests.cmake)