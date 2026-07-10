#ifndef RUNIR_KR_PS_BASE_DL_STRUCTURAL_TERMINATION_DETAIL_HPP_
#define RUNIR_KR_PS_BASE_DL_STRUCTURAL_TERMINATION_DETAIL_HPP_

#include "runir/kr/ps/base/dl/incomplete_structural_termination.hpp"
#include "runir/kr/ps/base/dl/structural_termination.hpp"
#include "runir/kr/ps/base/dl/syntactic_complexity.hpp"
#include "runir/kr/ps/base/rule_view.hpp"

#include <boost/dynamic_bitset.hpp>
#include <concepts>
#include <cstddef>
#include <utility>
#include <vector>
#include <yggdrasil/core/dependent_false.hpp>

namespace runir::kr::ps::base::dl::detail
{

struct RuleProfile
{
    boost::dynamic_bitset<> boolean_positive_conditions;
    boost::dynamic_bitset<> boolean_negative_conditions;
    boost::dynamic_bitset<> numerical_greater_conditions;
    boost::dynamic_bitset<> numerical_zero_conditions;
    boost::dynamic_bitset<> boolean_positive_effects;
    boost::dynamic_bitset<> boolean_negative_effects;
    boost::dynamic_bitset<> boolean_unchanged_effects;
    std::vector<NumericalChange> numerical_changes;

    RuleProfile(std::size_t num_booleans, std::size_t num_numericals) :
        boolean_positive_conditions(num_booleans),
        boolean_negative_conditions(num_booleans),
        numerical_greater_conditions(num_numericals),
        numerical_zero_conditions(num_numericals),
        boolean_positive_effects(num_booleans),
        boolean_negative_effects(num_booleans),
        boolean_unchanged_effects(num_booleans),
        numerical_changes(num_numericals, NumericalChange::UNCONSTRAINED)
    {
    }

    template<typename ObservationTag>
    auto& conditions() noexcept
    {
        namespace psdl = runir::kr::ps::dl;
        if constexpr (std::same_as<ObservationTag, psdl::Positive>)
            return boolean_positive_conditions;
        else if constexpr (std::same_as<ObservationTag, psdl::Negative>)
            return boolean_negative_conditions;
        else if constexpr (std::same_as<ObservationTag, psdl::GreaterZero>)
            return numerical_greater_conditions;
        else if constexpr (std::same_as<ObservationTag, psdl::EqualZero>)
            return numerical_zero_conditions;
        else
            static_assert(ygg::dependent_false<ObservationTag>::value, "unhandled condition observation tag");
    }

    template<typename ObservationTag>
    auto& effects() noexcept
    {
        namespace psdl = runir::kr::ps::dl;
        if constexpr (std::same_as<ObservationTag, psdl::Positive>)
            return boolean_positive_effects;
        else if constexpr (std::same_as<ObservationTag, psdl::Negative>)
            return boolean_negative_effects;
        else if constexpr (std::same_as<ObservationTag, psdl::Unchanged>)
            return boolean_unchanged_effects;
        else
            static_assert(ygg::dependent_false<ObservationTag>::value, "unhandled Boolean effect observation tag");
    }

    template<typename ObservationTag>
    static constexpr NumericalChange numerical_change() noexcept
    {
        namespace psdl = runir::kr::ps::dl;
        if constexpr (std::same_as<ObservationTag, psdl::Increases>)
            return NumericalChange::INCREASES;
        else if constexpr (std::same_as<ObservationTag, psdl::Decreases>)
            return NumericalChange::DECREASES;
        else if constexpr (std::same_as<ObservationTag, psdl::Unchanged>)
            return NumericalChange::UNCHANGED;
        else
            static_assert(ygg::dependent_false<ObservationTag>::value, "unhandled numerical effect observation tag");
    }
};

struct PolicyEdge
{
    std::size_t source;
    std::size_t target;
    std::size_t rule_position;
    bool alive = true;
};

struct SketchAnalysis
{
    FeatureSyntacticComplexityContext features;
    std::vector<RuleView> rules;
    std::vector<RuleProfile> profiles;
};

SketchAnalysis analyze_sketch(SketchView sketch);
std::vector<PolicyEdge> build_policy_edges(const SketchAnalysis& analysis);
std::pair<bool, std::vector<std::size_t>> sieve_policy_graph(std::vector<PolicyEdge>& edges, const SketchAnalysis& analysis);
StructuralTerminationResult
make_result(const SketchAnalysis& analysis, const std::vector<PolicyEdge>& edges, bool has_cycle, const std::vector<std::size_t>& component_of);
boost::dynamic_bitset<> vertex_booleans(std::size_t vertex, std::size_t num_booleans);
boost::dynamic_bitset<> vertex_numericals(std::size_t vertex, std::size_t num_booleans, std::size_t num_numericals);

struct IncompleteRuleChanges
{
    boost::dynamic_bitset<> boolean_to_true;
    boost::dynamic_bitset<> boolean_to_false;
    boost::dynamic_bitset<> numerical_decreases;
    boost::dynamic_bitset<> numerical_increases_or_unconstrained;
};

struct IncompleteSieveResult
{
    std::vector<IncompleteRuleChanges> changes;
    std::vector<bool> remaining;
    boost::dynamic_bitset<> marked_booleans;
    boost::dynamic_bitset<> marked_numericals;
};

IncompleteSieveResult run_incomplete_sieve(const SketchAnalysis& analysis);
std::vector<std::size_t> opposing_rules(const SketchAnalysis& analysis,
                                        const IncompleteSieveResult& sieve_result,
                                        std::size_t rule_position,
                                        bool is_boolean,
                                        std::size_t feature_position,
                                        bool to_true);
IncompleteStructuralTerminationResult make_incomplete_result(SketchView sketch, const SketchAnalysis& analysis, const IncompleteSieveResult& sieve_result);

}  // namespace runir::kr::ps::base::dl::detail

#endif
