#ifndef RUNIR_KR_PS_BASE_SKETCH_PROOF_GRAPH_HPP_
#define RUNIR_KR_PS_BASE_SKETCH_PROOF_GRAPH_HPP_

#include "runir/datasets/graph_types.hpp"
#include "runir/kr/ps/base/declarations.hpp"
#include "runir/kr/ps/base/repository.hpp"
#include "runir/kr/ps/base/rule_view.hpp"

#include <string_view>
#include <tuple>
#include <type_traits>
#include <tyr/formalism/declarations.hpp>
#include <tyr/planning/declarations.hpp>
#include <tyr/planning/states/state_repository.hpp>
#include <urd/elements/concepts.hpp>
#include <urd/repr/graphs/csr_graph.hpp>
#include <utility>
#include <yggdrasil/formalism/row_view.hpp>

namespace urd
{
template<tyr::TaskKind Kind>
struct data<runir::kr::ps::base::SketchProofVertexLabel<Kind>>
{
    tyr::planning::StateView<Kind> state;
    bool is_initial {};
    bool is_goal {};
    bool is_alive {};
    bool is_unsolvable {};

    data() = default;

    data(tyr::planning::StateView<Kind> state_, bool is_initial_, bool is_goal_, bool is_alive_, bool is_unsolvable_) noexcept :
        state(state_),
        is_initial(is_initial_),
        is_goal(is_goal_),
        is_alive(is_alive_),
        is_unsolvable(is_unsolvable_)
    {
    }

    static constexpr bool copyable() noexcept
    {
        static_assert(std::is_trivially_copyable_v<data>, "a copyable record is trivially copyable");
        return true;
    }

    static constexpr auto fields() noexcept
    {
        return urd::Fields<urd::field<"state", &data::state>,
                           urd::field<"is_initial", &data::is_initial>,
                           urd::field<"is_goal", &data::is_goal>,
                           urd::field<"is_alive", &data::is_alive>,
                           urd::field<"is_unsolvable", &data::is_unsolvable>> {};
    }

    static constexpr auto identifying_members() noexcept { return urd::members_of(fields()); }

    static constexpr auto cista_members() noexcept
    { return urd::Members<&data::state, &data::is_initial, &data::is_goal, &data::is_alive, &data::is_unsolvable> {}; }
};

template<>
struct data<runir::kr::ps::base::SketchProofEdgeLabel>
{
    tyr::formalism::ActionRowView action;
    runir::kr::ps::base::RuleView rule;

    data() = default;

    data(tyr::formalism::ActionRowView action_, runir::kr::ps::base::RuleView rule_) noexcept : action(action_), rule(rule_) {}

    static constexpr bool copyable() noexcept
    {
        static_assert(std::is_trivially_copyable_v<data>, "a copyable record is trivially copyable");
        return true;
    }

    static constexpr auto fields() noexcept { return urd::Fields<urd::field<"action", &data::action>, urd::field<"rule", &data::rule>> {}; }

    static constexpr auto identifying_members() noexcept { return urd::members_of(fields()); }

    static constexpr auto cista_members() noexcept { return urd::Members<&data::action, &data::rule> {}; }
};
}  // namespace urd

namespace runir::kr::ps::base
{

/// The states a sketch's proof search reached and the transitions it took, each labelled by its action and rule: a
/// frozen graph, each state labelled too.
template<tyr::TaskKind Kind>
using SketchProofGraph = urd::Store<urd::graph<VertexIndex, urd::data<SketchProofEdgeLabel>, urd::data<SketchProofVertexLabel<Kind>>>, urd::frozen>;
// The concepts it is bound and searched by (urd/graphs.hpp): a renamed member is a compile error, not a lost feature.
static_assert(urd::incidence_graph<SketchProofGraph<tyr::GroundTag>> && urd::edge_list_graph<SketchProofGraph<tyr::LiftedTag>>);

}  // namespace runir::kr::ps::base

#endif
