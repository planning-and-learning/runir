#ifndef RUNIR_CNF_GRAMMAR_GENERATE_HPP_
#define RUNIR_CNF_GRAMMAR_GENERATE_HPP_

#include "runir/knowledge_representation/dl/cnf_grammar/constructor_repository.hpp"
#include "runir/knowledge_representation/dl/repository.hpp"

#include <chrono>
#include <concepts>
#include <cstddef>
#include <optional>
#include <tyr/planning/state_view.hpp>
#include <vector>

namespace runir::kr::dl::cnf_grammar
{

struct GenerateStatistics
{
    size_t num_generated = 0;
    size_t num_pruned = 0;
    size_t num_kept = 0;
    std::chrono::steady_clock::duration total_time = std::chrono::steady_clock::duration::zero();

    GenerateStatistics& operator+=(const GenerateStatistics& other) noexcept
    {
        num_generated += other.num_generated;
        num_pruned += other.num_pruned;
        num_kept += other.num_kept;
        total_time += other.total_time;
        return *this;
    }
};

struct GenerateOptions
{
    size_t max_syntactic_complexity = 0;
    std::optional<std::chrono::steady_clock::duration> max_time = std::nullopt;
};

enum class GenerateStatus
{
    COMPLETED,
    OUT_OF_TIME
};

struct GenerateResults
{
    GenerateStatistics statistics;
    GenerateStatus status = GenerateStatus::COMPLETED;
    std::vector<runir::kr::dl::ConstructorView<runir::kr::dl::ConceptTag>> concepts;
    std::vector<runir::kr::dl::ConstructorView<runir::kr::dl::RoleTag>> roles;
    std::vector<runir::kr::dl::ConstructorView<runir::kr::dl::BooleanTag>> booleans;
    std::vector<runir::kr::dl::ConstructorView<runir::kr::dl::NumericalTag>> numericals;

    template<runir::kr::dl::CategoryTag Category>
    auto& get_constructors() noexcept
    {
        if constexpr (std::same_as<Category, runir::kr::dl::ConceptTag>)
            return concepts;
        else if constexpr (std::same_as<Category, runir::kr::dl::RoleTag>)
            return roles;
        else if constexpr (std::same_as<Category, runir::kr::dl::BooleanTag>)
            return booleans;
        else if constexpr (std::same_as<Category, runir::kr::dl::NumericalTag>)
            return numericals;
    }
};

template<tyr::planning::TaskKind Kind>
GenerateResults generate(GrammarView grammar,
                         const std::vector<tyr::planning::StateView<Kind>>& states,
                         runir::kr::dl::ConstructorRepository& output_repository,
                         const GenerateOptions& options);

}

#endif
