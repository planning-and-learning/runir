#ifndef RUNIR_KR_PS_STRUCTURAL_TERMINATION_DETAIL_HPP_
#define RUNIR_KR_PS_STRUCTURAL_TERMINATION_DETAIL_HPP_

#include "runir/kr/ps/dl/declarations.hpp"
#include "runir/kr/ps/dl/structural_termination.hpp"

#include <boost/dynamic_bitset.hpp>
#include <concepts>
#include <cstddef>
#include <span>
#include <stdexcept>
#include <utility>
#include <vector>
#include <yggdrasil/core/dependent_false.hpp>

namespace runir::kr::ps::detail
{

struct RuleProfile
{
    std::size_t source_memory_position;
    std::size_t target_memory_position;
    boost::dynamic_bitset<> boolean_positive_conditions;
    boost::dynamic_bitset<> boolean_negative_conditions;
    boost::dynamic_bitset<> numerical_greater_conditions;
    boost::dynamic_bitset<> numerical_zero_conditions;
    boost::dynamic_bitset<> boolean_positive_effects;
    boost::dynamic_bitset<> boolean_negative_effects;
    boost::dynamic_bitset<> boolean_unchanged_effects;
    std::vector<dl::NumericalChange> numerical_changes;

    RuleProfile(std::size_t num_booleans, std::size_t num_numericals, std::size_t source_memory_position_ = 0, std::size_t target_memory_position_ = 0) :
        source_memory_position(source_memory_position_),
        target_memory_position(target_memory_position_),
        boolean_positive_conditions(num_booleans),
        boolean_negative_conditions(num_booleans),
        numerical_greater_conditions(num_numericals),
        numerical_zero_conditions(num_numericals),
        boolean_positive_effects(num_booleans),
        boolean_negative_effects(num_booleans),
        boolean_unchanged_effects(num_booleans),
        numerical_changes(num_numericals, dl::NumericalChange::UNCONSTRAINED)
    {
    }

    template<typename ObservationTag>
    auto& conditions() noexcept
    {
        if constexpr (std::same_as<ObservationTag, dl::Positive>)
            return boolean_positive_conditions;
        else if constexpr (std::same_as<ObservationTag, dl::Negative>)
            return boolean_negative_conditions;
        else if constexpr (std::same_as<ObservationTag, dl::GreaterZero>)
            return numerical_greater_conditions;
        else if constexpr (std::same_as<ObservationTag, dl::EqualZero>)
            return numerical_zero_conditions;
        else
            static_assert(ygg::dependent_false<ObservationTag>::value, "unhandled condition observation tag");
    }

    template<typename ObservationTag>
    auto& effects() noexcept
    {
        if constexpr (std::same_as<ObservationTag, dl::Positive>)
            return boolean_positive_effects;
        else if constexpr (std::same_as<ObservationTag, dl::Negative>)
            return boolean_negative_effects;
        else if constexpr (std::same_as<ObservationTag, dl::Unchanged>)
            return boolean_unchanged_effects;
        else
            static_assert(ygg::dependent_false<ObservationTag>::value, "unhandled Boolean effect observation tag");
    }

    template<typename ObservationTag>
    static constexpr dl::NumericalChange numerical_change() noexcept
    {
        if constexpr (std::same_as<ObservationTag, dl::Increases>)
            return dl::NumericalChange::INCREASES;
        else if constexpr (std::same_as<ObservationTag, dl::Decreases>)
            return dl::NumericalChange::DECREASES;
        else if constexpr (std::same_as<ObservationTag, dl::Unchanged>)
            return dl::NumericalChange::UNCHANGED;
        else
            static_assert(ygg::dependent_false<ObservationTag>::value, "unhandled numerical effect observation tag");
    }
};

struct QualitativePolicy
{
    std::size_t num_memory_states;
    std::size_t num_booleans;
    std::size_t num_numericals;
    std::vector<RuleProfile> rule_profiles;

    QualitativePolicy(std::size_t num_memory_states_, std::size_t num_booleans_, std::size_t num_numericals_) :
        num_memory_states(num_memory_states_),
        num_booleans(num_booleans_),
        num_numericals(num_numericals_)
    {
        if (num_memory_states == 0)
            throw std::invalid_argument("a qualitative policy requires at least one memory state");
    }

    std::size_t num_valuations() const noexcept { return std::size_t { 1 } << (num_booleans + num_numericals); }
    std::size_t num_vertices() const noexcept { return num_valuations() * num_memory_states; }
};

struct PolicyEdge
{
    std::size_t source;
    std::size_t target;
    std::size_t rule_position;
    bool alive = true;
};

struct SieveResult
{
    bool has_cycle;
    std::vector<std::size_t> component_of;
};

struct StrongComponents
{
    std::size_t count;
    std::vector<std::size_t> component_of;
};

struct ProjectedPolicyComponent
{
    QualitativePolicy policy;
    std::vector<std::size_t> memory_positions;
    std::vector<std::size_t> boolean_positions;
    std::vector<std::size_t> numerical_positions;
    std::vector<std::size_t> rule_positions;
};

struct SievedPolicyComponent
{
    ProjectedPolicyComponent projected;
    std::vector<PolicyEdge> edges;
    SieveResult sieve;
};

using ComponentSieveResult = std::vector<SievedPolicyComponent>;

struct IncompletePolicyResult
{
    enum class FeatureKind
    {
        BOOLEAN,
        NUMERICAL
    };

    struct BlockingReason
    {
        FeatureKind feature_kind;
        std::size_t feature_position;
        std::vector<std::size_t> opposing_rule_positions;
    };

    struct SurvivingRule
    {
        std::size_t rule_position;
        std::vector<BlockingReason> blocking_reasons;
    };

    std::vector<SurvivingRule> surviving_rules;

    bool is_terminating() const noexcept { return surviving_rules.empty(); }
};

boost::dynamic_bitset<> vertex_booleans(std::size_t vertex, const QualitativePolicy& policy);
boost::dynamic_bitset<> vertex_numericals(std::size_t vertex, const QualitativePolicy& policy);
std::vector<PolicyEdge> build_policy_edges(const QualitativePolicy& policy);
StrongComponents find_strong_components(const std::vector<PolicyEdge>& edges, std::size_t num_vertices);
std::vector<ProjectedPolicyComponent> project_policy_components(const QualitativePolicy& policy, std::span<const std::size_t> rule_positions);
SieveResult sieve_policy_graph(std::vector<PolicyEdge>& edges, const QualitativePolicy& policy);
ComponentSieveResult sieve_policy(const QualitativePolicy& policy, std::size_t max_features, const IncompletePolicyResult& incomplete_result);
ComponentSieveResult sieve_policy(const QualitativePolicy& policy, std::size_t max_features, bool use_incomplete_preprocessing);
IncompletePolicyResult incomplete_structural_termination(const QualitativePolicy& policy);

}  // namespace runir::kr::ps::detail

#endif
