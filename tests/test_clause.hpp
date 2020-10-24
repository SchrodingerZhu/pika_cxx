//
// Created by schrodinger on 10/19/20.
//

#ifndef PIKA_TEST_CLAUSE_HPP
#define PIKA_TEST_CLAUSE_HPP

#include <pika/clause.hpp>
#include <pika/clause.ipp>
#include <cxxabi.h>
#include <sstream>

using namespace pika::clause;

struct Additive;
struct Multiplicative;

PIKA_DECLARE(Digit, PIKA_CHAR_RANGE('0', '9'), false);

PIKA_DECLARE(Number, PIKA_PLUS(Digit), true);

PIKA_DECLARE(Primary,
             PIKA_ORD(
                     PIKA_SEQ(PIKA_CHAR('('), Additive, PIKA_CHAR(')')),
                     Number), true);

PIKA_DECLARE(Multiplicative,
             PIKA_ORD(
                     PIKA_SEQ(Primary, PIKA_CHAR('*'), Multiplicative),
                     Primary), true);

PIKA_DECLARE(Additive,
             PIKA_ORD(
                     PIKA_SEQ(Multiplicative, PIKA_CHAR('+'), Additive),
                     Multiplicative), true);

PIKA_DECLARE(Toplevel,
             PIKA_SEQ(PIKA_FIRST, Additive, PIKA_NOT_FOLLOWED_BY(PIKA_ANY)), true);
PIKA_DECLARE(Add, PIKA_ORD(PIKA_SEQ(Add, PIKA_CHAR('+'), Number), Number), true);
TEST(Clause, Display) {
    auto target = "( ( Multiplicative ~ '+' ~ Additive ) / Multiplicative )\n"
                  "( ^ ~ Additive ~ !( ANYCHAR ) )\n";
    std::ostringstream output;
    output << Additive().display() << std::endl;
    output << Toplevel().display() << std::endl;
    EXPECT_EQ(output.str(), target);
}

TEST(Clause, Dump) {
    auto target = "Digit <- ['0'-'9']\n"
                  "Number <- ( Digit )+\n"
                  "Primary <- ( ( '(' ~ Additive ~ ')' ) / Number )\n"
                  "Multiplicative <- ( ( Primary ~ '*' ~ Multiplicative ) / Primary )\n"
                  "Additive <- ( ( Multiplicative ~ '+' ~ Additive ) / Multiplicative )\n"
                  "Toplevel <- ( ^ ~ Additive ~ !( ANYCHAR ) )\n";
    std::ostringstream output;
    Toplevel().dump(output);
    EXPECT_EQ(output.str(), target);
}

TEST(Clause, DFS) {
    std::vector<const pika::clause::Clause*> terminals;
    std::vector<const pika::clause::Clause*> nodes;
    absl::flat_hash_set<std::type_index> visited;
    Toplevel().dfs_traversal(visited, terminals, nodes);
    std::cout << "================ terminals ================" << std::endl;
    for(auto i : terminals) {
        std::cout << abi::__cxa_demangle(typeid(*i).name(), nullptr, nullptr, nullptr) << std::endl;
    }
    std::cout << "================ non-terminals ================" << std::endl;
    for(auto i : nodes) {
        std::cout << abi::__cxa_demangle(typeid(*i).name(), nullptr, nullptr, nullptr) << std::endl;
    }
}

TEST(Clause, PackratBasic) {
    pika::memotable::MemoTable table1("1");
    EXPECT_TRUE(Char<'1'>().packrat_match(table1, 0));
    pika::memotable::MemoTable table2("123");
    EXPECT_TRUE(Number().packrat_match(table2, 0));
}

TEST(Clause, PackratMatch) {
    {
        pika::memotable::MemoTable table("1+1");
        EXPECT_TRUE(Toplevel().packrat_match(table, 0));
    }
    {
        pika::memotable::MemoTable table("1*(5+5)");
        auto result = Toplevel().packrat_match(table, 0);
        EXPECT_TRUE(result);
    }
    {
        pika::memotable::MemoTable table("(1+2)*(123*4444*(1+555*2+3))*(3)+23*(5+5)*(123*(5+1234))");
        auto result = Toplevel().packrat_match(table, 0);
        EXPECT_TRUE(result);
    }
    {
        pika::memotable::MemoTable table("(1+2)*(123*4444-*(1+555*2+3))*(3)+23*(5+5)*(123*(5+1234))");
        auto result = Toplevel().packrat_match(table, 0);
        EXPECT_FALSE(result);
    }
    {
        pika::memotable::MemoTable table(
                "(((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((("
                "(((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((("
                "(((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((("
                "(((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((("
                "(((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((("
                "(((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((("
                "(((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((("
                "1"
                "))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))"
                "))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))"
                "))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))"
                "))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))"
                "))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))"
                "))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))"
                "))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))");
        auto result = Toplevel().packrat_match(table, 0);
        EXPECT_TRUE(result);
    }
    {
        pika::memotable::MemoTable table(
                "(((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((("
                "(((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((("
                "(((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((("
                "(((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((("
                "(((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((("
                "(((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((("
                "(((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((("
                "1"
                "))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))"
                "))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))"
                "))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))"
                "))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))"
                "))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))"
                "))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))"
                "))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))a");
        auto result = Toplevel().packrat_match(table, 0);
        EXPECT_FALSE(result);
    }
}

#endif //PIKA_TEST_CLAUSE_HPP
