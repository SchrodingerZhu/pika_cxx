enable_testing()

file(GLOB TEST_SRC tests/*.cpp)
add_executable(pika-test ${TEST_SRC})
target_link_libraries(pika-test snmallocshim gmock gtest absl::strings absl::flat_hash_map pika)
add_test(pika-test pika-test)



