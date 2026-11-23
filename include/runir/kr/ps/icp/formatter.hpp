#ifndef RUNIR_KR_PS_ICP_FORMATTER_HPP_
#define RUNIR_KR_PS_ICP_FORMATTER_HPP_

#include "runir/formatter.hpp"
#include "runir/kr/ps/formatter.hpp"
#include "runir/kr/ps/icp/declarations.hpp"
#include "runir/kr/ps/icp/object_reference.hpp"

#include <fmt/format.h>
#include <optional>
#include <tyr/declarations.hpp>
#include <utility>

// The icp family as its parser reads it: each formatter's format is defined in src/kr/ps/icp/formatter.cpp and
// explicitly instantiated there for each rule kind, indexical and task kind. A rule, a module and a program are written
// one section per line, nested ones indented (urd::scoped_indent).

/// A condition operation: belongs or not-belongs.
template<>
struct fmt::formatter<runir::kr::ps::icp::ConditionOperation> : runir::detail::without_format_spec
{
    fmt::format_context::iterator format(runir::kr::ps::icp::ConditionOperation value, fmt::format_context& context) const;
};

/// An effect operation: enter or exit.
template<>
struct fmt::formatter<runir::kr::ps::icp::EffectOperation> : runir::detail::without_format_spec
{
    fmt::format_context::iterator format(runir::kr::ps::icp::EffectOperation value, fmt::format_context& context) const;
};

/// An argument's position: its number.
template<>
struct fmt::formatter<runir::kr::ps::icp::ArgumentPosition> : runir::detail::without_format_spec
{
    fmt::format_context::iterator format(runir::kr::ps::icp::ArgumentPosition value, fmt::format_context& context) const;
};

/// A reset pair: (before, after).
template<>
struct fmt::formatter<runir::kr::ps::icp::ResetPairView> : runir::detail::without_format_spec
{
    fmt::format_context::iterator format(const runir::kr::ps::icp::ResetPairView& value, fmt::format_context& context) const;
};

namespace runir::kr::ps::icp
{
/// Selects an x-condition's or x-effect's text with its arguments named as the crule names them (arg<position> without
/// one): std::pair { view, ArgumentNames<CruleView> { rule } }. A template, so that the crule's view is instantiated only
/// where it is formatted, after its view_mixin.
template<typename CruleView>
struct ArgumentNames
{
    std::optional<CruleView> rule;
};
}  // namespace runir::kr::ps::icp

/// An x-condition or x-effect: (operation (:argument name) (:concept c)), or (:register (:concept r)) for its object.
template<typename X>
struct fmt::formatter<std::pair<urd::index_view<X, runir::kr::ps::icp::Repository>,
                                runir::kr::ps::icp::ArgumentNames<runir::kr::ps::icp::RuleView<runir::kr::ps::icp::CruleTag>>>> :
    runir::detail::without_format_spec
{
    fmt::format_context::iterator format(const std::pair<urd::index_view<X, runir::kr::ps::icp::Repository>,
                                                         runir::kr::ps::icp::ArgumentNames<runir::kr::ps::icp::RuleView<runir::kr::ps::icp::CruleTag>>>& value,
                                         fmt::format_context& context) const;
};

/// An x-condition on its own: its arguments arg<position>.
template<>
struct fmt::formatter<urd::index_view<runir::kr::ps::icp::XCondition, runir::kr::ps::icp::Repository>> : runir::detail::without_format_spec
{
    fmt::format_context::iterator format(const urd::index_view<runir::kr::ps::icp::XCondition, runir::kr::ps::icp::Repository>& value,
                                         fmt::format_context& context) const;
};

/// An x-effect on its own: its arguments arg<position>.
template<>
struct fmt::formatter<urd::index_view<runir::kr::ps::icp::XEffect, runir::kr::ps::icp::Repository>> : runir::detail::without_format_spec
{
    fmt::format_context::iterator format(const urd::index_view<runir::kr::ps::icp::XEffect, runir::kr::ps::icp::Repository>& value,
                                         fmt::format_context& context) const;
};

/// A rule: (:kind its sections), one per line.
template<runir::kr::ps::icp::RuleKind Kind>
struct fmt::formatter<runir::kr::ps::icp::RuleView<Kind>> : runir::detail::without_format_spec
{
    fmt::format_context::iterator format(const runir::kr::ps::icp::RuleView<Kind>& value, fmt::format_context& context) const;
};

/// A rule of any kind: as its concrete rule.
template<>
struct fmt::formatter<runir::kr::ps::icp::RuleVariantView> : runir::detail::without_format_spec
{
    fmt::format_context::iterator format(const runir::kr::ps::icp::RuleVariantView& value, fmt::format_context& context) const;
};

/// A module: (:module its symbol, registers, entry, memory, features, rules and reset pairs), one per line.
template<>
struct fmt::formatter<runir::kr::ps::icp::ModuleView> : runir::detail::without_format_spec
{
    fmt::format_context::iterator format(const runir::kr::ps::icp::ModuleView& value, fmt::format_context& context) const;
};

/// A program: (:program its entry and its entry module), one per line.
template<>
struct fmt::formatter<runir::kr::ps::icp::ProgramView> : runir::detail::without_format_spec
{
    fmt::format_context::iterator format(const runir::kr::ps::icp::ProgramView& value, fmt::format_context& context) const;
};

/// A proof graph's vertex: state=... initial=... goal=... alive=... unsolvable=...
template<tyr::TaskKind Kind>
struct fmt::formatter<urd::data<runir::kr::ps::ProgramProofVertexLabel<runir::kr::IcpFamilyTag, Kind>>> : runir::detail::without_format_spec
{
    fmt::format_context::iterator format(const urd::data<runir::kr::ps::ProgramProofVertexLabel<runir::kr::IcpFamilyTag, Kind>>& value,
                                         fmt::format_context& context) const;
};

#endif
