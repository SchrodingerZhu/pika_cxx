#include <pika/clause.hpp>
#include <pika/memotable.hpp>

std::optional<std::string_view> pika::clause::Clause::label() const {
    return std::nullopt;
}

std::string_view pika::clause::Clause::display() const {
    return "Clause";
}

void pika::clause::Clause::dump_inner(std::ostream &output, absl::flat_hash_set<std::type_index> &visited) const {
    if (!label() || visited.contains(typeid(*this))) {
        return;
    } else {
        visited.insert(typeid(*this));
        output << label().value() << " <- " << display() << std::endl;
    }
}

void pika::clause::Clause::dump(std::ostream &output) const {
    auto visited = absl::flat_hash_set<std::type_index>();
    dump_inner(output, visited);
}

bool pika::clause::Clause::active() const {
    return false;
}

std::shared_ptr<pika::memotable::Match>
pika::clause::Clause::packrat_match(pika::memotable::PackratMemoTable &table, size_t index) const {
    return nullptr;
}

std::shared_ptr<pika::memotable::Match>
pika::clause::First::packrat_match(pika::memotable::PackratMemoTable &table, size_t index) const {
    PIKA_CHECKED_MATCH(
            if (index == 0) {
                return table[key] =
                               std::make_shared<pika::memotable::Match>(key, 0, 0,
                                                                        std::vector<std::shared_ptr<pika::memotable::Match>>{});
            }
    );
}

std::shared_ptr<pika::memotable::Match>
pika::clause::Nothing::packrat_match(pika::memotable::PackratMemoTable &table, size_t index) const {
    PIKA_CHECKED_MATCH({

                           return table[key] =
                                          std::make_shared<pika::memotable::Match>(key, 0, 0,
                                                                                   std::vector<std::shared_ptr<pika::memotable::Match>>{});
                       }
    );
}

std::shared_ptr<pika::memotable::Match>
pika::clause::Any::packrat_match(pika::memotable::PackratMemoTable &table, size_t index) const {
    PIKA_CHECKED_MATCH(if (!table.at_end(index)) {
        return table[key] =
                       std::make_shared<pika::memotable::Match>(key, 1, 0,
                                                                std::vector<std::shared_ptr<pika::memotable::Match>>{});
    }
    );
}
