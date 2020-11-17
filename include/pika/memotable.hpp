#ifndef PIKA_MEMOTABLE_HPP
#define PIKA_MEMOTABLE_HPP

#include <absl/container/btree_map.h>
#include <absl/container/flat_hash_map.h>
#include <pika/clause.hpp>
#include <pika/type_utils.hpp>
#include <typeindex>
#include <utility>

#define PIKA_INLINE_MATCHED 7
namespace pika
{
    namespace parse_tree
    {
        class TreeNode;
    }
    namespace utils
    {
        class IntervalUnion
        {
            absl::btree_map<size_t, size_t> segments;

          public:
            void add_interval(size_t start, size_t end);
            [[nodiscard]] IntervalUnion invert(size_t start, size_t end) const;
            [[nodiscard]] bool is_overlap(size_t start, size_t end) const;
            [[nodiscard]] size_t size() const;
        };
    }
    namespace memotable
    {
        struct MemoKey
        {
            const std::type_index clause_type;
            const size_t start_position;
            const clause::Clause* const tag;

            MemoKey(const clause::Clause* tag, size_t start_position) noexcept;

            template<typename H>
            friend H AbslHashValue(H h, const MemoKey& k)
            {
                return H::combine(
                    std::move(h), k.clause_type, k.start_position);
            }

            bool operator==(const MemoKey& that) const noexcept;

            [[nodiscard]] pika::type_utils::BaseType
            get_base_type() const noexcept;
        };

        class Match
        {
          public:
            const MemoKey key;
            const size_t length;
            const size_t sub_fst_idx;
            const std::vector<std::shared_ptr<Match>> sub_matches;

            friend pika::parse_tree::TreeNode;

            Match(
                MemoKey key,
                size_t length,
                size_t sub_fst_idx,
                std::vector<std::shared_ptr<Match>> sub_matches);

            bool is_better_than(const Match& that);

            [[nodiscard]] size_t get_length() const;

            bool operator<(const Match& that) const;
        };

        class MemoTable
        : public absl::flat_hash_map<MemoKey, std::shared_ptr<Match>>
        {
            std::string_view target;

          public:
            using OrderedMatches =
                absl::btree_map<size_t, std::shared_ptr<Match>>;
            using OrderedTable =
                absl::flat_hash_map<std::type_index, OrderedMatches>;
            friend pika::graph::ClauseTable;
            friend pika::parse_tree::TreeNode;

            explicit MemoTable(std::string_view target);

            [[nodiscard]] char get_char(size_t index) const;

            [[nodiscard]] bool at_end(size_t index) const;

            [[nodiscard]] OrderedMatches
            ordered_matches(std::type_index clause) const;
            // TODO: is this needed in our case?
            [[nodiscard]] OrderedTable ordered_matches() const;
        };
    }

    std::vector<std::shared_ptr<memotable::Match>>
    nonoverlapping_matches(const memotable::MemoTable::OrderedMatches& matches);
}

#endif
