//
// Created by schrodinger on 10/24/20.
//

#ifndef PIKA_TEST_GRAPH_HPP
#define PIKA_TEST_GRAPH_HPP
#include "test_clause.hpp"
#include "test_parse_tree.hpp"
#include <pika/graph.hpp>

TEST(Graph, ConstructTable) {
    auto target = "(1+1)*2";
    auto table = pika::graph::construct_table(Toplevel(), target);
    for (auto &i : table) {
        std::cout << abi::__cxa_demangle(i.first.name(), nullptr, nullptr, nullptr)
                  << ", order: " << i.second.topological_order << std::endl;
        for (auto &j : i.second.candidates) {
            std::cout << " - "
                      << abi::__cxa_demangle(typeid(*j).name(), nullptr, nullptr, nullptr)
                      << std::endl;
        }
    }
}
#define PARSE(STR, RES) \
          {\
            auto target = STR;\
            auto table = pika::graph::construct_table(Toplevel(), target);\
            auto result = table.match();\
            EXPECT_TRUE(result);\
            auto tree = pika::parse_tree::TreeNode(*result, table.memo_table);\
            EXPECT_EQ(eval(tree), RES);\
          }

TEST(Graph, Parse) {
    PARSE("1+1*((((2))))", 3)
    PARSE("213*123+123*(1+(2*3+1))", 27183)
    PARSE("(11123+123*123+(123*123)+(123)+114514+(((((((((1)*1))*1)))*1)+2))+4)", 156025)
    PARSE("1*2*3*4*5*6", 720)
    PARSE("50000000000", 50000000000)
    PARSE("1*(1)+1", 2)
    PARSE("(((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((("
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
          "))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))", 1)
}

#endif //PIKA_TEST_GRAPH_HPP