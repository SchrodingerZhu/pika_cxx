//
// Created by schrodinger on 10/24/20.
//
#include <pika/graph.hpp>

#ifdef PIKA_DEBUG
#    include <iostream>
#endif

pika::graph::TableEntry::TableEntry(
    const pika::clause::Clause* instance, size_t topological_order)
: instance(instance), candidates({}), topological_order(topological_order)
{}

pika::graph::ClauseTable::ClauseTable(
    std::vector<const pika::clause::Clause*> specials,
    std::vector<const pika::clause::Clause*> terminals,
    std::string_view target,
    const clause::Clause* toplevel)
: specials(std::move(specials)),
  terminals(std::move(terminals)),
  memo_table(target),
  current_pos(target.size() + 1),
  toplevel(toplevel)
{}

char pika::graph::ClauseTable::get_current() const
{
    return (current_pos > memo_table.target.size()) ?
        EOF :
        memo_table.target[current_pos - 1];
}

void pika::graph::ClauseTable::add_candidates(std::type_index idx)
{
    for (auto i : this->at(idx).candidates)
    {
#ifdef PIKA_DEBUG
        std::cout << "added: "
                  << abi::__cxa_demangle(
                         typeid(*i).name(), nullptr, nullptr, nullptr)
                  << ", at: " << this->current_pos - 1 << std::endl;
#endif
        this->column.template emplace(
            this->template at(typeid(*i)).topological_order, i);
    }
}

bool pika::graph::ClauseTable::eof() const
{
    return memo_table.target.size() < current_pos;
}

void pika::graph::ClauseTable::try_add(
    const pika::clause::Clause* tag,
    size_t length,
    size_t fst_idx,
    std::vector<std::shared_ptr<memotable::Match>> subs)
{
    auto key = memotable::MemoKey{tag, current_pos - 1};
#ifdef PIKA_DEBUG
    std::cout << "matched "
              << abi::__cxa_demangle(
                     typeid(*tag).name(), nullptr, nullptr, nullptr)
              << ", at " << this->current_pos - 1 << ", content: "
              << this->memo_table.target.substr(
                     this->current_pos - 1, this->current_pos - 1 + length)
              << ", subs_len: " << subs.size() << ", length: " << length;
#endif
    auto match = memotable::Match{key, length, fst_idx, std::move(subs)};
    if (!memo_table.contains(key) || match.is_better_than(*memo_table.at(key)))
    {
#ifdef PIKA_DEBUG
        std::cout << ", this is a better match" << std::endl;
#endif
        memo_table[key] = std::make_shared<memotable::Match>(std::move(match));
        add_candidates(typeid(*tag));
    }
    else
    {
#ifdef PIKA_DEBUG
        std::cout << ", this is not a better match" << std::endl;
#endif
    }
}

bool pika::graph::ClauseTable::match_column()
{
    if (current_pos == 0)
        return false;
    assert(this->column.empty());
    for (auto i : terminals)
    {
        column.emplace(this->at(typeid(*i)).topological_order, i);
    }
    for (auto i : specials)
    {
        column.emplace(this->at(typeid(*i)).topological_order, i);
    }
    while (!column.empty())
    {
        const clause::Clause* top = column.top().second;
        column.pop();
#ifdef PIKA_DEBUG
        std::cout << "trying to parse: "
                  << abi::__cxa_demangle(
                         typeid(*top).name(), nullptr, nullptr, nullptr)
                  << ", at: " << this->current_pos - 1 << std::endl;
#endif
        top->pika_match(*this);
    }
    current_pos -= 1;
    return true;
}

std::shared_ptr<pika::memotable::Match> pika::graph::ClauseTable::match()
{
    while (match_column())
        ;
    auto key = memotable::MemoKey{toplevel, 0};
    if (memo_table.contains(key))
    {
        return memo_table.at(key);
    }
    return nullptr;
}

pika::graph::ClauseTable pika::graph::construct_table(
    const pika::clause::Clause& toplevel, std::string_view target)
{
    std::vector<const pika::clause::Clause*> terminals;
    std::vector<const pika::clause::Clause*> nodes;
    std::vector<const pika::clause::Clause*> specials;
    absl::flat_hash_set<std::type_index> visited;
    toplevel.dfs_traversal(visited, terminals, nodes);

    /*
     * The following things may always success, we need to check them everytime
     * the priority queue is empty They are behaving like a self-looping.
     */
    for (auto i : nodes)
    {
        if (i->get_base_type() == type_utils::BaseType::Asterisks ||
            i->get_base_type() == type_utils::BaseType::Optional ||
            i->get_base_type() == type_utils::BaseType::NotFollowedBy)
        {
            specials.push_back(i);
        }
    }

    ClauseTable table(
        std::move(specials), terminals, target, toplevel.get_instance());

    /*
     * All terminals are marked with 0
     */
    for (auto i : terminals)
    {
        table.emplace(typeid(*i), TableEntry{i, 0});
    }

    /*
     * Non-terminals are marked with higher values respectively
     */
    for (size_t i = 0; i < nodes.size(); ++i)
    {
        table.emplace(typeid(*nodes[i]), TableEntry{nodes[i], i + 1});
    }

    for (auto i : nodes)
    {
        i->mark_seeds(table);
    }

    return table;
}
