//
// Created by schrodinger on 10/21/20.
//
#include <pika/parse_tree.hpp>

std::vector<std::unique_ptr<const pika::parse_tree::TreeNode>> reduced(
        std::type_index parent,
        pika::type_utils::BaseType base,
        const std::vector<std::shared_ptr<pika::memotable::Match>> &sub_matches,
        const pika::memotable::MemoTable &table
) {
    std::vector<std::unique_ptr<const pika::parse_tree::TreeNode>> result;
    if (base != pika::type_utils::BaseType::Plus && base != pika::type_utils::BaseType::Asterisks) {
        for (const auto &i: sub_matches) {
            auto current = pika::parse_tree::TreeNode::build_from_match(*i, table);
            std::move(current.begin(), current.end(), std::back_inserter(result));
        }
    } else if (!sub_matches.empty()) {
        std::vector<std::shared_ptr<pika::memotable::Match>> real = sub_matches;
        auto last = sub_matches.back();
        while (parent == typeid(real.back()->key.tag)) {
            real.pop_back();
            for (const auto &i : last->sub_matches) {
                real.push_back(i);
            }
            last = real.back();
        }
        for (const auto &i: real) {
            auto current = pika::parse_tree::TreeNode::build_from_match(*i, table);
            std::move(current.begin(), current.end(), std::back_inserter(result));
        }
    }
    return result;
}

std::vector<std::unique_ptr<const pika::parse_tree::TreeNode>>
pika::parse_tree::TreeNode::build_from_match(const pika::memotable::Match &match,
                                             const pika::memotable::MemoTable &table) {
    if (match.key.tag->active()) {
        std::vector<std::unique_ptr<const pika::parse_tree::TreeNode>> result;
        result.emplace_back(std::make_unique<const pika::parse_tree::TreeNode>(match, table));
        return result;
    } else {
        return reduced(typeid(*match.key.tag), match.key.get_base_type(), match.sub_matches, table);
    }
}

bool pika::parse_tree::TreeNode::empty() const noexcept {
    return branches.empty();
}

pika::parse_tree::TreeNode::const_iterator pika::parse_tree::TreeNode::begin() const noexcept {
    return branches.begin();
}

pika::parse_tree::TreeNode::const_iterator pika::parse_tree::TreeNode::end() const noexcept {
    return branches.end();
}


pika::parse_tree::TreeNode::TreeNode
        (const pika::memotable::Match &match, const pika::memotable::MemoTable &table) :
        matched_clause(match.key.tag),
        matched_content(table.target.substr(match.key.start_position, match.key.start_position + match.length)),
        branches(reduced(typeid(*match.key.tag), match.key.get_base_type(), match.sub_matches, table)) {

}

size_t pika::parse_tree::TreeNode::size() const noexcept {
    return branches.size();
}
