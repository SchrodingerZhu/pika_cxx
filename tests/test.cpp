#include <gtest/gtest.h>
#include "test_memotable.hpp"
#include "test_clause.hpp"
int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}