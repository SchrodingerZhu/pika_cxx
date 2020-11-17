#ifndef PIKA_CLAUSE_IPP
#define PIKA_CLAUSE_IPP

#include <pika/clause.hpp>
#include <pika/graph.hpp>
#include <pika/memotable.hpp>
#include <typeindex>

template<char C>
std::shared_ptr<pika::memotable::Match> pika::clause::Char<C>::packrat_match(
    memotable::MemoTable& table, size_t index) const
{
    PIKA_CHECKED_MATCH({
        if (!table.at_end(index) && table.get_char(index) == C)
        {
            return table[key] = std::make_shared<pika::memotable::Match>(
                       key,
                       1,
                       0,
                       std::vector<std::shared_ptr<pika::memotable::Match>>{});
        }
    });
}

template<char C>
void pika::clause::Char<C>::pika_match(pika::graph::ClauseTable& table) const
{
    if (table.get_current() == C)
    {
        table.try_add(this->get_instance(), 1, 0, {});
    }
}

template<char Start, char End>
std::shared_ptr<pika::memotable::Match>
pika::clause::CharRange<Start, End>::packrat_match(
    memotable::MemoTable& table, size_t index) const
{
    PIKA_CHECKED_MATCH({
        if (!table.at_end(index) && table.get_char(index) >= Start &&
            table.get_char(index) <= End)
        {
            return table[key] = std::make_shared<pika::memotable::Match>(
                       key,
                       1,
                       0,
                       std::vector<std::shared_ptr<pika::memotable::Match>>{});
        }
    });
}

template<char Start, char End>
void pika::clause::CharRange<Start, End>::pika_match(
    pika::graph::ClauseTable& table) const
{
    auto current = table.get_current();
    if (current >= Start && current <= End)
    {
        table.try_add(this->get_instance(), 1, 0, {});
    }
}

template<typename S>
std::shared_ptr<pika::memotable::Match>
pika::clause::NotFollowedBy<S>::packrat_match(
    memotable::MemoTable& table, size_t index) const
{
    PIKA_CHECKED_MATCH({
        if (!S().packrat_match(table, index))
        {
            return table[key] = std::make_shared<pika::memotable::Match>(
                       key,
                       0,
                       0,
                       std::vector<std::shared_ptr<pika::memotable::Match>>{});
        }
    });
}

template<typename S>
std::shared_ptr<pika::memotable::Match>
pika::clause::FollowedBy<S>::packrat_match(
    memotable::MemoTable& table, size_t index) const
{
    PIKA_CHECKED_MATCH({
        if (auto res = S().packrat_match(table, index))
        {
            return table[key] = std::make_shared<pika::memotable::Match>(
                       key,
                       0,
                       0,
                       std::vector<std::shared_ptr<pika::memotable::Match>>{
                           res});
        }
    });
}

template<typename S>
std::shared_ptr<pika::memotable::Match>
pika::clause::Optional<S>::packrat_match(
    memotable::MemoTable& table, size_t index) const
{
    PIKA_CHECKED_MATCH({
        if (auto res = S().packrat_match(table, index))
        {
            return table[key] = std::make_shared<pika::memotable::Match>(
                       key,
                       res->get_length(),
                       0,
                       std::vector<std::shared_ptr<pika::memotable::Match>>{
                           res});
        }
        else
        {
            return table[key] = std::make_shared<pika::memotable::Match>(
                       key,
                       0,
                       0,
                       std::vector<std::shared_ptr<pika::memotable::Match>>{});
        }
    });
}

template<typename S>
std::shared_ptr<pika::memotable::Match>
pika::clause::Asterisks<S>::packrat_match(
    memotable::MemoTable& table, size_t index) const
{
    PIKA_CHECKED_MATCH({
        size_t matched_length = 0;
        S inner{};
        std::vector<std::shared_ptr<pika::memotable::Match>> sub_matches{};
        while (auto res = inner.packrat_match(table, index + matched_length))
        {
            matched_length += res->get_length();
            sub_matches.push_back(res);
            if (res->get_length() == 0)
                break;
        }
        return table[key] = std::make_shared<pika::memotable::Match>(
                   key, matched_length, 0, std::move(sub_matches));
    }

    );
}

template<typename S>
std::shared_ptr<pika::memotable::Match> pika::clause::Plus<S>::packrat_match(
    memotable::MemoTable& table, size_t index) const
{
    PIKA_CHECKED_MATCH({
        size_t matched_length = 0;
        S inner{};
        std::vector<std::shared_ptr<pika::memotable::Match>> sub_matches{};
        while (auto res = inner.packrat_match(table, index + matched_length))
        {
            matched_length += res->get_length();
            sub_matches.push_back(res);
            if (res->get_length() == 0)
                break;
        }
        if (!sub_matches.empty())
            return table[key] = std::make_shared<pika::memotable::Match>(
                       key, matched_length, 0, std::move(sub_matches));
    }

    );
}

template<typename S>
void pika::clause::Plus<S>::dfs_traversal(
    absl::flat_hash_set<std::type_index>& visited,
    std::vector<const Clause*>& terminals,
    std::vector<const Clause*>& nodes) const
{
    PIKA_DFS_UNARY(S)
}

template<typename S>
void pika::clause::Plus<S>::mark_seeds(pika::graph::ClauseTable& table) const
{
    table.at(typeid(S)).candidates.push_back(this->get_instance());
}

template<typename S>
void pika::clause::Plus<S>::pika_match(pika::graph::ClauseTable& table) const
{
    std::vector<std::shared_ptr<memotable::Match>> sub_matches;
    size_t length = 0;
    auto target = table.memo_table.template find(
        {S().get_instance(), table.current_pos - 1 + length});
    while (target != table.memo_table.end())
    {
        sub_matches.push_back(target->second);
        if (target->second->length == 0)
            break;
        length += target->second->length;
        auto reduced = table.memo_table.template find(
            {this->get_instance(), table.current_pos - 1 + length});
        if (reduced != table.memo_table.end())
        {
            length += reduced->second->length;
            sub_matches.push_back(reduced->second);
            break;
        }
        target = table.memo_table.template find(
            {S().get_instance(), table.current_pos - 1 + length});
    }
    if (!sub_matches.empty())
    {
        table.try_add(this->get_instance(), length, 0, std::move(sub_matches));
    }
}

template<typename S>
void pika::clause::Asterisks<S>::dfs_traversal(
    absl::flat_hash_set<std::type_index>& visited,
    std::vector<const Clause*>& terminals,
    std::vector<const Clause*>& nodes) const
{
    PIKA_DFS_UNARY(S)
}

template<typename S>
void pika::clause::Asterisks<S>::mark_seeds(
    pika::graph::ClauseTable& table) const
{
    table.at(typeid(S)).candidates.push_back(this->get_instance());
}

template<typename S>
void pika::clause::Asterisks<S>::pika_match(
    pika::graph::ClauseTable& table) const
{
    std::vector<std::shared_ptr<memotable::Match>> sub_matches;
    size_t length = 0;
    auto target = table.memo_table.template find(
        {S().get_instance(), table.current_pos - 1 + length});
    while (target != table.memo_table.end())
    {
        sub_matches.push_back(target->second);
        if (target->second->length == 0)
            break;
        length += target->second->length;
        auto reduced = table.memo_table.template find(
            {this->get_instance(), table.current_pos - 1 + length});
        if (reduced != table.memo_table.end())
        {
            length += reduced->second->length;
            sub_matches.push_back(reduced->second);
            break;
        }
        target = table.memo_table.template find(
            {S().get_instance(), table.current_pos - 1 + length});
    }
    table.try_add(this->get_instance(), length, 0, std::move(sub_matches));
}

template<typename S>
void pika::clause::Optional<S>::dfs_traversal(
    absl::flat_hash_set<std::type_index>& visited,
    std::vector<const Clause*>& terminals,
    std::vector<const Clause*>& nodes) const
{
    PIKA_DFS_UNARY(S)
}

template<typename S>
void pika::clause::Optional<S>::mark_seeds(
    pika::graph::ClauseTable& table) const
{
    table.at(typeid(S)).candidates.push_back(this->get_instance());
}

template<typename S>
void pika::clause::Optional<S>::pika_match(
    pika::graph::ClauseTable& table) const
{
    std::vector<std::shared_ptr<memotable::Match>> sub_matches;
    size_t length = 0;
    auto target = table.memo_table.template find(
        {S().get_instance(), table.current_pos - 1 + length});
    if (target != table.memo_table.end())
    {
        sub_matches.push_back(target->second);
        length += target->second->length;
    }
    table.try_add(this->get_instance(), length, 0, std::move(sub_matches));
}

template<typename S>
void pika::clause::FollowedBy<S>::dfs_traversal(
    absl::flat_hash_set<std::type_index>& visited,
    std::vector<const Clause*>& terminals,
    std::vector<const Clause*>& nodes) const
{
    PIKA_DFS_UNARY(S)
}

template<typename S>
void pika::clause::FollowedBy<S>::mark_seeds(
    pika::graph::ClauseTable& table) const
{
    table.at(typeid(S)).candidates.push_back(this->get_instance());
}

template<typename S>
void pika::clause::FollowedBy<S>::pika_match(
    pika::graph::ClauseTable& table) const
{
    if (table.memo_table.template contains(
            {S().get_instance(), table.current_pos - 1}))
    {
        table.try_add(this->get_instance(), 0, 0, {});
    }
}

template<typename S>
void pika::clause::NotFollowedBy<S>::dfs_traversal(
    absl::flat_hash_set<std::type_index>& visited,
    std::vector<const Clause*>& terminals,
    std::vector<const Clause*>& nodes) const
{
    PIKA_DFS_UNARY(S)
}

template<typename S>
void pika::clause::NotFollowedBy<S>::mark_seeds(
    pika::graph::ClauseTable& table) const
{
    table.at(typeid(S)).candidates.push_back(this->get_instance());
}

template<typename S>
void pika::clause::NotFollowedBy<S>::pika_match(
    pika::graph::ClauseTable& table) const
{
    if (!table.memo_table.template contains(
            {S().get_instance(), table.current_pos - 1}))
    {
        table.try_add(this->get_instance(), 0, 0, {});
    }
}

template<typename S>
std::shared_ptr<pika::memotable::Match> pika::clause::Ord<S>::packrat_match(
    memotable::MemoTable& table, size_t index) const
{
    PIKA_CHECKED_MATCH(if (auto res = S().packrat_match(table, index)) {
        return table[key] = std::make_shared<pika::memotable::Match>(
                   key,
                   res->get_length(),
                   0,
                   std::vector<std::shared_ptr<pika::memotable::Match>>{res});
    });
}

template<typename H, typename... T>
std::shared_ptr<pika::memotable::Match>
pika::clause::Ord<H, T...>::packrat_match(
    memotable::MemoTable& table, size_t index) const
{
    PIKA_CHECKED_MATCH(
        if (auto res = H().packrat_match(table, index)) {
            return table[key] = std::make_shared<pika::memotable::Match>(
                       key,
                       res->get_length(),
                       0,
                       std::vector<std::shared_ptr<pika::memotable::Match>>{
                           res});
        } else { return Ord<T...>::packrat_match(table, index); });
}

template<typename H, typename... T>
std::shared_ptr<pika::memotable::Match>
pika::clause::Seq<H, T...>::packrat_reduce(
    memotable::MemoTable& table,
    size_t index,
    size_t length,
    std::vector<std::shared_ptr<pika::memotable::Match>> collection) const
{
    if (auto res = H().packrat_match(table, index + length))
    {
        collection.push_back(res);
        return Seq<T...>::packrat_reduce(
            table, index, length + res->get_length(), std::move(collection));
    }
    else
    {
        return nullptr;
    }
}

template<typename H>
std::shared_ptr<pika::memotable::Match> pika::clause::Seq<H>::packrat_reduce(
    memotable::MemoTable& table,
    size_t index,
    size_t length,
    std::vector<std::shared_ptr<pika::memotable::Match>> collection) const
{
    if (auto res = H().packrat_match(table, index + length))
    {
        auto key = pika::memotable::MemoKey(this->get_instance(), index);
        collection.push_back(res);
        return std::make_shared<pika::memotable::Match>(
            key, length + res->get_length(), 0, std::move(collection));
    }
    else
    {
        return nullptr;
    }
}

template<typename S>
std::shared_ptr<pika::memotable::Match> pika::clause::Seq<S>::packrat_match(
    memotable::MemoTable& table, size_t index) const
{
    PIKA_CHECKED_MATCH({ return packrat_reduce(table, index, 0, {}); }

    );
}

template<typename H, typename... T>
void pika::clause::Seq<H, T...>::dfs_traversal(
    absl::flat_hash_set<std::type_index>& visited,
    std::vector<const Clause*>& terminals,
    std::vector<const Clause*>& nodes) const
{
    PIKA_DFS_CHECK({ _dfs_traversal(visited, terminals, nodes); })
}

template<typename H, typename... T>
void pika::clause::Seq<H, T...>::_dfs_traversal(
    absl::flat_hash_set<std::type_index>& visited,
    std::vector<const Clause*>& terminals,
    std::vector<const Clause*>& nodes) const
{
    H().dfs_traversal(visited, terminals, nodes);
    Seq<T...>::_dfs_traversal(visited, terminals, nodes);
}

template<typename H>
void pika::clause::Seq<H>::mark_seeds(pika::graph::ClauseTable& table) const
{
    table.at(typeid(H)).candidates.push_back(this->get_instance());
}

template<typename H, typename... T>
void pika::clause::Seq<H, T...>::pika_match(
    pika::graph::ClauseTable& table) const
{
    pika_match_unchecked(table, 0, {});
}

template<typename H, typename... T>
void pika::clause::Seq<H, T...>::pika_match_unchecked(
    pika::graph::ClauseTable& table,
    size_t length,
    std::vector<std::shared_ptr<memotable::Match>> matches) const
{
    auto target = table.memo_table.template find(
        {H().get_instance(), table.current_pos - 1 + length});
    if (target != table.memo_table.end())
    {
        matches.push_back(target->second);
        Seq<T...>::pika_match_unchecked(
            table, length + target->second->length, std::move(matches));
    }
}

template<typename H>
void pika::clause::Seq<H>::pika_match(pika::graph::ClauseTable& table) const
{
    pika_match_unchecked(table, 0, {});
}

template<typename H>
void pika::clause::Seq<H>::pika_match_unchecked(
    pika::graph::ClauseTable& table,
    size_t length,
    std::vector<std::shared_ptr<memotable::Match>> matches) const
{
    auto target = table.memo_table.template find(
        {H().get_instance(), table.current_pos - 1 + length});
    if (target != table.memo_table.end())
    {
        matches.push_back(target->second);
        table.try_add(
            this->get_instance(),
            length + target->second->length,
            0,
            std::move(matches));
    }
}

template<typename H, typename... T>
void pika::clause::Seq<H, T...>::mark_seeds(
    pika::graph::ClauseTable& table) const
{
    table.at(typeid(H)).candidates.push_back(this->get_instance());
}

template<typename H>
void pika::clause::Seq<H>::dfs_traversal(
    absl::flat_hash_set<std::type_index>& visited,
    std::vector<const Clause*>& terminals,
    std::vector<const Clause*>& nodes) const
{
    PIKA_DFS_CHECK({ _dfs_traversal(visited, terminals, nodes); })
}

template<typename H>
void pika::clause::Seq<H>::_dfs_traversal(
    absl::flat_hash_set<std::type_index>& visited,
    std::vector<const Clause*>& terminals,
    std::vector<const Clause*>& nodes) const
{
    H().dfs_traversal(visited, terminals, nodes);
    nodes.push_back(this->get_instance());
}

template<typename H, typename... T>
void pika::clause::Ord<H, T...>::dfs_traversal(
    absl::flat_hash_set<std::type_index>& visited,
    std::vector<const Clause*>& terminals,
    std::vector<const Clause*>& nodes) const
{
    PIKA_DFS_CHECK({ _dfs_traversal(visited, terminals, nodes); })
}

template<typename H, typename... T>
void pika::clause::Ord<H, T...>::_dfs_traversal(
    absl::flat_hash_set<std::type_index>& visited,
    std::vector<const Clause*>& terminals,
    std::vector<const Clause*>& nodes) const
{
    H().dfs_traversal(visited, terminals, nodes);
    Ord<T...>::_dfs_traversal(visited, terminals, nodes);
}

template<typename H, typename... T>
void pika::clause::Ord<H, T...>::mark_seeds(
    pika::graph::ClauseTable& table) const
{
    table.at(typeid(H)).candidates.push_back(this->get_instance());
    Ord<T...>::mark_seeds(table);
}

template<typename H, typename... T>
void pika::clause::Ord<H, T...>::pika_match(
    pika::graph::ClauseTable& table) const
{
    pika_match_unchecked(table, 0);
}

template<typename H, typename... T>
void pika::clause::Ord<H, T...>::pika_match_unchecked(
    pika::graph::ClauseTable& table, size_t order) const
{
    auto target = table.memo_table.template find(
        {H().get_instance(), table.current_pos - 1});
    if (target != table.memo_table.end())
    {
        table.try_add(
            this->get_instance(),
            target->second->length,
            order,
            {target->second});
    }
    else
    {
        Ord<T...>::pika_match_unchecked(table, order + 1);
    }
}

template<typename H>
void pika::clause::Ord<H>::pika_match(pika::graph::ClauseTable& table) const
{
    pika_match_unchecked(table, 0);
}

template<typename H>
void pika::clause::Ord<H>::pika_match_unchecked(
    pika::graph::ClauseTable& table, size_t order) const
{
    auto target = table.memo_table.template find(
        {H().get_instance(), table.current_pos - 1});
    if (target != table.memo_table.end())
    {
        table.try_add(
            this->get_instance(),
            target->second->length,
            order,
            {target->second});
    }
}

template<typename H>
void pika::clause::Ord<H>::mark_seeds(pika::graph::ClauseTable& table) const
{
    table.at(typeid(H)).candidates.push_back(this->get_instance());
}

template<typename H>
void pika::clause::Ord<H>::dfs_traversal(
    absl::flat_hash_set<std::type_index>& visited,
    std::vector<const Clause*>& terminals,
    std::vector<const Clause*>& nodes) const
{
    PIKA_DFS_CHECK({ _dfs_traversal(visited, terminals, nodes); })
}

template<typename H>
void pika::clause::Ord<H>::_dfs_traversal(
    absl::flat_hash_set<std::type_index>& visited,
    std::vector<const Clause*>& terminals,
    std::vector<const Clause*>& nodes) const
{
    H().dfs_traversal(visited, terminals, nodes);
    nodes.push_back(this->get_instance());
}

template<typename H, typename... T>
std::shared_ptr<pika::memotable::Match>
pika::clause::Seq<H, T...>::packrat_match(
    memotable::MemoTable& table, size_t index) const
{
    PIKA_CHECKED_MATCH({ return packrat_reduce(table, index, 0, {}); });
}

#endif // PIKA_CLAUSE_IPP
