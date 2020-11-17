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

bool pika::memotable::Match::operator<(const pika::memotable::Match& that) const
{
    return this->key.start_position < that.key.start_position;
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

pika::memotable::MemoTable::OrderedMatches
pika::memotable::MemoTable::ordered_matches(std::type_index clause) const
{
    absl::btree_map<size_t, std::shared_ptr<pika::memotable::Match>> matches;
    for (auto& entry : *this)
    {
        if (entry.first.clause_type == clause)
        {
            matches.insert({entry.second->key.start_position, entry.second});
        }
    }
    return matches;
}

pika::memotable::MemoTable::OrderedTable
pika::memotable::MemoTable::ordered_matches() const
{
    OrderedTable table;
    for (auto& entry : *this)
    {
        table[entry.first.clause_type].insert(
            {entry.second->key.start_position, entry.second});
    }
    return table;
}

std::vector<std::shared_ptr<pika::memotable::Match>>
pika::nonoverlapping_matches(
    const pika::memotable::MemoTable::OrderedMatches& matches)
{
    std::vector<std::shared_ptr<pika::memotable::Match>> result;
    for (auto i = matches.begin(); i != matches.end(); ++i)
    {
        auto end = i->first + i->second->get_length();
        result.push_back(i->second);
        while (true)
        {
            auto j = std::next(i);
            if (j != matches.end() && j->first < end)
            {
                i++;
            }
            else
                break;
        }
    }
    return result;
}

void pika::utils::IntervalUnion::add_interval(size_t start, size_t end) // [start, end)
{
    if (end <= start)
    {
        return;
    }
    auto less_equal = this->segments.upper_bound(start);
    if (less_equal != segments.begin()) {
        less_equal = std::prev(less_equal);
    } else {
        less_equal = this->segments.end();
    }
    std::pair<size_t, size_t> entry;
    if (less_equal == this->segments.end() || less_equal->second < start)
    // for less_equal->second == start, it is still needed to merge two intervals
    {
        entry.first = start;
        entry.second = end;
    }
    else
    {
        entry.first = less_equal->first;
        entry.second = std::max(less_equal->second, end);
        // remove original one seems okay to me.
        this->segments.erase(less_equal);
    }

    auto greater_equal = this->segments.lower_bound(start);
    // not need to be strict
    // previous equal one must be removed if ever exists
    if (greater_equal != this->segments.end() && greater_equal->first <= end)
    {
        entry.second = std::max(entry.second, greater_equal->second);
        this->segments.erase(greater_equal);
    }
    this->segments.insert(entry);
}

pika::utils::IntervalUnion
pika::utils::IntervalUnion::invert(size_t start, size_t end) const
{
    IntervalUnion result;
    auto prev = start;
    if (!segments.empty())
    {
        for (auto& entry : segments)
        {
            if (entry.first > end)
                break;
            if (entry.first > prev)
            {
                result.add_interval(prev, entry.first);
            }
            prev = entry.second;
        }
        auto& ending = *segments.rbegin();
        if (ending.second < end)
        {
            result.add_interval(ending.second, end);
        }
    }
    else
    {
        result.add_interval(start, end);
    }
    return result;
}

bool pika::utils::IntervalUnion::is_overlap(size_t start, size_t end) const
{
    auto less_equal = this->segments.upper_bound(start);
    if (less_equal != segments.begin()) {
        less_equal = std::prev(less_equal);
    } else {
        less_equal = this->segments.end();
    }
    if (less_equal != segments.end())
    {
        if ((start < less_equal->second && start >= less_equal->first) ||
            (end <= less_equal->second && end > less_equal->first))
        {
            return true;
        }
    }

    auto greater_equal = segments.lower_bound(start);
    if (greater_equal != segments.end())
    {
        if ((start < greater_equal->second && start >= greater_equal->first) ||
            (end <= greater_equal->second && end > greater_equal->first))
        {
            return true;
        }
    }

    return false;
}

size_t pika::utils::IntervalUnion::size() const {
    return segments.size();
}
