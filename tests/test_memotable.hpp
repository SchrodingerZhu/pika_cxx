#include <pika/memotable.hpp>
#include <vector>
#include <absl/hash/hash_testing.h>
#include <gtest/gtest.h>
using namespace pika::clause;

TEST(MemoKey, Hash) {
    EXPECT_TRUE(absl::VerifyTypeImplementsAbslHashCorrectly({
            pika::memotable::MemoKey(Char<'A'>(), 0),
            pika::memotable::MemoKey(Char<'B'>(), 1),
            pika::memotable::MemoKey(Seq<Char<'A'>, Char<'B'>>(), 1)
    }));
}
