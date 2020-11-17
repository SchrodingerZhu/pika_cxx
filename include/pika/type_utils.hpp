#ifndef PIKA_TYPE_UTILS_HPP
#define PIKA_TYPE_UTILS_HPP

#include <pika/clause.hpp>

namespace pika
{
    namespace type_utils
    {
        enum class BaseType
        {
            Seq,
            Ord,
            Asterisks,
            Optional,
            FollowedBy,
            NotFollowedBy,
            Plus,
            Char,
            CharRange,
            First,
            Nothing,
            Any,
            Error
        };
#define PIKA_CHECK_BASE(NAMESPACE, TYPE) \
    if constexpr (std::is_base_of_v<NAMESPACE::TYPE, std::decay_t<Clause>>) \
    { \
        return BaseType::TYPE; \
    }

        template<
            class Clause,
            typename = std::enable_if_t<
                std::is_base_of_v<pika::clause::Clause, Clause>>>
        constexpr BaseType get_base_type()
        {
            PIKA_CHECK_BASE(pika::clause::_internal, Seq)
            else PIKA_CHECK_BASE(pika::clause::_internal, Ord) else PIKA_CHECK_BASE(pika::clause::_internal, Asterisks) else PIKA_CHECK_BASE(pika::clause::_internal, Optional) else PIKA_CHECK_BASE(pika::clause::_internal, FollowedBy) else PIKA_CHECK_BASE(
                pika::clause::_internal, NotFollowedBy) else PIKA_CHECK_BASE(pika::clause::_internal, Plus) else PIKA_CHECK_BASE(pika::clause::_internal, Char) else PIKA_CHECK_BASE(pika::clause::_internal, CharRange) else PIKA_CHECK_BASE(pika::clause, First) else PIKA_CHECK_BASE(pika::clause, Nothing) else PIKA_CHECK_BASE(pika::clause, Any) else return BaseType::
                Error;
        }
    }
#undef PIKA_CHECK_BASE
}
#endif // PIKA_TYPE_UTILS_HPP