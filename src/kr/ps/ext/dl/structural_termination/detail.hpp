#ifndef RUNIR_KR_PS_EXT_DL_STRUCTURAL_TERMINATION_DETAIL_HPP_
#define RUNIR_KR_PS_EXT_DL_STRUCTURAL_TERMINATION_DETAIL_HPP_

#include "runir/kr/ps/ext/dl/structural_termination.hpp"

#include <boost/dynamic_bitset.hpp>
#include <concepts>
#include <cstddef>
#include <utility>
#include <vector>
#include <yggdrasil/core/dependent_false.hpp>

namespace runir::kr::ps::ext::dl::detail
{

struct ModuleFeatures
{
    std::vector<ygg::Index<runir::kr::ps::Feature<runir::kr::ExtFamilyTag, runir::kr::ps::dl::BooleanFeature>>> booleans;
    std::vector<ygg::Index<runir::kr::ps::Feature<runir::kr::ExtFamilyTag, runir::kr::ps::dl::NumericalFeature>>> numericals;
};

struct ModuleRuleProfile
{
    std::size_t source_memory_position = 0;
    std::size_t target_memory_position = 0;
    boost::dynamic_bitset<> boolean_positive_conditions;
    boost::dynamic_bitset<> boolean_negative_conditions;
    boost::dynamic_bitset<> numerical_greater_conditions;
    boost::dynamic_bitset<> numerical_zero_conditions;
    boost::dynamic_bitset<> boolean_positive_effects;
    boost::dynamic_bitset<> boolean_negative_effects;
    boost::dynamic_bitset<> boolean_unchanged_effects;
    std::vector<NumericalChange> numerical_changes;

    ModuleRuleProfile(std::size_t num_booleans, std::size_t num_numericals) :
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

struct ModulePolicyEdge
{
    std::size_t source;
    std::size_t target;
    std::size_t rule_position;
    bool alive = true;
};

struct ModuleAnalysis
{
    std::vector<MemoryStateView> memory_states;
    std::vector<RuleVariantView> rules;
    ModuleFeatures features;
    std::vector<ModuleRuleProfile> profiles;
};

struct CegComponentAnalysis
{
    std::vector<std::size_t> memory_positions;
    std::vector<std::size_t> boolean_positions;
    std::vector<std::size_t> numerical_positions;
    std::vector<std::size_t> rule_positions;
    std::vector<ModuleRuleProfile> profiles;
};

ModuleAnalysis analyze_module(ModuleView module_);

boost::dynamic_bitset<> vertex_booleans(std::size_t vertex, std::size_t num_memory_states, std::size_t num_booleans);
boost::dynamic_bitset<> vertex_numericals(std::size_t vertex, std::size_t num_memory_states, std::size_t num_booleans, std::size_t num_numericals);

std::vector<ModulePolicyEdge> build_policy_edges(const ModuleAnalysis& analysis);
std::vector<ModulePolicyEdge>
build_policy_edges(const std::vector<ModuleRuleProfile>& profiles, std::size_t num_memory_states, std::size_t num_booleans, std::size_t num_numericals);

std::pair<bool, std::vector<std::size_t>> sieve_policy_graph(std::vector<ModulePolicyEdge>& edges, const ModuleAnalysis& analysis);
std::pair<bool, std::vector<std::size_t>> sieve_policy_graph(std::vector<ModulePolicyEdge>& edges,
                                                             const std::vector<ModuleRuleProfile>& profiles,
                                                             std::size_t num_memory_states,
                                                             std::size_t num_booleans,
                                                             std::size_t num_numericals);

ModuleStructuralTerminationResult
make_result(const ModuleAnalysis& analysis, const std::vector<ModulePolicyEdge>& edges, bool has_cycle, const std::vector<std::size_t>& component_of);

std::vector<CegComponentAnalysis> analyze_ceg_components(const ModuleAnalysis& analysis);
void append_ceg_counterexample(const ModuleAnalysis& analysis,
                               const CegComponentAnalysis& component,
                               const std::vector<ModulePolicyEdge>& edges,
                               const std::vector<std::size_t>& component_of,
                               ModulePolicyGraphBuilder& builder);

}  // namespace runir::kr::ps::ext::dl::detail

#endif
