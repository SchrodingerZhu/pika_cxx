#ifndef PIKA_CLAUSE_HPP
#define PIKA_CLAUSE_HPP

#include <tuple>

namespace pika {

    namespace clause {

        struct Clause {
            virtual ~Clause() = default;
        };

        namespace _internal {
            struct Terminal : public Clause {

            };
            struct NonTerminal : public Clause {

            };

            struct Char : public Terminal {};
            struct CharRange : public Terminal {};
            struct Seq : public NonTerminal {};
            struct Ord : public NonTerminal {};
            struct Plus : public NonTerminal {};
            struct OneOrMore : public NonTerminal {};
            struct FollowedBy : public NonTerminal {};
            struct NotFollowedBy : public NonTerminal {};
        };

        template<char C>
        struct Char : public _internal::Char {

        };

        template<char Start, char End>
        struct CharRange : _internal::CharRange {

        };

        struct First : public _internal::Terminal {

        };

        struct Nothing : public _internal::Terminal {

        };

        struct Any : public _internal::Terminal {

        };

        template<typename H, typename ...T>
        struct Seq : public Seq<T...> {

        };

        template<typename H>
        struct Seq<H> : public _internal::Seq {

        };



        template<typename H, typename ...T>
        struct Ord : public Ord<T...> {

        };

        template<typename H>
        struct Ord<H> : public _internal::Ord {

        };


        template<typename S>
        struct Plus : public _internal::Plus {

        };


        template<typename S>
        struct OneOrMore : public _internal::OneOrMore {

        };


        template<typename S>
        struct FollowedBy : public _internal::FollowedBy {

        };

        template<typename S>
        struct NotFollowedBy : public _internal::NotFollowedBy {

        };

        template<const char* label>
        struct Label {};
    }
} // namespace pika::clause
#endif //PIKA_CLAUSE_HPP