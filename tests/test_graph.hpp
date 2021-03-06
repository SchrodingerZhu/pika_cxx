//
// Created by schrodinger on 10/24/20.
//

#ifndef PIKA_TEST_GRAPH_HPP
#define PIKA_TEST_GRAPH_HPP
#include "test_clause.hpp"
#include "test_parse_tree.hpp"

#include <pika/graph.hpp>

TEST(Graph, ConstructTable)
{
    auto target = "(1+1)*2";
    auto table = pika::graph::construct_table(Toplevel(), target);
    for (auto& i : table)
    {
        std::cout << abi::__cxa_demangle(
                         i.first.name(), nullptr, nullptr, nullptr)
                  << ", order: " << i.second.topological_order << std::endl;
        for (auto& j : i.second.candidates)
        {
            std::cout << " - "
                      << abi::__cxa_demangle(
                             typeid(*j).name(), nullptr, nullptr, nullptr)
                      << std::endl;
        }
    }
}
#define PARSE(RULE, STR, RES, EVAL) \
    { \
        auto target = STR; \
        auto table = pika::graph::construct_table(RULE(), target); \
        auto result = table.match(); \
        EXPECT_TRUE(result); \
        auto tree = pika::parse_tree::TreeNode(*result, table.memo_table); \
        EXPECT_EQ(EVAL(tree), RES); \
    }
TEST(Graph, Number)
{
    auto target = "123";
    auto table = pika::graph::construct_table(Number(), target);
    auto result = table.match();
    EXPECT_TRUE(result);
    auto tree = pika::parse_tree::TreeNode(*result, table.memo_table);
    EXPECT_EQ(eval(tree), 123);
}
TEST(Graph, Parse){
    PARSE(Toplevel, "1+1*((((2))))", 3, eval)
        PARSE(Toplevel, "213*123+123*(1+(2*3+1))", 27183, eval) PARSE(
            Toplevel,
            "(11123+123*123+(123*123)+(123)+114514+(((((((((1)*1))*1)))*1)+2))+"
            "4)",
            156025,
            eval) PARSE(Toplevel, "1*2*3*4*5*6", 720, eval)
            PARSE(Toplevel, "50000000000", 50000000000, eval)
                PARSE(Toplevel, "1*(1)+1", 2, eval) PARSE(
                    Toplevel,
                    "(((((((((((((((((((((((((((((((((((((((((((((((((((((((((("
                    "(((((((((((((((((((((((((((((((((((((((((("
                    "(((((((((((((((((((((((((((((((((((((((((((((((((((((((((("
                    "(((((((((((((((((((((((((((((((((((((((((("
                    "(((((((((((((((((((((((((((((((((((((((((((((((((((((((((("
                    "(((((((((((((((((((((((((((((((((((((((((("
                    "(((((((((((((((((((((((((((((((((((((((((((((((((((((((((("
                    "(((((((((((((((((((((((((((((((((((((((((("
                    "(((((((((((((((((((((((((((((((((((((((((((((((((((((((((("
                    "(((((((((((((((((((((((((((((((((((((((((("
                    "(((((((((((((((((((((((((((((((((((((((((((((((((((((((((("
                    "(((((((((((((((((((((((((((((((((((((((((("
                    "(((((((((((((((((((((((((((((((((((((((((((((((((((((((((("
                    "(((((((((((((((((((((((((((((((((((((((((("
                    "1"
                    "))))))))))))))))))))))))))))))))))))))))))))))))))))))))))"
                    "))))))))))))))))))))))))))))))))))))))))))"
                    "))))))))))))))))))))))))))))))))))))))))))))))))))))))))))"
                    "))))))))))))))))))))))))))))))))))))))))))"
                    "))))))))))))))))))))))))))))))))))))))))))))))))))))))))))"
                    "))))))))))))))))))))))))))))))))))))))))))"
                    "))))))))))))))))))))))))))))))))))))))))))))))))))))))))))"
                    "))))))))))))))))))))))))))))))))))))))))))"
                    "))))))))))))))))))))))))))))))))))))))))))))))))))))))))))"
                    "))))))))))))))))))))))))))))))))))))))))))"
                    "))))))))))))))))))))))))))))))))))))))))))))))))))))))))))"
                    "))))))))))))))))))))))))))))))))))))))))))"
                    "))))))))))))))))))))))))))))))))))))))))))))))))))))))))))"
                    "))))))))))))))))))))))))))))))))))))))))))",
                    1,
                    eval)}

PIKA_DECLARE(AorB, PIKA_ORD(PIKA_CHAR('a'), PIKA_CHAR('b')), false);
PIKA_DECLARE(Prefix, PIKA_SEQ(PIKA_OPTIONAL(PIKA_CHAR('c')), AorB), false);
PIKA_DECLARE(MyString, PIKA_ASTERISKS(Prefix), true);
auto extract(const pika::parse_tree::TreeNode& node)
{
    return node.matched_content;
}

TEST(Graph, String)
{
    PARSE(MyString, "aabb", "aabb", extract);
    PARSE(MyString, "cacacbdb", "cacacb", extract);
    PARSE(MyString, "ddd", "", extract);
    PARSE(MyString, "cabbaacb", "cabbaacb", extract);
}

TEST(Graph, LeftRecusion)
{
    PARSE(Add, "1", 1, eval);
    PARSE(Add, "1+1", 2, eval);
    PARSE(Add, "1+555+1+1", 558, eval);
}
PIKA_DECLARE(
    List, PIKA_ORD(PIKA_SEQ(List, PIKA_CHAR('a')), PIKA_CHAR('a')), true);

TEST(Graph, List)
{
    PARSE(List, "a", "a", extract);
    PARSE(List, "aaa", "aaa", extract);
    PARSE(List, "aaab", "aaa", extract);
    PARSE(List, "aaabaaa", "aaa", extract);
    std::string as(1000, 'a');
    PARSE(List, as, as, extract);
}
PIKA_DECLARE(List2, PIKA_PLUS(PIKA_CHAR('a')), true);
TEST(Graph, List2)
{
    PARSE(List2, "a", "a", extract);
    PARSE(List2, "aaa", "aaa", extract);
    PARSE(List2, "aaab", "aaa", extract);
    PARSE(List2, "aaabaaa", "aaa", extract);
    std::string as(1000, 'a');
    PARSE(List2, as, as, extract);
}

#endif // PIKA_TEST_GRAPH_HPP
