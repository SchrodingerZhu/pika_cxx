//
// Created by schrodinger on 10/21/20.
//

#ifndef PIKA_PARSE_TREE_HPP
#define PIKA_PARSE_TREE_HPP

#include <vector>
#include <memory>
#include <pika/clause.hpp>
#include <pika/memotable.hpp>

namespace pika {
    namespace parse_tree {
        class TreeNode {
            const std::vector<std::unique_ptr<const TreeNode>> branches;
            const pika::clause::Clause *const matched_clause;
        public:
            using const_iterator = std::vector<std::unique_ptr<const TreeNode>>::const_iterator;
            const std::string_view matched_content;

            template<class Clause>
            bool is_clause() const {
                return typeid(Clause) == typeid(*matched_clause);
            }

            [[nodiscard]] bool empty() const noexcept;

            [[nodiscard]] const_iterator begin() const noexcept;

            [[nodiscard]] const_iterator end() const noexcept;

            [[nodiscard]] size_t size() const noexcept;

            explicit TreeNode(const pika::memotable::Match &match, const pika::memotable::MemoTable &table);

            static std::vector<std::unique_ptr<const TreeNode>>
            build_from_match(const pika::memotable::Match &match, const pika::memotable::MemoTable &table);
        };
    }
}
#endif //PIKA_PARSE_TREE_HPP
