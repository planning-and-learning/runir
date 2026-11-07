#ifndef RUNIR_KR_PS_EXT_PROGRAM_PROOF_GRAPH_HPP_
#define RUNIR_KR_PS_EXT_PROGRAM_PROOF_GRAPH_HPP_

#include "runir/graphs/static_graph.hpp"
#include "runir/graphs/static_graph_builder.hpp"
#include "runir/kr/ps/ext/execution_view.hpp"

#include <cista/containers/optional.h>
#include <tuple>
#include <tyr/formalism/declarations.hpp>
#include <yggdrasil/formalism/binding_view.hpp>
#include <tyr/formalism/planning/repository.hpp>
#include <tyr/planning/declarations.hpp>
#include <utility>
#include <urd/elements/flat.hpp>

namespace runir::kr::ps::ext
{

template<tyr::TaskKind Kind>
struct ProgramProofVertexLabel : urd::compared_by_identity<ProgramProofVertexLabel<Kind>>
{
    ProgramStateView<Kind> program_state;
    bool is_initial = false;
    bool is_goal = false;
    bool is_alive = false;
    bool is_unsolvable = false;

    ProgramProofVertexLabel(ProgramStateView<Kind> program_state_, bool is_initial_, bool is_goal_, bool is_alive_, bool is_unsolvable_) noexcept :
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

struct ProgramProofEdgeLabel : urd::compared_by_identity<ProgramProofEdgeLabel>
{
    urd::flat_optional<tyr::formalism::planning::ActionBindingView> action;
    urd::flat_optional<RuleVariantView> rule;

    ProgramProofEdgeLabel() = default;
    ProgramProofEdgeLabel(urd::flat_optional<tyr::formalism::planning::ActionBindingView> action_, urd::flat_optional<RuleVariantView> rule_) :
        action(std::move(action_)),
        rule(std::move(rule_))
    {
    }

    static constexpr auto cista_members() noexcept { return urd::Members<&ProgramProofEdgeLabel::action, &ProgramProofEdgeLabel::rule> {}; }
    static constexpr auto identifying_members() noexcept { return urd::Members<&ProgramProofEdgeLabel::action, &ProgramProofEdgeLabel::rule> {}; }
};

template<tyr::TaskKind Kind>
using ProgramProofGraphBuilder = graphs::StaticGraphBuilder<ProgramProofVertexLabel<Kind>, ProgramProofEdgeLabel>;

template<tyr::TaskKind Kind>
using ProgramProofGraph = graphs::StaticGraph<ProgramProofVertexLabel<Kind>, ProgramProofEdgeLabel>;

}  // namespace runir::kr::ps::ext

#endif
