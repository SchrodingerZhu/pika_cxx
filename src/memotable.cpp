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

bool pika::memotable::Match::operator<(const pika::memotable::Match &that) const {
    return this->key.start_position < that.key.start_position;
}

pika::memotable::MemoTable::MemoTable(std::string_view target) : target(target),
                                                                 absl::flat_hash_map<MemoKey, std::shared_ptr<Match>>() {}

char pika::memotable::MemoTable::get_char(size_t index) const
{
    return target[index];
}

bool pika::memotable::MemoTable::at_end(size_t index) const
{
    return target.size() == index;
}

pika::memotable::MemoTable::OrderedMatches
pika::memotable::MemoTable::ordered_matches(std::type_index clause) const {
    absl::btree_map<size_t, std::shared_ptr<pika::memotable::Match>> matches;
    for (auto &entry : *this) {
        if (entry.first.clause_type == clause) {
            matches.insert({entry.second->key.start_position, entry.second});
        }
    }
    return matches;
}

pika::memotable::MemoTable::OrderedTable pika::memotable::MemoTable::ordered_matches() const {
    OrderedTable table;
    for (auto &entry : *this) {
        table[entry.first.clause_type].insert({entry.second->key.start_position, entry.second});
    }
    return table;
}

std::vector<std::shared_ptr<pika::memotable::Match>>
pika::nonoverlapping_matches(const pika::memotable::MemoTable::OrderedMatches &matches) {
    std::vector<std::shared_ptr<pika::memotable::Match>> result;
    for (auto i = matches.begin(); i != matches.end(); ++i) {
        auto end = i->first + i->second->get_length();
        result.push_back(i->second);
        while (true) {
            auto j = std::next(i);
            if (j != matches.end() && j->first < end) {
                i++;
            } else break;
        }
    }
}

void pika::utils::interval_union::add_interval(size_t start, size_t end) {
    if (end < start) {
        throw std::runtime_error("invalid interval: start > end");
    }
    const auto lower_bound = this->segments.lower_bound(start);
    std::pair <size_t, size_t> entry;
    if (lower_bound == this->segments.end() || lower_bound->second < start) {
        entry.first = start;
        entry.second = end;
    } else {
        entry.first = lower_bound->first;
        entry.second = std::max(lower_bound->second, end);
        // remove original one seems okay to me.
        this->segments.erase(lower_bound);
    }

    auto upper_bound = this->segments.upper_bound(start);
    // not need to be strict
    // previous equal one must be removed if ever exists
    if (upper_bound != this->segments.end() && upper_bound->first <= end) {
        entry.second = std::max(entry.second, upper_bound->second);
        this->segments.erase(upper_bound);
    }
    this->segments.insert(entry);
}

pika::utils::interval_union pika::utils::interval_union::invert(size_t start, size_t end) const {
    interval_union result;
    auto prev = start;
    if (!segments.empty()) {
        for (auto & entry : segments) {
            if (entry.first > end) break;
            if (entry.first > prev) {
                result.add_interval(prev, entry.first);
            }
            prev = entry.second;
        }
        auto & ending = *segments.end();
        if (ending.second < end) {
            result.add_interval(ending.second, end);
        }
    } else {
        result.add_interval(start, end);
    }
    return result;
}

bool pika::utils::interval_union::is_overlap(size_t start, size_t end) const {
    auto lower_bound = segments.lower_bound(start);
    if (lower_bound != segments.end()) {
        if (std::max(end, lower_bound->second) - std::min(start, lower_bound->first)
            < (end - start) + (lower_bound->second - lower_bound->first)) {
            return true;
        }
    }

    auto upper_bound = segments.upper_bound(start);
    if (upper_bound != segments.end()) {
        if (std::max(end, upper_bound->second) - std::min(start, upper_bound->first)
            < (end - start) + (upper_bound->second - upper_bound->first)) {
            return true;
        }
    }

    return false;

}


