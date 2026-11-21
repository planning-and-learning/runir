#ifndef RUNIR_KR_PS_FORMATTER_HPP_
#define RUNIR_KR_PS_FORMATTER_HPP_

#include "runir/formatter.hpp"
#include "runir/kr/declarations.hpp"
#include "runir/kr/ps/declarations.hpp"
#include "runir/kr/ps/dl/declarations.hpp"
#include "runir/kr/ps/program_proof_graph.hpp"

#include <fmt/format.h>

// The records the families share (features, conditions, effects, memory states, module symbols), as the families'
// parsers read them: each formatter's format is defined in src/kr/ps/formatter.cpp and explicitly instantiated there for
// each family and the repositories that hold its records. A family's rules, modules and programs are written by its own
// formatter.hpp.

/// A feature: as its concrete feature.
template<runir::kr::FamilyTag Family, runir::kr::ps::dl::FeatureTag FeatureTag, typename C>
struct fmt::formatter<urd::index_view<runir::kr::ps::Feature<Family, FeatureTag>, C>> : runir::detail::without_format_spec
{
    fmt::format_context::iterator format(const urd::index_view<runir::kr::ps::Feature<Family, FeatureTag>, C>& value, fmt::format_context& context) const;
};

/// A feature over a DL expression, one section per line: (:boolean (:symbol s) (:expression e)).
template<runir::kr::FamilyTag Family, runir::kr::ps::dl::FeatureTag FeatureTag, typename C>
struct fmt::formatter<urd::index_view<runir::kr::ps::ConcreteFeature<Family, FeatureTag>, C>> : runir::detail::without_format_spec
{
    fmt::format_context::iterator format(const urd::index_view<runir::kr::ps::ConcreteFeature<Family, FeatureTag>, C>& value,
                                         fmt::format_context& context) const;
};

/// A condition: (observation feature-symbol).
template<runir::kr::FamilyTag Family,
         runir::kr::ps::dl::FeatureTag FeatureTag,
         runir::kr::ps::dl::ConditionObservationTag<FeatureTag> ObservationTag,
         typename C>
struct fmt::formatter<urd::index_view<runir::kr::ps::ConcreteCondition<Family, FeatureTag, ObservationTag>, C>> :
    runir::detail::without_format_spec
{
    fmt::format_context::iterator
    format(const urd::index_view<runir::kr::ps::ConcreteCondition<Family, FeatureTag, ObservationTag>, C>& value,
           fmt::format_context& context) const;
};

/// An effect: (observation feature-symbol).
template<runir::kr::FamilyTag Family, runir::kr::ps::dl::FeatureTag FeatureTag, runir::kr::ps::dl::EffectObservationTag<FeatureTag> ObservationTag, typename C>
struct fmt::formatter<urd::index_view<runir::kr::ps::ConcreteEffect<Family, FeatureTag, ObservationTag>, C>> :
    runir::detail::without_format_spec
{
    fmt::format_context::iterator format(const urd::index_view<runir::kr::ps::ConcreteEffect<Family, FeatureTag, ObservationTag>, C>& value,
                                         fmt::format_context& context) const;
};

/// A proof graph's edge: rule=symbol, or rule=<none>.
template<runir::kr::ps::ProgramFamilyTag Family>
struct fmt::formatter<runir::kr::ps::ProgramProofEdgeLabel<Family>> : runir::detail::without_format_spec
{
    fmt::format_context::iterator format(const runir::kr::ps::ProgramProofEdgeLabel<Family>& value, fmt::format_context& context) const;
};

#endif
