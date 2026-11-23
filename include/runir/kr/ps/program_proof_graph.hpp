#ifndef RUNIR_KR_PS_PROGRAM_PROOF_GRAPH_HPP_
#define RUNIR_KR_PS_PROGRAM_PROOF_GRAPH_HPP_

#include "runir/datasets/graph_types.hpp"
#include "runir/kr/ps/declarations.hpp"

#include <cista/containers/optional.h>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <tyr/formalism/declarations.hpp>
#include <tyr/formalism/repository.hpp>
#include <tyr/planning/declarations.hpp>
#include <urd/elements/concepts.hpp>
#include <urd/elements/flat.hpp>
#include <urd/repr/graphs/csr_graph.hpp>
#include <utility>
#include <yggdrasil/formalism/row_view.hpp>

namespace urd
{
template<runir::kr::ps::ProgramFamilyTag Family, tyr::TaskKind Kind>
struct data<runir::kr::ps::ProgramProofVertexLabel<Family, Kind>>
{
    runir::kr::ps::ProgramStateView<Family, Kind> program_state;
    bool is_initial {};
    bool is_goal {};
    bool is_alive {};
    bool is_unsolvable {};

    data() = default;

    data(runir::kr::ps::ProgramStateView<Family, Kind> program_state_, bool is_initial_, bool is_goal_, bool is_alive_, bool is_unsolvable_) noexcept :
        program_state(program_state_),
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
        return urd::Fields<urd::field<"program_state", &data::program_state>,
                           urd::field<"is_initial", &data::is_initial>,
                           urd::field<"is_goal", &data::is_goal>,
                           urd::field<"is_alive", &data::is_alive>,
                           urd::field<"is_unsolvable", &data::is_unsolvable>> {};
    }

    static constexpr auto identifying_members() noexcept { return urd::members_of(fields()); }

    static constexpr auto cista_members() noexcept
    { return urd::Members<&data::program_state, &data::is_initial, &data::is_goal, &data::is_alive, &data::is_unsolvable> {}; }
};

template<runir::kr::ps::ProgramFamilyTag Family>
struct data<runir::kr::ps::ProgramProofEdgeLabel<Family>>
{
    urd::flat_optional<tyr::formalism::ActionRowView> action;
    urd::flat_optional<runir::kr::ps::RuleView<Family>> rule;

    data() = default;

    data(urd::flat_optional<tyr::formalism::ActionRowView> action_, urd::flat_optional<runir::kr::ps::RuleView<Family>> rule_) noexcept :
        action(std::move(action_)),
        rule(std::move(rule_))
    {
    }

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

namespace runir::kr::ps
{

/// The program states a program's proof search reached and the transitions it took: a frozen graph, each labelled.
template<runir::kr::ps::ProgramFamilyTag Family, tyr::TaskKind Kind>
using ProgramProofGraph =
    urd::Store<urd::graph<VertexIndex, urd::data<ProgramProofEdgeLabel<Family>>, urd::data<ProgramProofVertexLabel<Family, Kind>>>, urd::frozen>;

}  // namespace runir::kr::ps

#endif
