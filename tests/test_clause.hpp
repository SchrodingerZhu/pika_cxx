//
// Created by schrodinger on 10/19/20.
//

#ifndef PIKA_TEST_CLAUSE_HPP
#define PIKA_TEST_CLAUSE_HPP
#include <pika/clause.hpp>
#include <sstream>
using namespace pika::clause;

struct Additive;
struct Multiplicative;

struct Digit : CharRange<'0', '9'> {
    std::optional<std::string_view> label() override {
        return "Digit";
    }
};

struct Number : Plus<Digit> {
    std::optional<std::string_view> label() override {
        return "Number";
    }
};

struct Primary : Ord<Seq<Char<'('>, Additive, Char<')'>>, Number> {
    std::optional<std::string_view> label() override {
        return "Primary";
    }
};


struct Multiplicative : Ord<Seq<Primary, Char<'+'>, Multiplicative>, Primary> {
    std::optional<std::string_view> label() override {
        return "Multiplicative";
    }
};

struct Additive : Ord<Seq<Multiplicative, Char<'+'>, Additive>, Multiplicative> {
    std::optional<std::string_view> label() override {
        return "Additive";
    }
};

struct Toplevel : Seq<First, Additive, NotFollowedBy<Any>> {
    std::optional<std::string_view> label() override {
        return "Toplevel";
    }
};


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
#endif //PIKA_TEST_CLAUSE_HPP
