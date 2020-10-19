#ifndef PIKA_CLAUSE_HPP
#define PIKA_CLAUSE_HPP

#include <tuple>
#include <vector>
#include <absl/container/flat_hash_set.h>
#include <typeindex>
#include <queue>
#include <optional>
#include <ostream>

namespace pika {
    namespace clause {
        struct Clause {
            [[nodiscard]] virtual std::optional<std::string_view> label() {
                return std::nullopt;
            }

            [[nodiscard]] virtual std::string_view display() {
                return "Clause";
            }

            virtual void dump_inner(std::ostream& output, absl::flat_hash_set<std::type_index>& visited) {
                if (!label() || visited.contains(typeid(*this))) {
                    return;
                } else {
                    visited.insert(typeid(*this));
                    output << label().value() << " <- " << display() << std::endl;
                }
            }

            virtual void dump(std::ostream& output) {
                auto visited = absl::flat_hash_set<std::type_index>();
                dump_inner(output, visited);
            }
        };

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
#define DISPLAY(BLOCK) \
    [[nodiscard]] std::string_view display() override { \
        BLOCK \
    }

        template<char C>
        struct Char : public _internal::Char {
            constexpr static char CLAUSE_LABEL[] = {'\'', C, '\'', '\0'};

            DISPLAY({
                        return CLAUSE_LABEL;
                    })
        };

        template<char Start, char End>
        struct CharRange : _internal::CharRange {
            constexpr static char CLAUSE_LABEL[] = {'[', '\'', Start, '\'', '-', '\'', End, '\'', ']', '\0'};

            DISPLAY({
                        return CLAUSE_LABEL;
                    })
        };

        struct First : public _internal::Terminal {
            constexpr static char CLAUSE_LABEL[] = "^";

            DISPLAY({
                        return CLAUSE_LABEL;
                    })
        };

        struct Nothing : public _internal::Terminal {
            constexpr static char CLAUSE_LABEL[] = "NOTHING";

            DISPLAY({
                        return CLAUSE_LABEL;
                    })
        };

        struct Any : public _internal::Terminal {
            constexpr static char CLAUSE_LABEL[] = "ANYTHING";

            DISPLAY({
                        return CLAUSE_LABEL;
                    })
        };
#define UNARY_DUMP(H) \
     void dump_inner(std::ostream& output, absl::flat_hash_set<std::type_index>& visited) override { \
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
            virtual void dump_inner_unchecked(std::ostream& output, absl::flat_hash_set<std::type_index>& visited) {
                if (!this->label()) {
                    return;
                } else {
                    H().dump_inner(output, visited);
                    Seq<T...>::dump_inner_unchecked(output, visited);
                }
            }
            void dump_inner(std::ostream& output, absl::flat_hash_set<std::type_index>& visited) override {
                if (!this->label() || visited.contains(typeid(*this))) {
                    return;
                } else {
                    visited.insert(typeid(*this));
                    H().dump_inner(output, visited);
                    Seq<T...>::dump_inner_unchecked(output, visited);
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
        };

        template<typename H>
        struct Seq<H> : public _internal::Seq {
            virtual void dump_inner_unchecked(std::ostream& output, absl::flat_hash_set<std::type_index>& visited) {
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
        };


        template<typename H, typename ...T>
        struct Ord : public Ord<T...> {
            virtual void dump_inner_unchecked(std::ostream& output, absl::flat_hash_set<std::type_index>& visited) {
                if (!this->label()) {
                    return;
                } else {
                    H().dump_inner(output, visited);
                    Ord<T...>::dump_inner_unchecked(output, visited);
                }
            }
            void dump_inner(std::ostream& output, absl::flat_hash_set<std::type_index>& visited) override {
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
        };

        template<typename H>
        struct Ord<H> : public _internal::Ord {
            virtual void dump_inner_unchecked(std::ostream& output, absl::flat_hash_set<std::type_index>& visited) {
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
        };


        template<typename S>
        struct Plus : public _internal::Plus {
            UNARY_DUMP(S);
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
        };


        template<typename S>
        struct Asterisks : public _internal::Asterisks {
            UNARY_DUMP(S);
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
        };

        template<typename S>
        struct Optional : public _internal::Optional {
            UNARY_DUMP(S);
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
        };


        template<typename S>
        struct FollowedBy : public _internal::FollowedBy {
            UNARY_DUMP(S);
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
        };

        template<typename S>
        struct NotFollowedBy : public _internal::NotFollowedBy {
            UNARY_DUMP(S);
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
        };


    }

} // namespace pika::clause
#endif //PIKA_CLAUSE_HPP