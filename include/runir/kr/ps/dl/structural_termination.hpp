#ifndef RUNIR_KR_PS_DL_STRUCTURAL_TERMINATION_HPP_
#define RUNIR_KR_PS_DL_STRUCTURAL_TERMINATION_HPP_

#include "runir/kr/ps/dl/declarations.hpp"
#include "runir/kr/ps/feature_view.hpp"

#include <boost/dynamic_bitset.hpp>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <optional>
#include <stdexcept>
#include <vector>
#include <yggdrasil/core/dependent_false.hpp>

namespace runir::kr::ps::dl
{

static constexpr std::size_t default_max_features = 10;
static constexpr bool default_use_incomplete_preprocessing = true;
static constexpr bool default_use_memory_scc_scope = true;

/// Qualitative constraint on one numerical feature in a normalized policy rule.
enum class NumericalChange : std::uint8_t
{
    UNCONSTRAINED = 0,
    INCREASES = 1,
    DECREASES = 2,
    UNCHANGED = 3,
};

/// Qualitative conditions and effects for one policy rule. A Boolean effect
/// absent from all three effect bitsets is unconstrained, not unchanged.
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
    std::vector<NumericalChange> numerical_changes;

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
        numerical_changes(num_numericals, NumericalChange::UNCONSTRAINED)
    {
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

    template<typename ObservationTag>
    auto& effects() noexcept
    {
        if constexpr (std::same_as<ObservationTag, Positive>)
            return boolean_positive_effects;
        else if constexpr (std::same_as<ObservationTag, Negative>)
            return boolean_negative_effects;
        else if constexpr (std::same_as<ObservationTag, Unchanged>)
            return boolean_unchanged_effects;
        else
            static_assert(ygg::dependent_false<ObservationTag>::value, "unhandled Boolean effect observation tag");
    }

    template<typename ObservationTag>
    static constexpr NumericalChange numerical_change() noexcept
    {
        if constexpr (std::same_as<ObservationTag, Increases>)
            return NumericalChange::INCREASES;
        else if constexpr (std::same_as<ObservationTag, Decreases>)
            return NumericalChange::DECREASES;
        else if constexpr (std::same_as<ObservationTag, Unchanged>)
            return NumericalChange::UNCHANGED;
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
    boost::dynamic_bitset<> boolean_values;
    boost::dynamic_bitset<> numerical_values;
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
};

struct StructuralTerminationResult
{
    StructuralTerminationStatus status = StructuralTerminationStatus::TERMINATING;
    /// Present only on failure when incomplete preprocessing was enabled.
    std::optional<IncompletePolicyResult> incomplete_frontier;
    /// One normalized component per final cyclic SCC; empty iff terminating.
    std::vector<CounterexampleComponent> counterexample_components;

    bool is_terminating() const noexcept { return status == StructuralTerminationStatus::TERMINATING; }
};

/// Decides structural termination using incomplete elimination followed by
/// complete SIEVE. A non-terminating result contains only the stable failure
/// frontier, not the algorithm's recursive trace. Valuation capacity and
/// max_features apply per projected residual component, not to unused global
/// feature axes.
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
