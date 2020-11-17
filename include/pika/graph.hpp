//
// Created by schrodinger on 10/24/20.
//

#ifndef PIKA_GRAPH_HPP
#define PIKA_GRAPH_HPP

#include <pika/clause.hpp>
#include <pika/memotable.hpp>
#include <pika/type_utils.hpp>

namespace pika
{
    namespace graph
    {
        struct TableEntry
        {
            const pika::clause::Clause* instance;
            std::vector<const pika::clause::Clause*> candidates;
            const size_t topological_order;

            TableEntry(
                const pika::clause::Clause* instance, size_t topological_order);
        };

        using ParsingItem = std::pair<size_t, const pika::clause::Clause*>;

        struct ClauseTable
        : public absl::flat_hash_map<std::type_index, TableEntry>
        {
            const std::vector<const pika::clause::Clause*> specials;
            const std::vector<const pika::clause::Clause*> terminals;
            pika::memotable::MemoTable memo_table;
            std::priority_queue<
                ParsingItem,
                std::vector<ParsingItem>,
                std::greater<>>
                column{};
            size_t current_pos;
            const clause::Clause* toplevel;

            explicit ClauseTable(
                std::vector<const pika::clause::Clause*> specials,
                std::vector<const pika::clause::Clause*> terminals,
                std::string_view target,
                const clause::Clause* toplevel);

            char get_current() const;

            void try_add(
                const clause::Clause*,
                size_t length,
                size_t fst_idx,
                std::vector<std::shared_ptr<memotable::Match>> subs);

            void add_candidates(std::type_index idx);

            bool eof() const;

            bool match_column();
            std::shared_ptr<memotable::Match> match();
        };

        ClauseTable construct_table(
            const clause::Clause& toplevel, std::string_view target);
    }
}
#endif // PIKA_GRAPH_HPP
