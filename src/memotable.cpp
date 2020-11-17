#include <pika/memotable.hpp>

bool pika::memotable::MemoKey::operator==(const MemoKey& that) const noexcept
{
    return that.start_position == start_position &&
        that.clause_type == clause_type;
}

pika::memotable::MemoKey::MemoKey(
    const pika::clause::Clause* tag, size_t start_position) noexcept
: start_position(start_position), clause_type(typeid(*tag)), tag(tag)
{}

pika::type_utils::BaseType
pika::memotable::MemoKey::get_base_type() const noexcept
{
    return tag->get_base_type();
}

pika::memotable::Match::Match(
    pika::memotable::MemoKey key,
    size_t length,
    size_t sub_fst_idx,
    std::vector<std::shared_ptr<Match>> sub_matches)
: key(std::move(key)),
  length(length),
  sub_fst_idx(sub_fst_idx),
  sub_matches(std::move(sub_matches))
{}

bool pika::memotable::Match::is_better_than(const pika::memotable::Match& that)
{
    if (&that == this)
    {
        return false;
    }
    return (key.get_base_type() == pika::type_utils::BaseType::Ord &&
            this->sub_fst_idx < that.sub_fst_idx) ||
        this->length > that.length;
}

size_t pika::memotable::Match::get_length() const
{
    return length;
}

pika::memotable::MemoTable::MemoTable(std::string_view target)
: target(target), absl::flat_hash_map<MemoKey, std::shared_ptr<Match>>()
{}

char pika::memotable::MemoTable::get_char(size_t index) const
{
    return target[index];
}

bool pika::memotable::MemoTable::at_end(size_t index) const
{
    return target.size() == index;
}
