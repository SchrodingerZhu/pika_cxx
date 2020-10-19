#ifndef PIKA_CLAUSE_HPP
#define PIKA_CLAUSE_HPP

#include <tuple>

namespace pika {

    namespace clause {
        struct Clause {

        };

        struct Terminal : public Clause {

        };
        struct NonTerminal : public Clause {

        };

        template<char C>
        struct Char : public Terminal {

        };

        template<char Start, char End>
        struct CharRange : public Terminal {

        };

        struct Nothing : public Terminal {

        };

        struct Any : public Terminal {

        };

        template<typename H, typename ...T>
        struct Seq : private H, private Seq<T...> {

        };

        template<typename H>
        struct Seq<H> : private H, public NonTerminal {

        };

        template<typename H, typename ...T>
        struct Ord : private H, private Ord<T...> {

        };

        template<typename H>
        struct Ord<H> : private H, public NonTerminal {

        };

        template<typename S>
        struct Plus : public NonTerminal {

        };

        template<typename S>
        struct OneOrMore : public NonTerminal {

        };

        template<typename S>
        struct FollowedBy : public NonTerminal {

        };

        template<typename S>
        struct NotFollowedBy : public NonTerminal {

        };

    }
} // namespace pika::clause
#endif //PIKA_CLAUSE_HPP