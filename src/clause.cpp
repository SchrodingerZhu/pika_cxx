#include <pika/clause.hpp>
#include <pika/graph.hpp>
#include <pika/memotable.hpp>

std::optional<std::string_view> pika::clause::Clause::label() const
{
    return std::nullopt;
}

std::string_view pika::clause::Clause::display() const
{
    return "Clause";
}

void pika::clause::Clause::dump_inner(
    std::ostream& output, absl::flat_hash_set<std::type_index>& visited) const
{
    if (!label() || visited.contains(typeid(*this)))
    {
        return;
    }
    else
    {
        visited.insert(typeid(*this));
        output << label().value() << " <- " << display() << std::endl;
    }
}

void pika::clause::Clause::dump(std::ostream& output) const
{
    auto visited = absl::flat_hash_set<std::type_index>();
    dump_inner(output, visited);
}

bool pika::clause::Clause::active() const
{
    return false;
}

std::shared_ptr<pika::memotable::Match> pika::clause::Clause::packrat_match(
    pika::memotable::MemoTable& table, size_t index) const
{
    return nullptr;
}

std::shared_ptr<pika::memotable::Match> pika::clause::First::packrat_match(
    pika::memotable::MemoTable& table, size_t index) const
{
    PIKA_CHECKED_MATCH(if (index == 0) {
        return table[key] = std::make_shared<pika::memotable::Match>(
                   key,
                   0,
                   0,
                   std::vector<std::shared_ptr<pika::memotable::Match>>{});
    });
}

pika::type_utils::BaseType pika::clause::First::get_base_type() const noexcept
{
    return pika::type_utils::BaseType::First;
}

void pika::clause::First::pika_match(pika::graph::ClauseTable& table) const
{
    if (table.current_pos == 1)
    {
        table.try_add(this->get_instance(), 0, 0, {});
    }
}

std::shared_ptr<pika::memotable::Match> pika::clause::Nothing::packrat_match(
    pika::memotable::MemoTable& table, size_t index) const
{
    PIKA_CHECKED_MATCH({
        return table[key] = std::make_shared<pika::memotable::Match>(
                   key,
                   0,
                   0,
                   std::vector<std::shared_ptr<pika::memotable::Match>>{});
    });
}

pika::type_utils::BaseType pika::clause::Nothing::get_base_type() const noexcept
{
    return pika::type_utils::BaseType::Nothing;
}

void pika::clause::Nothing::pika_match(pika::graph::ClauseTable& table) const
{
    table.try_add(this->get_instance(), 0, 0, {});
}

std::shared_ptr<pika::memotable::Match> pika::clause::Any::packrat_match(
    pika::memotable::MemoTable& table, size_t index) const
{
    PIKA_CHECKED_MATCH(if (!table.at_end(index)) {
        return table[key] = std::make_shared<pika::memotable::Match>(
                   key,
                   1,
                   0,
                   std::vector<std::shared_ptr<pika::memotable::Match>>{});
    });
}

pika::type_utils::BaseType pika::clause::Any::get_base_type() const noexcept
{
    return pika::type_utils::BaseType::Any;
}

void pika::clause::Any::pika_match(pika::graph::ClauseTable& table) const
{
    if (!table.eof())
    {
        table.try_add(this->get_instance(), 1, 0, {});
    }
}

pika::type_utils::BaseType pika::clause::Clause::get_base_type() const noexcept
{
    return pika::type_utils::BaseType::Error;
}

void pika::clause::Clause::mark_seeds(pika::graph::ClauseTable& table) const {}

pika::type_utils::BaseType
pika::clause::_internal::Char::get_base_type() const noexcept
{
    return pika::type_utils::BaseType::Char;
}

pika::type_utils::BaseType
pika::clause::_internal::CharRange::get_base_type() const noexcept
{
    return pika::type_utils::BaseType::CharRange;
}

pika::type_utils::BaseType
pika::clause::_internal::NotFollowedBy::get_base_type() const noexcept
{
    return pika::type_utils::BaseType::NotFollowedBy;
}

pika::type_utils::BaseType
pika::clause::_internal::FollowedBy::get_base_type() const noexcept
{
    return pika::type_utils::BaseType::FollowedBy;
}

pika::type_utils::BaseType
pika::clause::_internal::Optional::get_base_type() const noexcept
{
    return pika::type_utils::BaseType::Optional;
}

pika::type_utils::BaseType
pika::clause::_internal::Asterisks::get_base_type() const noexcept
{
    return pika::type_utils::BaseType::Asterisks;
}

pika::type_utils::BaseType
pika::clause::_internal::Plus::get_base_type() const noexcept
{
    return pika::type_utils::BaseType::Plus;
}

pika::type_utils::BaseType
pika::clause::_internal::Ord::get_base_type() const noexcept
{
    return pika::type_utils::BaseType::Ord;
}

pika::type_utils::BaseType
pika::clause::_internal::Seq::get_base_type() const noexcept
{
    return pika::type_utils::BaseType::Seq;
}

void pika::clause::_internal::Terminal::dfs_traversal(
    absl::flat_hash_set<std::type_index>& visited,
    std::vector<const Clause*>& terminals,
    std::vector<const Clause*>& nodes) const
{
    PIKA_DFS_CHECK({ terminals.push_back(this->get_instance()); })
}
