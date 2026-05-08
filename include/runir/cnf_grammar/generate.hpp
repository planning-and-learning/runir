#ifndef RUNIR_CNF_GRAMMAR_GENERATE_HPP_
#define RUNIR_CNF_GRAMMAR_GENERATE_HPP_

#include "runir/cnf_grammar/constructor_repository.hpp"

#include <cstddef>
#include <tyr/planning/state_view.hpp>
#include <vector>

namespace runir::cnf_grammar
{

struct GenerateStatistics
{
    size_t num_generated = 0;
    size_t num_pruned = 0;
    size_t num_kept = 0;

    GenerateStatistics& operator+=(const GenerateStatistics& other) noexcept
    {
        num_generated += other.num_generated;
        num_pruned += other.num_pruned;
        num_kept += other.num_kept;
        return *this;
    }
};

struct GenerateOptions
{
    size_t max_syntactic_complexity = 0;
};

struct GenerateResults
{
    GenerateStatistics statistics;
};

template<tyr::planning::TaskKind Kind>
GenerateResults generate(GrammarView, const std::vector<tyr::planning::StateView<Kind>>&, const GenerateOptions&)
{
    return {};
}

}

#endif
