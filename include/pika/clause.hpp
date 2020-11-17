#ifndef PIKA_CLAUSE_HPP
#define PIKA_CLAUSE_HPP

#include <absl/container/flat_hash_map.h>
#include <absl/container/flat_hash_set.h>
#include <optional>
#include <ostream>
#include <queue>
#include <tuple>
#include <typeindex>
#include <vector>

namespace pika
{
    namespace memotable
    {
        struct MemoKey;
        struct MemoTable;
        struct Match;
    }
    namespace type_utils
    {
        enum class BaseType;
    }
    namespace graph
    {
        struct ClauseTable;
    }
    namespace clause
    {
        struct Clause
        {
            [[nodiscard]] virtual std::optional<std::string_view> label() const;

            [[nodiscard]] virtual std::string_view display() const;

            virtual void dump_inner(
                std::ostream& output,
                absl::flat_hash_set<std::type_index>& visited) const;

            virtual void dump(std::ostream& output) const;

            [[nodiscard]] virtual const Clause* get_instance() const = 0;

            [[nodiscard]] virtual bool active() const;
            ;

            [[nodiscard]] virtual std::shared_ptr<pika::memotable::Match>
            packrat_match(
                pika::memotable::MemoTable& table, size_t index) const;

            [[nodiscard]] virtual pika::type_utils::BaseType
            get_base_type() const noexcept;

            virtual void dfs_traversal(
                absl::flat_hash_set<std::type_index>& visited,
                std::vector<const Clause*>& terminals,
                std::vector<const Clause*>& nodes) const = 0;
            virtual void mark_seeds(graph::ClauseTable& table) const;
            virtual void pika_match(graph::ClauseTable& table) const = 0;
        };

#define PIKA_DEFAULT_INSTANCE \
    const Clause* get_instance() const override \
    { \
        static std::decay_t<typeof(*this)> INIT; \
        return &INIT; \
    }

#define PIKA_DFS_CHECK(BLOCK) \
    if (visited.template contains(typeid(*this))) \
    { \
        return; \
    } \
    { \
        visited.insert(typeid(*this)); \
        BLOCK \
    }

#define PIKA_DFS_UNARY(T) \
    PIKA_DFS_CHECK({ \
        T().dfs_traversal(visited, terminals, nodes); \
        nodes.push_back(this->get_instance()); \
    })

        namespace _internal
        {
            struct Terminal : public Clause
            {
                void dfs_traversal(
                    absl::flat_hash_set<std::type_index>& visited,
                    std::vector<const Clause*>& terminals,
                    std::vector<const Clause*>& nodes) const override;
            };

            struct NonTerminal : public Clause
            {};

            struct Char : public Terminal
            {
                [[nodiscard]] pika::type_utils::BaseType
                get_base_type() const noexcept override;
            };

            struct CharRange : public Terminal
            {
                [[nodiscard]] pika::type_utils::BaseType
                get_base_type() const noexcept override;
            };

            struct Seq : public NonTerminal
            {
                [[nodiscard]] pika::type_utils::BaseType
                get_base_type() const noexcept override;
            };

            struct Ord : public NonTerminal
            {
                [[nodiscard]] pika::type_utils::BaseType
                get_base_type() const noexcept override;
            };

            struct Plus : public NonTerminal
            {
                [[nodiscard]] pika::type_utils::BaseType
                get_base_type() const noexcept override;
            };

            struct Asterisks : public NonTerminal
            {
                [[nodiscard]] pika::type_utils::BaseType
                get_base_type() const noexcept override;
            };

            struct Optional : public NonTerminal
            {
                [[nodiscard]] pika::type_utils::BaseType
                get_base_type() const noexcept override;
            };

            struct FollowedBy : public NonTerminal
            {
                [[nodiscard]] pika::type_utils::BaseType
                get_base_type() const noexcept override;
            };

            struct NotFollowedBy : public NonTerminal
            {
                [[nodiscard]] pika::type_utils::BaseType
                get_base_type() const noexcept override;
            };
        }

#define DISPLAY(BLOCK) \
    [[nodiscard]] std::string_view display() const override \
    { \
        BLOCK \
    }

        template<char C>
        struct Char : public _internal::Char
        {
            constexpr static char CLAUSE_LABEL[] = {'\'', C, '\'', '\0'};

            PIKA_DEFAULT_INSTANCE;

            DISPLAY({ return CLAUSE_LABEL; })

            std::shared_ptr<pika::memotable::Match> packrat_match(
                pika::memotable::MemoTable& table, size_t index) const override;
            void pika_match(graph::ClauseTable& table) const override;
        };

        template<char Start, char End>
        struct CharRange : _internal::CharRange
        {
            PIKA_DEFAULT_INSTANCE;
            constexpr static char CLAUSE_LABEL[] = {
                '[', '\'', Start, '\'', '-', '\'', End, '\'', ']', '\0'};

            DISPLAY({ return CLAUSE_LABEL; })

            std::shared_ptr<pika::memotable::Match> packrat_match(
                pika::memotable::MemoTable& table, size_t index) const override;
            void pika_match(pika::graph::ClauseTable& table) const override;
        };

        struct First : public _internal::Terminal
        {
            PIKA_DEFAULT_INSTANCE;
            constexpr static char CLAUSE_LABEL[] = "^";

            DISPLAY({ return CLAUSE_LABEL; })

            std::shared_ptr<pika::memotable::Match> packrat_match(
                pika::memotable::MemoTable& table, size_t index) const override;

            [[nodiscard]] pika::type_utils::BaseType
            get_base_type() const noexcept override;
            void pika_match(pika::graph::ClauseTable& table) const override;
        };

        struct Nothing : public _internal::Terminal
        {
            PIKA_DEFAULT_INSTANCE;
            constexpr static char CLAUSE_LABEL[] = "NOTHING";

            DISPLAY({ return CLAUSE_LABEL; })

            std::shared_ptr<pika::memotable::Match> packrat_match(
                pika::memotable::MemoTable& table, size_t index) const override;

            [[nodiscard]] pika::type_utils::BaseType
            get_base_type() const noexcept override;
            void pika_match(pika::graph::ClauseTable& table) const override;
        };

        struct Any : public _internal::Terminal
        {
            PIKA_DEFAULT_INSTANCE;
            constexpr static char CLAUSE_LABEL[] = "ANYCHAR";

            DISPLAY({ return CLAUSE_LABEL; })

            std::shared_ptr<pika::memotable::Match> packrat_match(
                pika::memotable::MemoTable& table, size_t index) const override;

            [[nodiscard]] pika::type_utils::BaseType
            get_base_type() const noexcept override;
            void pika_match(pika::graph::ClauseTable& table) const override;
        };

#define UNARY_DUMP(H) \
    void dump_inner( \
        std::ostream& output, absl::flat_hash_set<std::type_index>& visited) \
        const override \
    { \
        if (!this->label() || visited.contains(typeid(*this))) \
        { \
            return; \
        } \
        else \
        { \
            visited.insert(typeid(*this)); \
            H().dump_inner(output, visited); \
            output << this->label().value() << " <- " << display() \
                   << std::endl; \
        } \
    }

        template<typename H, typename... T>
        struct Seq : public Seq<T...>
        {
            PIKA_DEFAULT_INSTANCE;

            virtual void dump_inner_unchecked(
                std::ostream& output,
                absl::flat_hash_set<std::type_index>& visited) const
            {
                if (!this->label())
                {
                    return;
                }
                else
                {
                    H().dump_inner(output, visited);
                    Seq<T...>::dump_inner_unchecked(output, visited);
                }
            }

            void dump_inner(
                std::ostream& output,
                absl::flat_hash_set<std::type_index>& visited) const override
            {
                if (!this->label() || visited.contains(typeid(*this)))
                {
                    return;
                }
                else
                {
                    visited.insert(typeid(*this));
                    H().dump_inner(output, visited);
                    Seq<T...>::dump_inner_unchecked(output, visited);
                    output << this->label().value() << " <- " << display()
                           << std::endl;
                }
            }

            void dfs_traversal(
                absl::flat_hash_set<std::type_index>& visited,
                std::vector<const Clause*>& terminals,
                std::vector<const Clause*>& nodes) const override;

            virtual void _dfs_traversal(
                absl::flat_hash_set<std::type_index>& visited,
                std::vector<const Clause*>& terminals,
                std::vector<const Clause*>& nodes) const;

            static std::string_view __display()
            {
                static std::string CLAUSE_LABEL = {};
                static bool INIT = false;
                if (!INIT)
                {
                    INIT = true;
                    if (auto res = H().label())
                    {
                        CLAUSE_LABEL.append(
                            res.value().begin(), res.value().end());
                    }
                    else
                    {
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
                if (!INIT)
                {
                    INIT = true;
                    auto res = __display();
                    CLAUSE_LABEL.append("( ");
                    CLAUSE_LABEL.append(res.begin(), res.end());
                    CLAUSE_LABEL.append(" )");
                }
                return CLAUSE_LABEL;
            })

            virtual std::shared_ptr<pika::memotable::Match> packrat_reduce(
                pika::memotable::MemoTable& table,
                size_t index,
                size_t length,
                std::vector<std::shared_ptr<pika::memotable::Match>>) const;

            std::shared_ptr<pika::memotable::Match> packrat_match(
                pika::memotable::MemoTable& table, size_t index) const override;

            void mark_seeds(graph::ClauseTable& table) const override;
            void pika_match(pika::graph::ClauseTable& table) const override;
            virtual void pika_match_unchecked(
                pika::graph::ClauseTable& table,
                size_t length,
                std::vector<std::shared_ptr<memotable::Match>>) const;
        };

        template<typename H>
        struct Seq<H> : public _internal::Seq
        {
            PIKA_DEFAULT_INSTANCE;

            virtual void dump_inner_unchecked(
                std::ostream& output,
                absl::flat_hash_set<std::type_index>& visited) const
            {
                if (!this->label())
                {
                    return;
                }
                else
                {
                    H().dump_inner(output, visited);
                }
            }

            UNARY_DUMP(H);

            static std::string_view __display()
            {
                static std::string CLAUSE_LABEL = {};
                static bool INIT = false;
                if (!INIT)
                {
                    INIT = true;
                    if (auto res = H().label())
                    {
                        CLAUSE_LABEL.append(
                            res.value().begin(), res.value().end());
                    }
                    else
                    {
                        auto dis = H().display();
                        CLAUSE_LABEL.append(dis.begin(), dis.end());
                    }
                }
                return CLAUSE_LABEL;
            }

            DISPLAY({
                static std::string CLAUSE_LABEL = {};
                static bool INIT = false;
                if (!INIT)
                {
                    INIT = true;
                    auto res = __display();
                    CLAUSE_LABEL.append("( ");
                    CLAUSE_LABEL.append(res.begin(), res.end());
                    CLAUSE_LABEL.append(" )");
                }
                return CLAUSE_LABEL;
            })

            virtual std::shared_ptr<pika::memotable::Match> packrat_reduce(
                pika::memotable::MemoTable& table,
                size_t index,
                size_t length,
                std::vector<std::shared_ptr<pika::memotable::Match>>) const;

            std::shared_ptr<pika::memotable::Match> packrat_match(
                pika::memotable::MemoTable& table, size_t index) const override;

            void dfs_traversal(
                absl::flat_hash_set<std::type_index>& visited,
                std::vector<const Clause*>& terminals,
                std::vector<const Clause*>& nodes) const override;

            virtual void _dfs_traversal(
                absl::flat_hash_set<std::type_index>& visited,
                std::vector<const Clause*>& terminals,
                std::vector<const Clause*>& nodes) const;
            void mark_seeds(pika::graph::ClauseTable& table) const override;
            void pika_match(pika::graph::ClauseTable& table) const override;
            virtual void pika_match_unchecked(
                pika::graph::ClauseTable& table,
                size_t length,
                std::vector<std::shared_ptr<memotable::Match>>) const;
        };

        template<typename H, typename... T>
        struct Ord : public Ord<T...>
        {
            PIKA_DEFAULT_INSTANCE;

            virtual void dump_inner_unchecked(
                std::ostream& output,
                absl::flat_hash_set<std::type_index>& visited) const
            {
                if (!this->label())
                {
                    return;
                }
                else
                {
                    H().dump_inner(output, visited);
                    Ord<T...>::dump_inner_unchecked(output, visited);
                }
            }

            void dump_inner(
                std::ostream& output,
                absl::flat_hash_set<std::type_index>& visited) const override
            {
                if (!this->label() || visited.contains(typeid(*this)))
                {
                    return;
                }
                else
                {
                    visited.insert(typeid(*this));
                    H().dump_inner(output, visited);
                    Ord<T...>::dump_inner_unchecked(output, visited);
                    output << this->label().value() << " <- " << display()
                           << std::endl;
                }
            }

            static std::string_view __display()
            {
                static std::string CLAUSE_LABEL = {};
                static bool INIT = false;
                if (!INIT)
                {
                    INIT = true;
                    if (auto res = H().label())
                    {
                        CLAUSE_LABEL.append(
                            res.value().begin(), res.value().end());
                    }
                    else
                    {
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
                if (!INIT)
                {
                    INIT = true;
                    auto res = __display();
                    CLAUSE_LABEL.append("( ");
                    CLAUSE_LABEL.append(res.begin(), res.end());
                    CLAUSE_LABEL.append(" )");
                }
                return CLAUSE_LABEL;
            })

            std::shared_ptr<pika::memotable::Match> packrat_match(
                pika::memotable::MemoTable& table, size_t index) const override;
            void dfs_traversal(
                absl::flat_hash_set<std::type_index>& visited,
                std::vector<const Clause*>& terminals,
                std::vector<const Clause*>& nodes) const override;

            virtual void _dfs_traversal(
                absl::flat_hash_set<std::type_index>& visited,
                std::vector<const Clause*>& terminals,
                std::vector<const Clause*>& nodes) const;
            void mark_seeds(pika::graph::ClauseTable& table) const override;
            void pika_match(pika::graph::ClauseTable& table) const override;
            virtual void pika_match_unchecked(
                pika::graph::ClauseTable& table, size_t order) const;
        };

        template<typename H>
        struct Ord<H> : public _internal::Ord
        {
            PIKA_DEFAULT_INSTANCE;

            virtual void dump_inner_unchecked(
                std::ostream& output,
                absl::flat_hash_set<std::type_index>& visited) const
            {
                if (!this->label())
                {
                    return;
                }
                else
                {
                    H().dump_inner(output, visited);
                }
            }

            UNARY_DUMP(H);

            static std::string_view __display()
            {
                static std::string CLAUSE_LABEL = {};
                static bool INIT = false;
                if (!INIT)
                {
                    INIT = true;
                    if (auto res = H().label())
                    {
                        CLAUSE_LABEL.append(
                            res.value().begin(), res.value().end());
                    }
                    else
                    {
                        auto dis = H().display();
                        CLAUSE_LABEL.append(dis.begin(), dis.end());
                    }
                }
                return CLAUSE_LABEL;
            }

            DISPLAY({
                static std::string CLAUSE_LABEL = {};
                static bool INIT = false;
                if (!INIT)
                {
                    INIT = true;
                    auto res = __display();
                    CLAUSE_LABEL.append("( ");
                    CLAUSE_LABEL.append(res.begin(), res.end());
                    CLAUSE_LABEL.append(" )");
                }
                return CLAUSE_LABEL;
            })

            std::shared_ptr<pika::memotable::Match> packrat_match(
                pika::memotable::MemoTable& table, size_t index) const override;

            void dfs_traversal(
                absl::flat_hash_set<std::type_index>& visited,
                std::vector<const Clause*>& terminals,
                std::vector<const Clause*>& nodes) const override;

            virtual void _dfs_traversal(
                absl::flat_hash_set<std::type_index>& visited,
                std::vector<const Clause*>& terminals,
                std::vector<const Clause*>& nodes) const;
            void mark_seeds(pika::graph::ClauseTable& table) const override;
            void pika_match(pika::graph::ClauseTable& table) const override;
            virtual void pika_match_unchecked(
                pika::graph::ClauseTable& table, size_t order) const;
        };

        template<typename S>
        struct Plus : public _internal::Plus
        {
            UNARY_DUMP(S);

            PIKA_DEFAULT_INSTANCE;

            DISPLAY({
                static std::string CLAUSE_LABEL = {};
                static bool INIT = false;
                if (!INIT)
                {
                    INIT = true;
                    CLAUSE_LABEL.append("( ");
                    if (auto res = S().label())
                    {
                        CLAUSE_LABEL.append(
                            res.value().begin(), res.value().end());
                    }
                    else
                    {
                        auto dis = S().display();
                        CLAUSE_LABEL.append(dis.begin(), dis.end());
                    }
                    CLAUSE_LABEL.append(" )+");
                }
                return CLAUSE_LABEL;
            })

            std::shared_ptr<pika::memotable::Match> packrat_match(
                pika::memotable::MemoTable& table, size_t index) const override;
            void dfs_traversal(
                absl::flat_hash_set<std::type_index>& visited,
                std::vector<const Clause*>& terminals,
                std::vector<const Clause*>& nodes) const override;
            void mark_seeds(pika::graph::ClauseTable& table) const override;
            void pika_match(pika::graph::ClauseTable& table) const override;
        };

        template<typename S>
        struct Asterisks : public _internal::Asterisks
        {
            UNARY_DUMP(S);

            PIKA_DEFAULT_INSTANCE;

            DISPLAY({
                static std::string CLAUSE_LABEL = {};
                static bool INIT = false;
                if (!INIT)
                {
                    INIT = true;
                    CLAUSE_LABEL.append("( ");
                    if (auto res = S().label())
                    {
                        CLAUSE_LABEL.append(
                            res.value().begin(), res.value().end());
                    }
                    else
                    {
                        auto dis = S().display();
                        CLAUSE_LABEL.append(dis.begin(), dis.end());
                    }
                    CLAUSE_LABEL.append(" )*");
                }
                return CLAUSE_LABEL;
            })

            std::shared_ptr<pika::memotable::Match> packrat_match(
                pika::memotable::MemoTable& table, size_t index) const override;
            void dfs_traversal(
                absl::flat_hash_set<std::type_index>& visited,
                std::vector<const Clause*>& terminals,
                std::vector<const Clause*>& nodes) const override;
            void mark_seeds(pika::graph::ClauseTable& table) const override;
            void pika_match(pika::graph::ClauseTable& table) const override;
        };

        template<typename S>
        struct Optional : public _internal::Optional
        {
            UNARY_DUMP(S);

            PIKA_DEFAULT_INSTANCE;

            DISPLAY({
                static std::string CLAUSE_LABEL = {};
                static bool INIT = false;
                if (!INIT)
                {
                    INIT = true;
                    CLAUSE_LABEL.append("( ");
                    if (auto res = S().label())
                    {
                        CLAUSE_LABEL.append(
                            res.value().begin(), res.value().end());
                    }
                    else
                    {
                        auto dis = S().display();
                        CLAUSE_LABEL.append(dis.begin(), dis.end());
                    }
                    CLAUSE_LABEL.append(" )?");
                }
                return CLAUSE_LABEL;
            })

            std::shared_ptr<pika::memotable::Match> packrat_match(
                pika::memotable::MemoTable& table, size_t index) const override;
            void dfs_traversal(
                absl::flat_hash_set<std::type_index>& visited,
                std::vector<const Clause*>& terminals,
                std::vector<const Clause*>& nodes) const override;
            void mark_seeds(pika::graph::ClauseTable& table) const override;
            void pika_match(pika::graph::ClauseTable& table) const override;
        };

        template<typename S>
        struct FollowedBy : public _internal::FollowedBy
        {
            UNARY_DUMP(S);

            PIKA_DEFAULT_INSTANCE;

            DISPLAY({
                static std::string CLAUSE_LABEL = {};
                static bool INIT = false;
                if (!INIT)
                {
                    INIT = true;

                    CLAUSE_LABEL.append("&( ");
                    if (auto res = S().label())
                    {
                        CLAUSE_LABEL.append(
                            res.value().begin(), res.value().end());
                    }
                    else
                    {
                        auto dis = S().display();
                        CLAUSE_LABEL.append(dis.begin(), dis.end());
                    }
                    CLAUSE_LABEL.append(" )");
                }
                return CLAUSE_LABEL;
            })

            std::shared_ptr<pika::memotable::Match> packrat_match(
                pika::memotable::MemoTable& table, size_t index) const override;
            void dfs_traversal(
                absl::flat_hash_set<std::type_index>& visited,
                std::vector<const Clause*>& terminals,
                std::vector<const Clause*>& nodes) const override;
            void mark_seeds(pika::graph::ClauseTable& table) const override;
            void pika_match(pika::graph::ClauseTable& table) const override;
        };

        template<typename S>
        struct NotFollowedBy : public _internal::NotFollowedBy
        {
            UNARY_DUMP(S);

            PIKA_DEFAULT_INSTANCE;

            DISPLAY({
                static std::string CLAUSE_LABEL = {};
                static bool INIT = false;
                if (!INIT)
                {
                    INIT = true;
                    CLAUSE_LABEL.append("!( ");
                    if (auto res = S().label())
                    {
                        CLAUSE_LABEL.append(
                            res.value().begin(), res.value().end());
                    }
                    else
                    {
                        auto dis = S().display();
                        CLAUSE_LABEL.append(dis.begin(), dis.end());
                    }
                    CLAUSE_LABEL.append(" )");
                }
                return CLAUSE_LABEL;
            })

            std::shared_ptr<pika::memotable::Match> packrat_match(
                pika::memotable::MemoTable& table, size_t index) const override;
            void dfs_traversal(
                absl::flat_hash_set<std::type_index>& visited,
                std::vector<const Clause*>& terminals,
                std::vector<const Clause*>& nodes) const override;
            void mark_seeds(pika::graph::ClauseTable& table) const override;
            void pika_match(pika::graph::ClauseTable& table) const override;
        };

    }

} // namespace pika::clause

#define PIKA_DECLARE(TYPE_NAME, RULE, ACTIVE) \
    struct TYPE_NAME : RULE \
    { \
        PIKA_DEFAULT_INSTANCE; \
        [[nodiscard]] std::optional<std::string_view> label() const override \
        { \
            return #TYPE_NAME; \
        } \
        [[nodiscard]] bool active() const override \
        { \
            return ACTIVE; \
        }; \
    }

#define PIKA_ANY pika::clause::Any
#define PIKA_NOTHING pika::clause::Nothing
#define PIKA_FIRST pika::clause::First
#define PIKA_CHAR(C) pika::clause::Char<C>
#define PIKA_CHAR_RANGE(A, B) pika::clause::CharRange<A, B>
#define PIKA_SEQ(...) pika::clause::Seq<__VA_ARGS__>
#define PIKA_ORD(...) pika::clause::Ord<__VA_ARGS__>
#define PIKA_PLUS(C) pika::clause::Plus<C>
#define PIKA_OPTIONAL(C) pika::clause::Optional<C>
#define PIKA_ASTERISKS(C) pika::clause::Asterisks<C>
#define PIKA_FOLLOWED_BY(C) pika::clause::FollowedBy<C>
#define PIKA_NOT_FOLLOWED_BY(C) pika::clause::NotFollowedBy<C>
#ifdef PACKRAT_DEBUG
#    include <iostream>
#    define PACKRAT_DEBUG \
        std::cout << "parsing: " \
                  << (this->label() ? this->label().value() : this->display()) \
                  << ", from: " << index << std::endl;
#else
#    define PACKRAT_DEBUG
#endif
#define PIKA_CHECKED_MATCH(BLOCK) \
    PACKRAT_DEBUG \
    auto key = pika::memotable::MemoKey(this->get_instance(), index); \
    if (table.template contains(key)) \
    { \
        return table[key]; \
    } \
    { \
        BLOCK \
    } \
    return table[key] = nullptr
#endif // PIKA_CLAUSE_HPP