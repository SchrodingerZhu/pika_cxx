#ifndef PIKA_CLAUSE_IPP
#define PIKA_CLAUSE_IPP

#include <pika/clause.hpp>
#include <pika/memotable.hpp>
#include <pika/graph.hpp>
#include <typeindex>


template<char C>
std::shared_ptr<pika::memotable::Match>
Char<C>::packrat_match(memotable::MemoTable &table, size_t index) const {
    PIKA_CHECKED_MATCH({
                           if (!table.at_end(index) && table.get_char(index) == C) {
                               return table[key] =
                                              std::make_shared<pika::memotable::Match>(key, 1, 0,
                                                                                       std::vector<std::shared_ptr<pika::memotable::Match>>{});
                           }
                       });
}

template<char C>
void Char<C>::pika_match(pika::graph::ClauseTable &table) const {
    if (table.get_current() == C) {
        table.try_add(this->get_instance(), 1, 0, {});
    }

}

template<char Start, char End>
std::shared_ptr<pika::memotable::Match>
CharRange<Start, End>::packrat_match(memotable::MemoTable &table, size_t index) const {
    PIKA_CHECKED_MATCH({
                           if (!table.at_end(index) && table.get_char(index) >= Start && table.get_char(index) <= End) {
                               return table[key] =
                                              std::make_shared<pika::memotable::Match>(key, 1, 0,
                                                                                       std::vector<std::shared_ptr<pika::memotable::Match>>{});
                           }
                       });
}

template<char Start, char End>
void CharRange<Start, End>::pika_match(pika::graph::ClauseTable &table) const {
    auto current = table.get_current();
    if (current >= Start && current <= End) {
        table.try_add(this->get_instance(), 1, 0, {});
    }
}

template<typename S>
std::shared_ptr<pika::memotable::Match>
NotFollowedBy<S>::packrat_match(memotable::MemoTable &table, size_t index) const {
    PIKA_CHECKED_MATCH({
                           if (!S().packrat_match(table, index)) {
                               return table[key] =
                                              std::make_shared<pika::memotable::Match>(key, 0, 0,
                                                                                       std::vector<std::shared_ptr<pika::memotable::Match>>{});
                           }
                       });
}

template<typename S>
std::shared_ptr<pika::memotable::Match>
FollowedBy<S>::packrat_match(memotable::MemoTable &table, size_t index) const {
    PIKA_CHECKED_MATCH({
                           if (auto res = S().packrat_match(table, index)) {
                               return table[key] =
                                              std::make_shared<pika::memotable::Match>(key, 0, 0,
                                                                                       std::vector<std::shared_ptr<pika::memotable::Match>>{
                                                                                               res});
                           }
                       });
}

template<typename S>
std::shared_ptr<pika::memotable::Match>
Optional<S>::packrat_match(memotable::MemoTable &table, size_t index) const {
    PIKA_CHECKED_MATCH({
                           if (auto res = S().packrat_match(table, index)) {
                               return table[key] =
                                              std::make_shared<pika::memotable::Match>(key, res->get_length(), 0,
                                                                                       std::vector<std::shared_ptr<pika::memotable::Match>>{
                                                                                               res});
                           } else {
                               return table[key] =
                                              std::make_shared<pika::memotable::Match>(key, 0, 0,
                                                                                       std::vector<std::shared_ptr<pika::memotable::Match>>{});
                           }
                       });
}

template<typename S>
std::shared_ptr<pika::memotable::Match>
Asterisks<S>::packrat_match(memotable::MemoTable &table, size_t index) const {
    PIKA_CHECKED_MATCH({
                           size_t matched_length = 0;
                           S inner{};
                           std::vector<std::shared_ptr<pika::memotable::Match>> sub_matches{};
                           while (auto res = inner.packrat_match(table, index + matched_length)) {
                               matched_length += res->get_length();
                               sub_matches.push_back(res);
                           }
                           return table[key] =
                                          std::make_shared<pika::memotable::Match>(key, matched_length, 0,
                                                                                   std::move(sub_matches));
                       }

    );
}

template<typename S>
std::shared_ptr<pika::memotable::Match>
Plus<S>::packrat_match(memotable::MemoTable &table, size_t index) const {
    PIKA_CHECKED_MATCH({
                           size_t matched_length = 0;
                           S inner{};
                           std::vector<std::shared_ptr<pika::memotable::Match>> sub_matches{};
                           while (auto res = inner.packrat_match(table, index + matched_length)) {
                               matched_length += res->get_length();
                               sub_matches.push_back(res);
                           }
                           if (!sub_matches.empty())
                               return table[key] =
                                              std::make_shared<pika::memotable::Match>(key, matched_length, 0,
                                                                                       std::move(sub_matches));
                       }

    );
}

template<typename S>
void Plus<S>::dfs_traversal(absl::flat_hash_set<std::type_index> &visited, std::vector<const Clause *> &terminals,
                            std::vector<const Clause *> &nodes) const {
    PIKA_DFS_UNARY(S)
}

template<typename S>
void Plus<S>::mark_seeds(pika::graph::ClauseTable &table) const {
    table.at(typeid(S)).candidates.push_back(this->get_instance());
}

template<typename S>
void Plus<S>::pika_match(pika::graph::ClauseTable &table) const {
    std::vector<std::shared_ptr<memotable::Match>> sub_matches;
    size_t length = 0;
    auto target = table.memo_table.template find({S().get_instance(), table.current_pos - 1 + length});
    while (target != table.memo_table.end()) {
        sub_matches.push_back(target->second);
        if (target->second->length == 0) break;
        length += target->second->length;
        target = table.memo_table.template find({S().get_instance(), table.current_pos - 1 + length});
    }
    if (!sub_matches.empty()) {
        table.try_add(this->get_instance(), length, table.template at(typeid(S)).topological_order,
                      std::move(sub_matches));
    }
}

template<typename S>
void Asterisks<S>::dfs_traversal(absl::flat_hash_set<std::type_index> &visited, std::vector<const Clause *> &terminals,
                                 std::vector<const Clause *> &nodes) const {
    PIKA_DFS_UNARY(S)
}

template<typename S>
void Asterisks<S>::mark_seeds(pika::graph::ClauseTable &table) const {
    table.at(typeid(S)).candidates.push_back(this->get_instance());
}

template<typename S>
void Asterisks<S>::pika_match(pika::graph::ClauseTable &table) const {
    std::vector<std::shared_ptr<memotable::Match>> sub_matches;
    size_t length = 0;
    auto target = table.memo_table.template find({S().get_instance(), table.current_pos - 1 + length});
    while (target != table.memo_table.end()) {
        sub_matches.push_back(target->second);
        if (target->second->length == 0) break;
        length += target->second->length;
        target = table.memo_table.template find({S().get_instance(), table.current_pos - 1 + length});
    }
    table.try_add(this->get_instance(), length, table.template at(typeid(S)).topological_order, std::move(sub_matches));
}

template<typename S>
void Optional<S>::dfs_traversal(absl::flat_hash_set<std::type_index> &visited, std::vector<const Clause *> &terminals,
                                std::vector<const Clause *> &nodes) const {
    PIKA_DFS_UNARY(S)
}

template<typename S>
void Optional<S>::mark_seeds(pika::graph::ClauseTable &table) const {
    table.at(typeid(S)).candidates.push_back(this->get_instance());
}

template<typename S>
void Optional<S>::pika_match(pika::graph::ClauseTable &table) const {
    std::vector<std::shared_ptr<memotable::Match>> sub_matches;
    size_t length = 0;
    auto target = table.memo_table.template find({S().get_instance(), table.current_pos - 1 + length});
    if (target != table.memo_table.end()) {
        sub_matches.push_back(target->second);
        length += target->second->length;
    }
    table.try_add(this->get_instance(), length, table.template at(typeid(S)).topological_order, std::move(sub_matches));
}

template<typename S>
void FollowedBy<S>::dfs_traversal(absl::flat_hash_set<std::type_index> &visited, std::vector<const Clause *> &terminals,
                                  std::vector<const Clause *> &nodes) const {
    PIKA_DFS_UNARY(S)
}

template<typename S>
void FollowedBy<S>::mark_seeds(pika::graph::ClauseTable &table) const {
    table.at(typeid(S)).candidates.push_back(this->get_instance());
}

template<typename S>
void FollowedBy<S>::pika_match(pika::graph::ClauseTable &table) const {
    if (table.memo_table.template contains({S().get_instance(), table.current_pos - 1})) {
        table.try_add(this->get_instance(), 0, table.template at(typeid(S)).topological_order, {});
    }
}

template<typename S>
void
NotFollowedBy<S>::dfs_traversal(absl::flat_hash_set<std::type_index> &visited, std::vector<const Clause *> &terminals,
                                std::vector<const Clause *> &nodes) const {
    PIKA_DFS_UNARY(S)
}

template<typename S>
void NotFollowedBy<S>::mark_seeds(pika::graph::ClauseTable &table) const {
    table.at(typeid(S)).candidates.push_back(this->get_instance());
}

template<typename S>
void NotFollowedBy<S>::pika_match(pika::graph::ClauseTable &table) const {
    if (!table.memo_table.template contains({S().get_instance(), table.current_pos - 1})) {
        table.try_add(this->get_instance(), 0, table.template at(typeid(S)).topological_order, {});
    }
}

template<typename S>
std::shared_ptr<pika::memotable::Match>
Ord<S>::packrat_match(memotable::MemoTable &table, size_t index) const {
    PIKA_CHECKED_MATCH(
            if (auto res = S().packrat_match(table, index)) {
                return table[key] =
                               std::make_shared<pika::memotable::Match>(key, res->get_length(), 0,
                                                                        std::vector<std::shared_ptr<pika::memotable::Match>>{
                                                                                res});

            }
    );
}

template<typename H, typename ...T>
std::shared_ptr<pika::memotable::Match>
Ord<H, T...>::packrat_match(memotable::MemoTable &table, size_t index) const {
    PIKA_CHECKED_MATCH(
            if (auto res = H().packrat_match(table, index)) {
                return table[key] =
                               std::make_shared<pika::memotable::Match>(key, res->get_length(), 0,
                                                                        std::vector<std::shared_ptr<pika::memotable::Match>>{
                                                                                res});
            } else {
                return Ord<T...>::packrat_match(table, index);
            }
    );
}

template<typename H, typename... T>
std::shared_ptr<pika::memotable::Match>
Seq<H, T...>::packrat_reduce(memotable::MemoTable &table, size_t index, size_t length,
                             std::vector<std::shared_ptr<pika::memotable::Match>> collection) const {
    if (auto res = H().packrat_match(table, index + length)) {
        collection.push_back(res);
        return Seq<T...>::packrat_reduce(table, index, length + res->get_length(), std::move(collection));
    } else {
        return nullptr;
    }
}

template<typename H>
std::shared_ptr<pika::memotable::Match>
Seq<H>::packrat_reduce(memotable::MemoTable &table, size_t index, size_t length,
                       std::vector<std::shared_ptr<pika::memotable::Match>> collection) const {
    if (auto res = H().packrat_match(table, index + length)) {
        auto key = pika::memotable::MemoKey(this->get_instance(), index);
        collection.push_back(res);
        return std::make_shared<pika::memotable::Match>(key, length + res->get_length(), 0, std::move(collection));
    } else {
        return nullptr;
    }
}


template<typename S>
std::shared_ptr<pika::memotable::Match>
Seq<S>::packrat_match(memotable::MemoTable &table, size_t index) const {
    PIKA_CHECKED_MATCH({
                           return packrat_reduce(table, index, 0, {});
                       }

    );
}

template<typename H, typename... T>
void Seq<H, T...>::dfs_traversal(absl::flat_hash_set<std::type_index> &visited, std::vector<const Clause *> &terminals,
                                 std::vector<const Clause *> &nodes) const {
    PIKA_DFS_CHECK({
                       _dfs_traversal(visited, terminals, nodes);
                   })
}

template<typename H, typename... T>
void Seq<H, T...>::_dfs_traversal(absl::flat_hash_set<std::type_index> &visited,
                                  std::vector<const Clause *> &terminals,
                                  std::vector<const Clause *> &nodes) const {
    H().dfs_traversal(visited, terminals, nodes);
    Seq<T...>::_dfs_traversal(visited, terminals, nodes);
}

template<typename H>
void Seq<H>::mark_seeds(pika::graph::ClauseTable &table) const {
    table.at(typeid(H)).candidates.push_back(this->get_instance());
}

template<typename H, typename... T>
void Seq<H, T...>::pika_match(pika::graph::ClauseTable &table) const {
    pika_match_unchecked(table, table.at(typeid(H)).topological_order, 0, {});
}

template<typename H, typename... T>
void Seq<H, T...>::pika_match_unchecked(pika::graph::ClauseTable &table,
                                        size_t first_idx,
                                        size_t length,
                                        std::vector<std::shared_ptr<memotable::Match>> matches) const {
    auto target = table.memo_table.template find({H().get_instance(), table.current_pos - 1 + length});
    if (target != table.memo_table.end()) {
        matches.push_back(target->second);
        Seq<T...>::pika_match_unchecked(table, first_idx, length + target->second->length, std::move(matches));
    }
}

template<typename H>
void Seq<H>::pika_match(pika::graph::ClauseTable &table) const {
    pika_match_unchecked(table, table.at(typeid(H)).topological_order, 0, {});
}

template<typename H>
void Seq<H>::pika_match_unchecked(pika::graph::ClauseTable &table,
                                  size_t first_idx,
                                  size_t length,
                                  std::vector<std::shared_ptr<memotable::Match>> matches) const {
    auto target = table.memo_table.template find({H().get_instance(), table.current_pos - 1 + length});
    if (target != table.memo_table.end()) {
        matches.push_back(target->second);
        table.try_add(this->get_instance(), length + target->second->length, first_idx, std::move(matches));
    }
}

template<typename H, typename... T>
void Seq<H, T...>::mark_seeds(pika::graph::ClauseTable &table) const {
    table.at(typeid(H)).candidates.push_back(this->get_instance());
}

template<typename H>
void Seq<H>::dfs_traversal(absl::flat_hash_set<std::type_index> &visited, std::vector<const Clause *> &terminals,
                           std::vector<const Clause *> &nodes) const {
    PIKA_DFS_CHECK({
                       _dfs_traversal(visited, terminals, nodes);
                   })
}

template<typename H>
void Seq<H>::_dfs_traversal(absl::flat_hash_set<std::type_index> &visited,
                            std::vector<const Clause *> &terminals,
                            std::vector<const Clause *> &nodes) const {
    H().dfs_traversal(visited, terminals, nodes);
    nodes.push_back(this->get_instance());
}

template<typename H, typename... T>
void Ord<H, T...>::dfs_traversal(absl::flat_hash_set<std::type_index> &visited, std::vector<const Clause *> &terminals,
                                 std::vector<const Clause *> &nodes) const {
    PIKA_DFS_CHECK({
                       _dfs_traversal(visited, terminals, nodes);
                   })
}

template<typename H, typename... T>
void Ord<H, T...>::_dfs_traversal(absl::flat_hash_set<std::type_index> &visited,
                                  std::vector<const Clause *> &terminals,
                                  std::vector<const Clause *> &nodes) const {
    H().dfs_traversal(visited, terminals, nodes);
    Ord<T...>::_dfs_traversal(visited, terminals, nodes);
}

template<typename H, typename... T>
void Ord<H, T...>::mark_seeds(pika::graph::ClauseTable &table) const {
    table.at(typeid(H)).candidates.push_back(this->get_instance());
    Ord<T...>::mark_seeds(table);
}

template<typename H, typename... T>
void Ord<H, T...>::pika_match(pika::graph::ClauseTable &table) const {
    pika_match_unchecked(table);
}

template<typename H, typename... T>
void Ord<H, T...>::pika_match_unchecked(pika::graph::ClauseTable &table) const {
    auto target = table.memo_table.template find({H().get_instance(), table.current_pos - 1});
    if (target != table.memo_table.end()) {
        table.try_add(this->get_instance(), target->second->length, table.template at(typeid(H)).topological_order,
                      {target->second});
    } else {
        Ord<T...>::pika_match_unchecked(table);
    }
}

template<typename H>
void Ord<H>::pika_match(pika::graph::ClauseTable &table) const {
    pika_match_unchecked(table);
}

template<typename H>
void Ord<H>::pika_match_unchecked(pika::graph::ClauseTable &table) const {
    auto target = table.memo_table.template find({H().get_instance(), table.current_pos - 1});
    if (target != table.memo_table.end()) {
        table.try_add(this->get_instance(), target->second->length, table.template at(typeid(H)).topological_order,
                      {target->second});
    }
}

template<typename H>
void Ord<H>::mark_seeds(pika::graph::ClauseTable &table) const {
    table.at(typeid(H)).candidates.push_back(this->get_instance());
}

template<typename H>
void Ord<H>::dfs_traversal(absl::flat_hash_set<std::type_index> &visited, std::vector<const Clause *> &terminals,
                           std::vector<const Clause *> &nodes) const {
    PIKA_DFS_CHECK({
                       _dfs_traversal(visited, terminals, nodes);
                   })
}

template<typename H>
void Ord<H>::_dfs_traversal(absl::flat_hash_set<std::type_index> &visited,
                            std::vector<const Clause *> &terminals,
                            std::vector<const Clause *> &nodes) const {
    H().dfs_traversal(visited, terminals, nodes);
    nodes.push_back(this->get_instance());
}


template<typename H, typename ...T>
std::shared_ptr<pika::memotable::Match>
Seq<H, T...>::packrat_match(memotable::MemoTable &table, size_t index) const {
    PIKA_CHECKED_MATCH(
            {
                return packrat_reduce(table, index, 0, {});
            }
    );
}

#endif // PIKA_CLAUSE_IPP
