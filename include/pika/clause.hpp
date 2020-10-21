#ifndef PIKA_CLAUSE_HPP
#define PIKA_CLAUSE_HPP

#include <tuple>
#include <vector>
#include <absl/container/flat_hash_set.h>
#include <absl/container/flat_hash_map.h>
#include <typeindex>
#include <queue>
#include <optional>
#include <ostream>

namespace pika {
    namespace memotable {
        struct MemoKey;
        struct PackratMemoTable;
        struct Match;
    }
    namespace clause {
        using ClauseTable = absl::flat_hash_map<std::type_index, const struct Clause *>;

        struct Clause {
            [[nodiscard]] virtual std::optional<std::string_view> label() const;

            [[nodiscard]] virtual std::string_view display() const;

            virtual void dump_inner(std::ostream &output, absl::flat_hash_set<std::type_index> &visited) const;

            virtual void dump(std::ostream &output) const;

            virtual void construct_table(ClauseTable &table) const = 0;

            [[nodiscard]] virtual const Clause *get_instance() const = 0;

            [[nodiscard]] virtual bool active() const;;

            [[nodiscard]] virtual std::shared_ptr<pika::memotable::Match>
            packrat_match(pika::memotable::PackratMemoTable &table, size_t index) const;
        };

#define DEFAULT_INSTANCE \
        const Clause* get_instance() const override { \
            static std::decay_t<typeof(*this)> INIT;            \
            return & INIT;                 \
        }

        namespace _internal {
            struct Terminal : public Clause {
            };
            struct NonTerminal : public Clause {
            };
            struct Char : public Terminal {
            };
            struct CharRange : public Terminal {
            };
            struct Seq : public NonTerminal {
            };
            struct Ord : public NonTerminal {
            };
            struct Plus : public NonTerminal {
            };
            struct Asterisks : public NonTerminal {
            };
            struct Optional : public NonTerminal {
            };
            struct FollowedBy : public NonTerminal {
            };
            struct NotFollowedBy : public NonTerminal {
            };
        }

#define CLAUSE_TABLE(TYPE) \
        void construct_table(ClauseTable& table) const override { \
              if (table.contains(typeid(*this))) {                 \
                    return;             \
              }            \
              table.insert({typeid(*this), this->get_instance()});\
        }

#define UNARY_CLAUSE_TABLE(TYPE, SUB) \
        void construct_table(ClauseTable& table) const override {    \
              if (table.contains(typeid(*this))) {                    \
                    return;                                          \
              }                                                      \
              table.insert({typeid(*this),this->get_instance()});\
              SUB().construct_table(table);                                   \
        }


#define DISPLAY(BLOCK) \
    [[nodiscard]] std::string_view display() const override { \
        BLOCK \
    }

        template<char C>
        struct Char : public _internal::Char {
            constexpr static char CLAUSE_LABEL[] = {'\'', C, '\'', '\0'};

            DEFAULT_INSTANCE;

            CLAUSE_TABLE(Char);

            DISPLAY({
                        return CLAUSE_LABEL;
                    })

            std::shared_ptr<pika::memotable::Match>
            packrat_match(pika::memotable::PackratMemoTable &table, size_t index) const override;
        };

        template<char Start, char End>
        struct CharRange : _internal::CharRange {
            DEFAULT_INSTANCE;
            constexpr static char CLAUSE_LABEL[] = {'[', '\'', Start, '\'', '-', '\'', End, '\'', ']', '\0'};

            CLAUSE_TABLE(CharRange);

            DISPLAY({
                        return CLAUSE_LABEL;
                    })

            std::shared_ptr<pika::memotable::Match>
            packrat_match(pika::memotable::PackratMemoTable &table, size_t index) const override;
        };

        struct First : public _internal::Terminal {
            DEFAULT_INSTANCE;
            constexpr static char CLAUSE_LABEL[] = "^";

            CLAUSE_TABLE(First);

            DISPLAY({
                        return CLAUSE_LABEL;
                    })

            std::shared_ptr<pika::memotable::Match>
            packrat_match(pika::memotable::PackratMemoTable &table, size_t index) const override;
        };

        struct Nothing : public _internal::Terminal {
            DEFAULT_INSTANCE;
            constexpr static char CLAUSE_LABEL[] = "NOTHING";

            CLAUSE_TABLE(Nothing);

            DISPLAY({
                        return CLAUSE_LABEL;
                    })

            std::shared_ptr<pika::memotable::Match>
            packrat_match(pika::memotable::PackratMemoTable &table, size_t index) const override;
        };

        struct Any : public _internal::Terminal {
            DEFAULT_INSTANCE;
            constexpr static char CLAUSE_LABEL[] = "ANYCHAR";

            CLAUSE_TABLE(Any);

            DISPLAY({
                        return CLAUSE_LABEL;
                    })

            std::shared_ptr<pika::memotable::Match>
            packrat_match(pika::memotable::PackratMemoTable &table, size_t index) const override;
        };

#define UNARY_DUMP(H) \
     void dump_inner(std::ostream& output, absl::flat_hash_set<std::type_index>& visited) const override { \
        if (!this->label() || visited.contains(typeid(*this))) { \
          return; \
        } else { \
        visited.insert(typeid(*this)); \
        H().dump_inner(output, visited); \
       output << this->label().value() << " <- " << display() << std::endl; \
       } \
    }

        template<typename H, typename ...T>
        struct Seq : public Seq<T...> {
            DEFAULT_INSTANCE;

            virtual void
            dump_inner_unchecked(std::ostream &output, absl::flat_hash_set<std::type_index> &visited) const {
                if (!this->label()) {
                    return;
                } else {
                    H().dump_inner(output, visited);
                    Seq<T...>::dump_inner_unchecked(output, visited);
                }
            }

            void dump_inner(std::ostream &output, absl::flat_hash_set<std::type_index> &visited) const override {
                if (!this->label() || visited.contains(typeid(*this))) {
                    return;
                } else {
                    visited.insert(typeid(*this));
                    H().dump_inner(output, visited);
                    Seq<T...>::dump_inner_unchecked(output, visited);
                    output << this->label().value() << " <- " << display() << std::endl;
                }
            }

            virtual void construct_table_unchecked(ClauseTable &table) const {
                H().construct_table(table);
                Seq<T...>::construct_table_unchecked(table);
            }

            void construct_table(ClauseTable &table) const override {
                if (table.template contains(typeid(*this))) {
                    return;
                }
                table.template insert({typeid(*this), this->get_instance()});
                construct_table_unchecked(table);
            }

            static std::string_view __display() {
                static std::string CLAUSE_LABEL = {};
                static bool INIT = false;
                if (!INIT) {
                    INIT = true;
                    if (auto res = H().label()) {
                        CLAUSE_LABEL.append(res.value().begin(), res.value().end());
                    } else {
                        auto dis = H().display();
                        CLAUSE_LABEL.append(dis.begin(), dis.end());
                    }
                    auto res = Seq<T...>::__display();
                    CLAUSE_LABEL.append(" ~ ");
                    CLAUSE_LABEL.append(res.begin(), res.end());

                }
                return CLAUSE_LABEL;
            }

            DISPLAY({
                        static std::string CLAUSE_LABEL = {};
                        static bool INIT = false;
                        if (!INIT) {
                            INIT = true;
                            auto res = __display();
                            CLAUSE_LABEL.append("( ");
                            CLAUSE_LABEL.append(res.begin(), res.end());
                            CLAUSE_LABEL.append(" )");
                        }
                        return CLAUSE_LABEL;
                    })

            virtual std::shared_ptr<pika::memotable::Match>
            packrat_reduce(pika::memotable::PackratMemoTable &table, size_t index, size_t length,
                           std::vector<std::shared_ptr<pika::memotable::Match>>) const;
            std::shared_ptr<pika::memotable::Match>
            packrat_match(pika::memotable::PackratMemoTable &table, size_t index) const override;
        };



        template<typename H>
        struct Seq<H> : public _internal::Seq {
            DEFAULT_INSTANCE;

            virtual void
            dump_inner_unchecked(std::ostream &output, absl::flat_hash_set<std::type_index> &visited) const {
                if (!this->label()) {
                    return;
                } else {
                    H().dump_inner(output, visited);
                }
            }

            virtual void construct_table_unchecked(ClauseTable &table) const {
                H().construct_table(table);
            }

            UNARY_CLAUSE_TABLE(Seq, H);

            UNARY_DUMP(H);

            static std::string_view __display() {
                static std::string CLAUSE_LABEL = {};
                static bool INIT = false;
                if (!INIT) {
                    INIT = true;
                    if (auto res = H().label()) {
                        CLAUSE_LABEL.append(res.value().begin(), res.value().end());
                    } else {
                        auto dis = H().display();
                        CLAUSE_LABEL.append(dis.begin(), dis.end());
                    }
                }
                return CLAUSE_LABEL;
            }

            DISPLAY({
                        static std::string CLAUSE_LABEL = {};
                        static bool INIT = false;
                        if (!INIT) {
                            INIT = true;
                            auto res = __display();
                            CLAUSE_LABEL.append("( ");
                            CLAUSE_LABEL.append(res.begin(), res.end());
                            CLAUSE_LABEL.append(" )");
                        }
                        return CLAUSE_LABEL;
                    })

            virtual std::shared_ptr<pika::memotable::Match>
            packrat_reduce(pika::memotable::PackratMemoTable &table, size_t index, size_t length,
                           std::vector<std::shared_ptr<pika::memotable::Match>>) const;
            std::shared_ptr<pika::memotable::Match>
            packrat_match(pika::memotable::PackratMemoTable &table, size_t index) const override;
        };


        template<typename H, typename ...T>
        struct Ord : public Ord<T...> {
            DEFAULT_INSTANCE;

            virtual void construct_table_unchecked(ClauseTable &table) const {
                H().construct_table(table);
                Ord<T...>::construct_table_unchecked(table);
            }

            void construct_table(ClauseTable &table) const override {
                if (table.template contains(typeid(*this))) {
                    return;
                }
                table.template insert({typeid(*this), this->get_instance()});
                construct_table_unchecked(table);
            }

            virtual void
            dump_inner_unchecked(std::ostream &output, absl::flat_hash_set<std::type_index> &visited) const {
                if (!this->label()) {
                    return;
                } else {
                    H().dump_inner(output, visited);
                    Ord<T...>::dump_inner_unchecked(output, visited);
                }
            }

            void dump_inner(std::ostream &output, absl::flat_hash_set<std::type_index> &visited) const override {
                if (!this->label() || visited.contains(typeid(*this))) {
                    return;
                } else {
                    visited.insert(typeid(*this));
                    H().dump_inner(output, visited);
                    Ord<T...>::dump_inner_unchecked(output, visited);
                    output << this->label().value() << " <- " << display() << std::endl;
                }
            }

            static std::string_view __display() {
                static std::string CLAUSE_LABEL = {};
                static bool INIT = false;
                if (!INIT) {
                    INIT = true;
                    if (auto res = H().label()) {
                        CLAUSE_LABEL.append(res.value().begin(), res.value().end());
                    } else {
                        auto dis = H().display();
                        CLAUSE_LABEL.append(dis.begin(), dis.end());
                    }
                    auto res = Ord<T...>::__display();
                    CLAUSE_LABEL.append(" / ");
                    CLAUSE_LABEL.append(res.begin(), res.end());

                }
                return CLAUSE_LABEL;
            }

            DISPLAY({
                        static std::string CLAUSE_LABEL = {};
                        static bool INIT = false;
                        if (!INIT) {
                            INIT = true;
                            auto res = __display();
                            CLAUSE_LABEL.append("( ");
                            CLAUSE_LABEL.append(res.begin(), res.end());
                            CLAUSE_LABEL.append(" )");
                        }
                        return CLAUSE_LABEL;
                    })

            std::shared_ptr<pika::memotable::Match>
            packrat_match(pika::memotable::PackratMemoTable &table, size_t index) const override;
        };

        template<typename H>
        struct Ord<H> : public _internal::Ord {
            DEFAULT_INSTANCE;

            virtual void construct_table_unchecked(ClauseTable &table) const {
                H().construct_table(table);
            }

            UNARY_CLAUSE_TABLE(Ord, H);

            virtual void
            dump_inner_unchecked(std::ostream &output, absl::flat_hash_set<std::type_index> &visited) const {
                if (!this->label()) {
                    return;
                } else {
                    H().dump_inner(output, visited);
                }
            }

            UNARY_DUMP(H);

            static std::string_view __display() {
                static std::string CLAUSE_LABEL = {};
                static bool INIT = false;
                if (!INIT) {
                    INIT = true;
                    if (auto res = H().label()) {
                        CLAUSE_LABEL.append(res.value().begin(), res.value().end());
                    } else {
                        auto dis = H().display();
                        CLAUSE_LABEL.append(dis.begin(), dis.end());
                    }
                }
                return CLAUSE_LABEL;
            }

            DISPLAY({
                        static std::string CLAUSE_LABEL = {};
                        static bool INIT = false;
                        if (!INIT) {
                            INIT = true;
                            auto res = __display();
                            CLAUSE_LABEL.append("( ");
                            CLAUSE_LABEL.append(res.begin(), res.end());
                            CLAUSE_LABEL.append(" )");
                        }
                        return CLAUSE_LABEL;
                    })

            std::shared_ptr<pika::memotable::Match>
            packrat_match(pika::memotable::PackratMemoTable &table, size_t index) const override;
        };


        template<typename S>
        struct Plus : public _internal::Plus {
            UNARY_DUMP(S);

            UNARY_CLAUSE_TABLE(Plus, S);

            DEFAULT_INSTANCE;

            DISPLAY({
                        static std::string CLAUSE_LABEL = {};
                        static bool INIT = false;
                        if (!INIT) {
                            INIT = true;
                            CLAUSE_LABEL.append("( ");
                            if (auto res = S().label()) {
                                CLAUSE_LABEL.append(res.value().begin(), res.value().end());
                            } else {
                                auto dis = S().display();
                                CLAUSE_LABEL.append(dis.begin(), dis.end());
                            }
                            CLAUSE_LABEL.append(" )+");
                        }
                        return CLAUSE_LABEL;
                    })

            std::shared_ptr<pika::memotable::Match>
            packrat_match(pika::memotable::PackratMemoTable &table, size_t index) const override;
        };


        template<typename S>
        struct Asterisks : public _internal::Asterisks {
            UNARY_DUMP(S);

            UNARY_CLAUSE_TABLE(Asterisks, S);

            DEFAULT_INSTANCE;

            DISPLAY({
                        static std::string CLAUSE_LABEL = {};
                        static bool INIT = false;
                        if (!INIT) {
                            INIT = true;
                            CLAUSE_LABEL.append("( ");
                            if (auto res = S().label()) {
                                CLAUSE_LABEL.append(res.value().begin(), res.value().end());
                            } else {
                                auto dis = S().display();
                                CLAUSE_LABEL.append(dis.begin(), dis.end());
                            }
                            CLAUSE_LABEL.append(" )*");
                        }
                        return CLAUSE_LABEL;
                    })

            std::shared_ptr<pika::memotable::Match>
            packrat_match(pika::memotable::PackratMemoTable &table, size_t index) const override;
        };

        template<typename S>
        struct Optional : public _internal::Optional {
            UNARY_DUMP(S);

            UNARY_CLAUSE_TABLE(Optional, S);

            DEFAULT_INSTANCE;

            DISPLAY({
                        static std::string CLAUSE_LABEL = {};
                        static bool INIT = false;
                        if (!INIT) {
                            INIT = true;
                            CLAUSE_LABEL.append("( ");
                            if (auto res = S().label()) {
                                CLAUSE_LABEL.append(res.value().begin(), res.value().end());
                            } else {
                                auto dis = S().display();
                                CLAUSE_LABEL.append(dis.begin(), dis.end());
                            }
                            CLAUSE_LABEL.append(" )?");
                        }
                        return CLAUSE_LABEL;
                    })

            std::shared_ptr<pika::memotable::Match>
            packrat_match(pika::memotable::PackratMemoTable &table, size_t index) const override;
        };


        template<typename S>
        struct FollowedBy : public _internal::FollowedBy {
            UNARY_DUMP(S);

            UNARY_CLAUSE_TABLE(FollowedBy, S);

            DEFAULT_INSTANCE;

            DISPLAY({
                        static std::string CLAUSE_LABEL = {};
                        static bool INIT = false;
                        if (!INIT) {
                            INIT = true;

                            CLAUSE_LABEL.append("&( ");
                            if (auto res = S().label()) {
                                CLAUSE_LABEL.append(res.value().begin(), res.value().end());
                            } else {
                                auto dis = S().display();
                                CLAUSE_LABEL.append(dis.begin(), dis.end());
                            }
                            CLAUSE_LABEL.append(" )");

                        }
                        return CLAUSE_LABEL;
                    })

            std::shared_ptr<pika::memotable::Match>
            packrat_match(pika::memotable::PackratMemoTable &table, size_t index) const override;
        };

        template<typename S>
        struct NotFollowedBy : public _internal::NotFollowedBy {
            UNARY_DUMP(S);

            UNARY_CLAUSE_TABLE(NotFollowedBy, S);

            DEFAULT_INSTANCE;

            DISPLAY({
                        static std::string CLAUSE_LABEL = {};
                        static bool INIT = false;
                        if (!INIT) {
                            INIT = true;
                            CLAUSE_LABEL.append("!( ");
                            if (auto res = S().label()) {
                                CLAUSE_LABEL.append(res.value().begin(), res.value().end());
                            } else {
                                auto dis = S().display();
                                CLAUSE_LABEL.append(dis.begin(), dis.end());
                            }
                            CLAUSE_LABEL.append(" )");
                        }
                        return CLAUSE_LABEL;
                    })

            std::shared_ptr<pika::memotable::Match>
            packrat_match(pika::memotable::PackratMemoTable &table, size_t index) const override;
        };

    }

} // namespace pika::clause


#define PIKA_DECLARE(TYPE_NAME, RULE, ACTIVE) \
struct TYPE_NAME : RULE {                   \
    DEFAULT_INSTANCE;                                            \
    [[nodiscard]] std::optional<std::string_view> label() const override { \
        return #TYPE_NAME; \
    }                                           \
    [[nodiscard]] bool active() const override { \
        return ACTIVE;                           \
    };                                          \
}

#define PIKA_ANY pika::clause::Any
#define PIKA_NOTHING pika::clause::Nothing
#define PIKA_FIRST pika::clause::First
#define PIKA_CHAR(C) pika::clause::Char<C>
#define PIKA_CHAR_RANGE(A, B) pika::clause::CharRange<A, B>
#define PIKA_SEQ(...) pika::clause::Seq<__VA_ARGS__>
#define PIKA_ORD(...) pika::clause::Ord<__VA_ARGS__>
#define PIKA_PLUS(C) pika::clause::Plus<C>
#define PIKA_ASTERISKS(C) pika::clause::Asterisks<C>
#define PIKA_FOLLOWED_BY(C) pika::clause::FollowedBy<C>
#define PIKA_NOT_FOLLOWED_BY(C) pika::clause::NotFollowedBy<C>
#include <iostream>
#define PIKA_CHECKED_MATCH(BLOCK) \
    std::cout << "parsing: " << display() << std::endl;                              \
    auto key = pika::memotable::MemoKey(this->get_instance(), index); \
    if (table.template contains(key)) { \
        return table[key]; \
    } else {\
        BLOCK                \
    }                        \
    return table[key] = nullptr
#endif //PIKA_CLAUSE_HPP