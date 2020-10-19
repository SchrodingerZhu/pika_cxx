#ifndef PIKA_MEMOTABLE_HPP
#define PIKA_MEMOTABLE_HPP

#include <pika/clause.hpp>
#include <absl/container/flat_hash_map.h>
#include <typeindex>
#include <utility>


#define PIKA_INLINE_MATCHED 7
namespace pika {
    namespace memotable {
        namespace _internal {
            enum class BaseType {
                Seq,
                Ord,
                OneOrMore,
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
                 if constexpr(std::is_base_of_v<NAMESPACE::TYPE, std::decay_t<Clause>>) \
                 { return BaseType::TYPE; }

            template<class Clause, typename = std::enable_if_t<std::is_base_of_v<pika::clause::Clause, Clause>>>
            constexpr BaseType get_base_type() {
                PIKA_CHECK_BASE(pika::clause::_internal, Seq)
                else PIKA_CHECK_BASE(pika::clause::_internal, Ord)
                else PIKA_CHECK_BASE(pika::clause::_internal, OneOrMore)
                else PIKA_CHECK_BASE(pika::clause::_internal, FollowedBy)
                else PIKA_CHECK_BASE(pika::clause::_internal, NotFollowedBy)
                else PIKA_CHECK_BASE(pika::clause::_internal, Plus)
                else PIKA_CHECK_BASE(pika::clause::_internal, Char)
                else PIKA_CHECK_BASE(pika::clause::_internal, CharRange)
                else PIKA_CHECK_BASE(pika::clause, First)
                else PIKA_CHECK_BASE(pika::clause, Nothing)
                else PIKA_CHECK_BASE(pika::clause, Any)
                else return BaseType::Error;
            }
        }
#undef PIKA_CHECK_BASE
        struct MemoKey {
            const std::type_index clause_type;
            const size_t start_position;
            const _internal::BaseType base_type;

            template<class Clause, typename = std::enable_if_t<std::is_base_of_v<pika::clause::Clause, Clause>>>
            MemoKey(const Clause &tag, size_t start_position) noexcept : base_type(_internal::get_base_type<Clause>()), start_position(start_position),
                                                                         clause_type(typeid(Clause)) {}

            template<typename H>
            friend H AbslHashValue(H h, const MemoKey &k) {
                return H::combine(std::move(h), k.clause_type, k.start_position);
            }

            bool operator==(const MemoKey &that) const noexcept {
                return that.start_position == start_position &&
                       that.clause_type == clause_type;
            }

            template<class C>
            [[nodiscard]] bool is_based_on() const noexcept {
                return _internal::get_base_type<C>() == base_type;
            }
        };

        class Match {
            const MemoKey key;
            const size_t length;
            const size_t sub_fst_idx;
            const std::vector<std::shared_ptr<Match>> sub_matches;
        public:

            Match(MemoKey key, size_t length, size_t sub_fst_idx, std::vector<std::shared_ptr<Match>> sub_matches)
                    : key(std::move(key)), length(length), sub_fst_idx(sub_fst_idx),
                      sub_matches(std::move(sub_matches)) {}

            std::vector<std::pair<std::string_view, Match>>
            get_sub_matches() {
                throw std::runtime_error("unimplemented"); /// TODO: implement this
                return this->get_sub_matches();
            }

            bool is_better_than(const Match& that) {
                if (&that == this) {
                    return false;
                }
                return (key.is_based_on<pika::clause::First>()
                        && this->sub_fst_idx < that.sub_fst_idx)
                        || this->length > that.length;
            }
        };

        class MemoTable : public absl::flat_hash_map<MemoKey, Match> {
            MemoTable() : absl::flat_hash_map<MemoKey, Match>() {

            }
        };
    }
}

#endif
