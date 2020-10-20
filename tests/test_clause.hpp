//
// Created by schrodinger on 10/19/20.
//

#ifndef PIKA_TEST_CLAUSE_HPP
#define PIKA_TEST_CLAUSE_HPP
#include <pika/clause.hpp>
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
                     PIKA_SEQ(Primary, PIKA_CHAR('+'), Multiplicative),
                     Primary), true);
PIKA_DECLARE(Additive,
             PIKA_ORD(
                     PIKA_SEQ(Multiplicative, PIKA_CHAR('+'), Additive),
                     Multiplicative), true);
PIKA_DECLARE(Toplevel,
             PIKA_SEQ(PIKA_FIRST, Additive, PIKA_NOT_FOLLOWED_BY(PIKA_ANY)), true);

TEST(Clause, Display) {
    auto target = "( ( Multiplicative ~ '+' ~ Additive ) / Multiplicative )\n"
                  "( ^ ~ Additive ~ !( ANYTHING ) )\n";
    std::ostringstream output;
    output << Additive().display() << std::endl;
    output << Toplevel().display() << std::endl;
    EXPECT_EQ(output.str(), target);
}
TEST(Clause, Dump) {
    auto target = "Digit <- ['0'-'9']\n"
                  "Number <- ( Digit )+\n"
                  "Primary <- ( ( '(' ~ Additive ~ ')' ) / Number )\n"
                  "Multiplicative <- ( ( Primary ~ '+' ~ Multiplicative ) / Primary )\n"
                  "Additive <- ( ( Multiplicative ~ '+' ~ Additive ) / Multiplicative )\n"
                  "Toplevel <- ( ^ ~ Additive ~ !( ANYTHING ) )\n";
    std::ostringstream output;
    Toplevel().dump(output);
    EXPECT_EQ( output.str(), target);
}

TEST(Clause, FindTerminals) {
    pika::clause::ClauseTable table;
    Toplevel().construct_table(table);
    for (auto & i : table) {
        std::cout << abi::__cxa_demangle(i.first.name(), nullptr, nullptr, nullptr) << std::endl;
        EXPECT_EQ(i.first, typeid(*i.second));
    }
}
#endif //PIKA_TEST_CLAUSE_HPP
