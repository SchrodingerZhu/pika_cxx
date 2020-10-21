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

TEST(Clause, ConstructTable) {
    pika::clause::ClauseTable table;
    Toplevel().construct_table(table);
    for (auto &i : table) {
        EXPECT_EQ(i.first, typeid(*i.second));
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
