//
// Created by schrodinger on 10/21/20.
//

#ifndef PIKA_TEST_PARSE_TREE_HPP
#define PIKA_TEST_PARSE_TREE_HPP

#include <pika/parse_tree.hpp>
#include "test_clause.hpp"

size_t eval(const pika::parse_tree::TreeNode &node) {
    if (node.is_clause<Number>()) {
        return std::stoull(std::string{node.matched_content.begin(), node.matched_content.end()});
    } else if (node.size() == 1) {
        return eval(**node.begin());
    } else {
        if (node.is_clause<Multiplicative>()) {
            EXPECT_EQ(node.size(), 2);
            return eval(**node.begin()) * eval(**(node.begin() + 1));
        } else if (node.is_clause<Additive>()) {
            EXPECT_EQ(node.size(), 2);
            return eval(**node.begin()) + eval(**(node.begin() + 1));
        }
    }
    throw std::runtime_error ("unreachable");
}

TEST(ParseTree, Build) {
    std::vector<std::pair<std::string_view, size_t>> tests = {
            {"1+1", 2},
            {"(13*5)*2+14*(1+(5*(1+(2*3))))", 634},
            {"(1)*223*(11)+114514*1+(1*1*1)*(((((((1)))))))", 116968}
    };
    for(auto& i: tests)
    {
        pika::memotable::MemoTable table(i.first);
        auto match = Toplevel().packrat_match(table, 0);
        auto tree = pika::parse_tree::TreeNode(*match, table);
        EXPECT_EQ(eval(tree), i.second);
    }
}


#endif //PIKA_TEST_PARSE_TREE_HPP
