//
// Created by schrodinger on 10/21/20.
//
#include <pika/parse_tree.hpp>

std::vector<std::unique_ptr<const pika::parse_tree::TreeNode>> reduced(
        const std::vector<std::shared_ptr<pika::memotable::Match>>& sub_matches,
        const pika::memotable::MemoTable &table
) {
    std::vector<std::unique_ptr<const pika::parse_tree::TreeNode>> result;
    for (const auto & i: sub_matches) {
        auto current = pika::parse_tree::TreeNode::build_from_match(*i, table);
        std::move(current.begin(), current.end(), std::back_inserter(result));
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
        return reduced(match.sub_matches, table);
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
        branches(reduced(match.sub_matches, table)) {

}

size_t pika::parse_tree::TreeNode::size() const noexcept {
    return branches.size();
}
