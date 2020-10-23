#ifndef PIKA_MEMOTABLE_HPP
#define PIKA_MEMOTABLE_HPP

#include <pika/clause.hpp>
#include <absl/container/flat_hash_map.h>
#include <typeindex>
#include <pika/type_utils.hpp>
#include <utility>


#define PIKA_INLINE_MATCHED 7
namespace pika {
    namespace parse_tree {
        class TreeNode;
    }
    namespace memotable {
        struct MemoKey {
            const std::type_index clause_type;
            const size_t start_position;
            const clause::Clause *const tag;

            MemoKey(const clause::Clause *tag, size_t start_position) noexcept;

            template<typename H>
            friend H AbslHashValue(H h, const MemoKey &k) {
                return H::combine(std::move(h), k.clause_type, k.start_position);
            }

            bool operator==(const MemoKey &that) const noexcept;

            [[nodiscard]] pika::type_utils::BaseType get_base_type() const noexcept;
        };

        class Match {
            const MemoKey key;
            const size_t length;
            const size_t sub_fst_idx;
            const std::vector<std::shared_ptr<Match>> sub_matches;
        public:
            friend pika::parse_tree::TreeNode;

            Match(MemoKey key, size_t length, size_t sub_fst_idx, std::vector<std::shared_ptr<Match>> sub_matches);

            std::vector<std::pair<std::string_view, Match>>
            get_sub_matches();

            bool is_better_than(const Match &that);

            [[nodiscard]] size_t get_length() const;
        };

        class MemoTable : public absl::flat_hash_map<MemoKey, std::shared_ptr<Match>> {
            std::string_view target;
        public:
            friend pika::parse_tree::TreeNode;

            explicit MemoTable(std::string_view target);

            [[nodiscard]] char get_char(size_t index) const;

            [[nodiscard]] bool at_end(size_t index) const;
        };
    }
}

#endif
