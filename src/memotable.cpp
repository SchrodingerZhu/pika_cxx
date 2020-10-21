#include <pika/memotable.hpp>

bool pika::memotable::MemoKey::operator==(const MemoKey &that) const noexcept {
    return that.start_position == start_position &&
           that.clause_type == clause_type;
}

pika::memotable::MemoKey::MemoKey(const pika::clause::Clause *const tag, size_t start_position) noexcept: base_type(
        type_utils::get_base_type<clause::Clause>()), start_position(start_position),
                                                                                                          clause_type(typeid(*tag)), tag(tag) {}

pika::memotable::Match::Match(pika::memotable::MemoKey key, size_t length, size_t sub_fst_idx,
                              std::vector<std::shared_ptr<Match>> sub_matches)
        : key(std::move(key)), length(length), sub_fst_idx(sub_fst_idx),
          sub_matches(std::move(sub_matches)) {}

std::vector<std::pair<std::string_view, pika::memotable::Match>> pika::memotable::Match::get_sub_matches() {
    throw std::runtime_error("unimplemented"); /// TODO: implement this
    return this->get_sub_matches();
}

bool pika::memotable::Match::is_better_than(const pika::memotable::Match &that) {
    if (&that == this) {
        return false;
    }
    return (key.is_based_on<pika::clause::First>()
            && this->sub_fst_idx < that.sub_fst_idx)
           || this->length > that.length;
}

size_t pika::memotable::Match::get_length() const {return length; }

pika::memotable::MemoTable::MemoTable(std::string_view target) : target(target), absl::flat_hash_map<MemoKey, std::shared_ptr<Match>>() {}

char pika::memotable::MemoTable::get_char(size_t index) const {
    return target[index];
}

bool pika::memotable::MemoTable::at_end(size_t index) const {
    return target.size() == index;
}
