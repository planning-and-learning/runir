#ifndef RUNIR_KR_PS_PROGRAM_PROOF_GRAPH_HPP_
#define RUNIR_KR_PS_PROGRAM_PROOF_GRAPH_HPP_

#include "runir/graphs/static_graph.hpp"
#include "runir/graphs/static_graph_builder.hpp"
#include "runir/kr/ps/declarations.hpp"

#include <cista/containers/optional.h>
#include <tuple>
#include <tyr/formalism/declarations.hpp>
#include <tyr/formalism/planning/repository.hpp>
#include <tyr/planning/declarations.hpp>
#include <urd/elements/flat.hpp>
#include <utility>
#include <yggdrasil/formalism/row_view.hpp>

namespace runir::kr::ps
{

template<ProgramFamilyTag Family, tyr::TaskKind Kind>
struct ProgramProofVertexLabel : urd::compared_by_identity<ProgramProofVertexLabel<Family, Kind>>
{
    ProgramStateView<Family, Kind> program_state;
    bool is_initial = false;
    bool is_goal = false;
    bool is_alive = false;
    bool is_unsolvable = false;

    ProgramProofVertexLabel(ProgramStateView<Family, Kind> program_state_, bool is_initial_, bool is_goal_, bool is_alive_, bool is_unsolvable_) noexcept :
        program_state(std::move(program_state_)),
        is_initial(is_initial_),
        is_goal(is_goal_),
        is_alive(is_alive_),
        is_unsolvable(is_unsolvable_)
    {
    }

    static constexpr auto cista_members() noexcept
    {
        return urd::Members<&ProgramProofVertexLabel::program_state,
                            &ProgramProofVertexLabel::is_initial,
                            &ProgramProofVertexLabel::is_goal,
                            &ProgramProofVertexLabel::is_alive,
                            &ProgramProofVertexLabel::is_unsolvable> {};
    }
    static constexpr auto identifying_members() noexcept { return cista_members(); }
};

template<ProgramFamilyTag Family>
struct ProgramProofEdgeLabel : urd::compared_by_identity<ProgramProofEdgeLabel<Family>>
{
    urd::flat_optional<tyr::formalism::planning::ActionRowView> action;
    urd::flat_optional<RuleView<Family>> rule;

    ProgramProofEdgeLabel() = default;
    ProgramProofEdgeLabel(urd::flat_optional<tyr::formalism::planning::ActionRowView> action_, urd::flat_optional<RuleView<Family>> rule_) :
        action(std::move(action_)),
        rule(std::move(rule_))
    {
    }

    static constexpr auto cista_members() noexcept { return urd::Members<&ProgramProofEdgeLabel::action, &ProgramProofEdgeLabel::rule> {}; }
    static constexpr auto identifying_members() noexcept { return urd::Members<&ProgramProofEdgeLabel::action, &ProgramProofEdgeLabel::rule> {}; }
};

template<ProgramFamilyTag Family, tyr::TaskKind Kind>
using ProgramProofGraphBuilder = graphs::StaticGraphBuilder<ProgramProofVertexLabel<Family, Kind>, ProgramProofEdgeLabel<Family>>;

template<ProgramFamilyTag Family, tyr::TaskKind Kind>
using ProgramProofGraph = graphs::StaticGraph<ProgramProofVertexLabel<Family, Kind>, ProgramProofEdgeLabel<Family>>;

}  // namespace runir::kr::ps

#endif
