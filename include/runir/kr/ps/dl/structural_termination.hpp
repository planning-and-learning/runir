#ifndef RUNIR_KR_PS_DL_STRUCTURAL_TERMINATION_HPP_
#define RUNIR_KR_PS_DL_STRUCTURAL_TERMINATION_HPP_

#include "runir/kr/ps/dl/declarations.hpp"
#include "runir/kr/ps/feature_view.hpp"

#include <concepts>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <optional>
#include <stdexcept>
#include <vector>
#include <yggdrasil/containers/dynamic_bitset.hpp>
#include <yggdrasil/core/dependent_false.hpp>

namespace runir::kr::ps::dl
{

static constexpr std::size_t default_max_features = 10;
static constexpr std::size_t max_supported_features = 64;
static constexpr bool default_use_incomplete_preprocessing = true;
static constexpr bool default_use_memory_scc_scope = true;

inline void validate_feature_capacity(std::size_t num_booleans, std::size_t num_numericals)
{
    if (num_booleans > max_supported_features || num_numericals > max_supported_features - num_booleans)
        throw std::invalid_argument("structural_termination: a policy supports at most 64 Boolean and numerical features in total");
}

/// Qualitative conditions and effects in the policy's per-kind feature order.
/// Each feature belongs to exactly one of its four effect masks.
struct RuleProfile
{
    std::size_t source_memory_position = 0;
    std::size_t target_memory_position = 0;
    std::uint64_t boolean_positive_conditions = 0;
    std::uint64_t boolean_negative_conditions = 0;
    std::uint64_t numerical_greater_conditions = 0;
    std::uint64_t numerical_zero_conditions = 0;
    std::uint64_t boolean_positive_effects = 0;
    std::uint64_t boolean_negative_effects = 0;
    std::uint64_t boolean_unchanged_effects = 0;
    std::uint64_t boolean_unconstrained_effects = 0;
    std::uint64_t numerical_increase_effects = 0;
    std::uint64_t numerical_decrease_effects = 0;
    std::uint64_t numerical_unchanged_effects = 0;
    std::uint64_t numerical_unconstrained_effects = 0;

    RuleProfile(std::size_t num_booleans, std::size_t num_numericals, std::size_t source_memory_position_ = 0, std::size_t target_memory_position_ = 0) :
        source_memory_position(source_memory_position_),
        target_memory_position(target_memory_position_)
    {
        validate_feature_capacity(num_booleans, num_numericals);
        ygg::BitsetSpan<std::uint64_t>(&boolean_unconstrained_effects, num_booleans).set();
        ygg::BitsetSpan<std::uint64_t>(&numerical_unconstrained_effects, num_numericals).set();
    }

    template<typename ObservationTag>
    auto& conditions() noexcept
    {
        if constexpr (std::same_as<ObservationTag, Positive>)
            return boolean_positive_conditions;
        else if constexpr (std::same_as<ObservationTag, Negative>)
            return boolean_negative_conditions;
        else if constexpr (std::same_as<ObservationTag, GreaterZero>)
            return numerical_greater_conditions;
        else if constexpr (std::same_as<ObservationTag, EqualZero>)
            return numerical_zero_conditions;
        else
            static_assert(ygg::dependent_false<ObservationTag>::value, "unhandled condition observation tag");
    }

    template<typename FeatureTag, typename ObservationTag>
    auto& effects() noexcept
    {
        if constexpr (std::same_as<FeatureTag, BooleanFeature> && std::same_as<ObservationTag, Positive>)
            return boolean_positive_effects;
        else if constexpr (std::same_as<FeatureTag, BooleanFeature> && std::same_as<ObservationTag, Negative>)
            return boolean_negative_effects;
        else if constexpr (std::same_as<FeatureTag, BooleanFeature> && std::same_as<ObservationTag, Unchanged>)
            return boolean_unchanged_effects;
        else if constexpr (std::same_as<FeatureTag, BooleanFeature> && std::same_as<ObservationTag, Unconstrained>)
            return boolean_unconstrained_effects;
        else if constexpr (std::same_as<FeatureTag, NumericalFeature> && std::same_as<ObservationTag, Increases>)
            return numerical_increase_effects;
        else if constexpr (std::same_as<FeatureTag, NumericalFeature> && std::same_as<ObservationTag, Decreases>)
            return numerical_decrease_effects;
        else if constexpr (std::same_as<FeatureTag, NumericalFeature> && std::same_as<ObservationTag, Unchanged>)
            return numerical_unchanged_effects;
        else if constexpr (std::same_as<FeatureTag, NumericalFeature> && std::same_as<ObservationTag, Unconstrained>)
            return numerical_unconstrained_effects;
        else
            static_assert(ygg::dependent_false<ObservationTag>::value, "unhandled effect feature or observation tag");
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
        validate_feature_capacity();
    }

    void validate_feature_capacity() const { dl::validate_feature_capacity(num_booleans, num_numericals); }

    std::size_t num_valuations() const
    {
        constexpr auto digits = std::numeric_limits<std::size_t>::digits;
        if (num_booleans >= digits || num_numericals >= digits - num_booleans)
            throw std::invalid_argument("a qualitative policy has too many features to enumerate valuations");
        return std::size_t { 1 } << (num_booleans + num_numericals);
    }

    std::size_t num_vertices() const
    {
        const auto valuations = num_valuations();
        if (num_memory_states > std::numeric_limits<std::size_t>::max() / valuations)
            throw std::invalid_argument("a qualitative policy has too many vertices");
        return valuations * num_memory_states;
    }
};

struct IncompletePolicyResult
{
    enum class FeatureKind
    {
        BOOLEAN,
        NUMERICAL
    };

    struct MarkedFeature
    {
        FeatureKind feature_kind;
        std::size_t feature_position;
        /// Original policy-rule positions that established this mark.
        std::vector<std::size_t> witnessing_rule_positions;
    };

    struct ResidualMemoryScope
    {
        /// Original memory positions in this final residual SCC.
        std::vector<std::size_t> memory_positions;
        /// Effective marks, including marks inherited through SCC refinement.
        std::vector<MarkedFeature> marked_features;
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

    std::vector<ResidualMemoryScope> residual_scopes;
    std::vector<SurvivingRule> surviving_rules;

    bool is_terminating() const noexcept { return surviving_rules.empty(); }
    /// Original rule positions not ruled out by incomplete elimination.
    std::vector<std::size_t> get_cyclic_rule_positions() const;
};

enum class StructuralTerminationStatus
{
    TERMINATING,
    NON_TERMINATING,
};

struct CounterexampleVertex
{
    /// Original memory position.
    std::size_t memory_position;
    /// Full-policy axes; only the component's listed feature positions are meaningful.
    std::uint64_t boolean_values;
    std::uint64_t numerical_values;
};

struct CounterexampleEdge
{
    std::size_t source_vertex_position;
    std::size_t target_vertex_position;
    std::size_t rule_position;
};

struct CounterexampleComponent
{
    /// Original positions participating in this final cyclic SCC.
    std::vector<std::size_t> memory_positions;
    std::vector<std::size_t> boolean_positions;
    std::vector<std::size_t> numerical_positions;
    std::vector<CounterexampleVertex> vertices;
    std::vector<CounterexampleEdge> edges;

    /// One closed path of positions into edges, preserving the rule labels.
    std::vector<std::size_t> get_cycle() const;
};

struct StructuralTerminationResult
{
    StructuralTerminationStatus status = StructuralTerminationStatus::TERMINATING;
    /// Present only on failure when incomplete preprocessing was enabled.
    std::optional<IncompletePolicyResult> incomplete_frontier;
    /// One normalized component per final cyclic SCC; empty iff terminating.
    std::vector<CounterexampleComponent> counterexample_components;

    bool is_terminating() const noexcept { return status == StructuralTerminationStatus::TERMINATING; }
    /// Sorted unique original rule positions on any final residual cycle.
    /// Hybrid results use the final SIEVE frontier, not the incomplete prefix.
    std::vector<std::size_t> get_cyclic_rule_positions() const;
};

/// Sound incomplete elimination; surviving rules are potentially cyclic.
IncompletePolicyResult incomplete_structural_termination(const QualitativePolicy& policy, bool use_memory_scc_scope = default_use_memory_scc_scope);

/// Decides structural termination using incomplete elimination followed by
/// complete SIEVE. A non-terminating result contains only the stable failure
/// frontier, not the algorithm's recursive trace. Policies support at most
/// 64 features in total; max_features and enumeration limits apply per
/// projected residual component.
StructuralTerminationResult structural_termination(const QualitativePolicy& policy,
                                                   std::size_t max_features = default_max_features,
                                                   bool use_incomplete_preprocessing = default_use_incomplete_preprocessing,
                                                   bool use_memory_scc_scope = default_use_memory_scc_scope);

template<runir::kr::FamilyTag Family, typename C>
struct SccStructuralTerminationResult
{
    std::vector<ygg::View<ygg::Index<runir::kr::ps::Feature<Family, BooleanFeature>>, C>> booleans;
    std::vector<ygg::View<ygg::Index<runir::kr::ps::Feature<Family, NumericalFeature>>, C>> numericals;
};

}  // namespace runir::kr::ps::dl

#endif
