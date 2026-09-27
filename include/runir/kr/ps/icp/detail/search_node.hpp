#ifndef RUNIR_KR_PS_ICP_DETAIL_SEARCH_NODE_HPP_
#define RUNIR_KR_PS_ICP_DETAIL_SEARCH_NODE_HPP_

#include "runir/kr/ps/icp/execution_view.hpp"
#include "runir/kr/ps/icp/rule_variant_view.hpp"

#include <cstddef>
#include <optional>
#include <tyr/formalism/binding_view.hpp>

namespace runir::kr::ps::icp::detail
{

template<tyr::TaskKind Kind>
struct SearchNode
{
    ProgramStateView<Kind> state;
    std::optional<std::size_t> parent;
    std::optional<tyr::formalism::planning::ActionBindingView> action;
    bool is_goal = false;
    bool is_unsolvable = false;
    bool is_deadend = false;
    bool is_open = false;
};

struct Predecessor
{
    std::size_t source;
    std::size_t target;
    std::optional<tyr::formalism::planning::ActionBindingView> action;
    std::optional<RuleVariantView> rule;
};

}  // namespace runir::kr::ps::icp::detail

#endif
