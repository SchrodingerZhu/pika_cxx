#ifndef PIKA_TEST_MEMOTABLE_HPP
#define PIKA_TEST_MEMOTABLE_HPP

#include <absl/hash/hash_testing.h>
#include <gtest/gtest.h>
#include <pika/memotable.hpp>
#include <vector>

using namespace pika::clause;

TEST(MemoTable, Hash)
{
    EXPECT_TRUE(absl::VerifyTypeImplementsAbslHashCorrectly(
        {pika::memotable::MemoKey(Char<'A'>().get_instance(), 0),
         pika::memotable::MemoKey(Char<'B'>().get_instance(), 1),
         pika::memotable::MemoKey(
             Seq<Char<'A'>, Char<'B'>>().get_instance(), 1)}));
}


TEST(MemoTable, IntervalUnion)
{
    using namespace pika::memotable;
    using namespace pika::utils;
    IntervalUnion interval;
    //[1, 1, 1, 1, 1, 0, 0, 1, 1, 1]
    interval.add_interval(0, 6);
    interval.add_interval(8, 11);

    //[0, 0, 0, 0, 0, 1, 1, 0, 0, 0]
    auto inverted = interval.invert(0, 11);
    EXPECT_EQ(inverted.size(), 1);

    EXPECT_FALSE(interval.is_overlap(6, 8));
    EXPECT_TRUE(interval.is_overlap(5, 6));
    EXPECT_TRUE(interval.is_overlap(7, 10));

    EXPECT_TRUE(inverted.is_overlap(6, 7));
    EXPECT_TRUE(inverted.is_overlap(5, 8));
    EXPECT_FALSE(inverted.is_overlap(0, 5));
    EXPECT_FALSE(inverted.is_overlap(8, 10));

    interval.add_interval(6, 7);
    EXPECT_EQ(interval.size(), 2);
    interval.add_interval(7, 8);
    EXPECT_EQ(interval.size(), 1);
    EXPECT_TRUE(interval.is_overlap(5, 8));
    EXPECT_EQ(interval.invert(0, 9).size(), 0);
}


#endif // PIKA_TEST_MEMOTABLE_HPP
