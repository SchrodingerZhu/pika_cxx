#ifndef PIKA_MEMOTABLE_HPP
#define PIKA_MEMOTABLE_HPP
#include <pika/clause.hpp>
#include <absl/container/flat_hash_map.h>
#include <typeindex>
namespace pika {
    namespace memotable {
        struct MemoKey {
            const std::type_index clause_type;
            size_t start_position;
            template<class Clause, typename = std::enable_if_t<std::is_base_of_v<pika::clause::Clause, Clause>>>
            MemoKey(const Clause&,  size_t start_position) : start_position(start_position), clause_type(typeid(Clause)) {
            }

            template <typename H>
            friend H AbslHashValue(H h, const MemoKey& k) {
                return H::combine(std::move(h), k.clause_type, k.start_position);
            }

            bool operator==(const MemoKey& that) const {
                return that.start_position == start_position &&
                    that.clause_type == clause_type;
            }
        };
    }
}

#endif
