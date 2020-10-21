#include <gtest/gtest.h>
#include "test_memotable.hpp"
#include "test_clause.hpp"
#include "test_parse_tree.hpp"
int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}