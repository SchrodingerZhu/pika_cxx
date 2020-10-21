#ifndef PIKA_CLAUSE_IPP
#define PIKA_CLAUSE_IPP

#include <pika/clause.hpp>
#include <pika/memotable.hpp>
#include <typeindex>


template<char C>
std::shared_ptr<pika::memotable::Match>
Char<C>::packrat_match(memotable::PackratMemoTable &table, size_t index) const {
    PIKA_CHECKED_MATCH({
                           if (!table.at_end(index) && table.get_char(index) == C) {
                               return table[key] =
                                              std::make_shared<pika::memotable::Match>(key, 1, 0,
                                                                                       std::vector<std::shared_ptr<pika::memotable::Match>>{});
                           }
                       });
}

template<char Start, char End>
std::shared_ptr<pika::memotable::Match>
CharRange<Start, End>::packrat_match(memotable::PackratMemoTable &table, size_t index) const {
    PIKA_CHECKED_MATCH({
                           if (!table.at_end(index) && table.get_char(index) >= Start && table.get_char(index) <= End) {
                               return table[key] =
                                              std::make_shared<pika::memotable::Match>(key, 1, 0,
                                                                                       std::vector<std::shared_ptr<pika::memotable::Match>>{});
                           }
                       });
}

template<typename S>
std::shared_ptr<pika::memotable::Match>
NotFollowedBy<S>::packrat_match(memotable::PackratMemoTable &table, size_t index) const {
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
FollowedBy<S>::packrat_match(memotable::PackratMemoTable &table, size_t index) const {
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
Optional<S>::packrat_match(memotable::PackratMemoTable &table, size_t index) const {
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
Asterisks<S>::packrat_match(memotable::PackratMemoTable &table, size_t index) const {
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
Plus<S>::packrat_match(memotable::PackratMemoTable &table, size_t index) const {
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
std::shared_ptr<pika::memotable::Match>
Ord<S>::packrat_match(memotable::PackratMemoTable &table, size_t index) const {
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
Ord<H, T...>::packrat_match(memotable::PackratMemoTable &table, size_t index) const {
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
Seq<H, T...>::packrat_reduce(memotable::PackratMemoTable &table, size_t index, size_t length,
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
Seq<H>::packrat_reduce(memotable::PackratMemoTable &table, size_t index, size_t length,
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
Seq<S>::packrat_match(memotable::PackratMemoTable &table, size_t index) const {
    PIKA_CHECKED_MATCH(
            if (auto res = packrat_reduce(table, index, 0, {})) {
                return res;
            }
    );
}

template<typename H, typename ...T>
std::shared_ptr<pika::memotable::Match>
Seq<H, T...>::packrat_match(memotable::PackratMemoTable &table, size_t index) const {
    PIKA_CHECKED_MATCH(
            if (auto res = packrat_reduce(table, index, 0, {})) {
                return res;
            }
    );
}

#endif // PIKA_CLAUSE_IPP
