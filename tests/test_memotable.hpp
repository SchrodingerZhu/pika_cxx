#ifndef PIKA_TEST_MEMOTABLE_HPP
#define PIKA_TEST_MEMOTABLE_HPP

#include <absl/hash/hash_testing.h>
#include <gtest/gtest.h>
#include <pika/memotable.hpp>
#include <vector>

using namespace pika::clause;

TEST(MemoKey, Hash)
{
    EXPECT_TRUE(absl::VerifyTypeImplementsAbslHashCorrectly(
        {pika::memotable::MemoKey(Char<'A'>().get_instance(), 0),
         pika::memotable::MemoKey(Char<'B'>().get_instance(), 1),
         pika::memotable::MemoKey(
             Seq<Char<'A'>, Char<'B'>>().get_instance(), 1)}));
}

#endif // PIKA_TEST_MEMOTABLE_HPP
