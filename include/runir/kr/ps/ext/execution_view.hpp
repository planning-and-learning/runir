#ifndef RUNIR_KR_PS_EXT_EXECUTION_VIEW_HPP_
#define RUNIR_KR_PS_EXT_EXECUTION_VIEW_HPP_

#include "runir/kr/dl/semantics/call_arguments_view.hpp"
#include "runir/kr/dl/semantics/register_values_view.hpp"
#include "runir/kr/dl/semantics/register_values_view_concepts.hpp"
#include "runir/kr/ps/execution_repository.hpp"
#include "runir/kr/ps/ext/control_view.hpp"
#include "runir/kr/ps/ext/execution_builder.hpp"
#include "runir/kr/ps/ext/execution_data.hpp"
#include "runir/kr/ps/ext/program_view.hpp"
#include "runir/kr/ps/ext/repository.hpp"

#include <boost/mp11.hpp>
#include <optional>
#include <stdexcept>
#include <tuple>
#include <type_traits>
#include <tyr/formalism/object_view.hpp>
#include <tyr/planning/state_repository.hpp>
#include <urd/elements/flat.hpp>
#include <variant>
#include <yggdrasil/formalism/declarations.hpp>

namespace runir::kr::ps::ext::detail
{
/// A program state's planning state as its readers take it: a packed one (a record's view: interned, or held) unpacked
/// into its state view, a built one as its builder's view; a child's (a variant of the views of its forms) the variant
/// of those.
template<tyr::TaskKind Kind, urd::View V>
auto planning_state(const V& view)
{
    if constexpr (std::same_as<urd::record_of<V>, urd::data<tyr::planning::State<Kind>>>)
        return view.unpack();
    else
        return view;
}

template<tyr::TaskKind Kind, urd::View... Vs>
auto planning_state(const std::variant<Vs...>& views)
{
    using Read = boost::mp11::mp_unique<std::variant<decltype(planning_state<Kind>(std::declval<const Vs&>()))...>>;  // packed forms: one
    return std::visit([](const auto& view) { return Read(planning_state<Kind>(view)); }, views);
}
}  // namespace runir::kr::ps::ext::detail

/// Getters shared by the index, data and builder views of a program state, as for a module state. A builder may move
/// when search buffers grow; create a builder view only while its owner remains in place.
template<tyr::TaskKind Kind, typename V>
struct urd::view_mixin<runir::kr::ps::ext::ProgramState<Kind>, V>
{
private:
    const V& self() const noexcept { return static_cast<const V&>(*this); }

public:
    auto get_state() const { return runir::kr::ps::ext::detail::planning_state<Kind>(self().template member<&urd::record_of<V>::state>()); }

    auto get_module_state() const noexcept { return self().template member<&urd::record_of<V>::module_state>(); }

    auto get_call_stack() const { return self().template member<&urd::record_of<V>::call_stack>(); }
};

namespace runir::kr::ps::ext
{
namespace detail
{
template<typename R, typename Kind>
inline constexpr bool state_views = false;
template<typename Kind, typename... Vs>
inline constexpr bool state_views<std::variant<Vs...>, Kind> = (tyr::planning::StateViewConcept<Vs, Kind> && ...);

template<typename R>
inline constexpr bool optional_call_stack = false;
template<typename R>
inline constexpr bool optional_call_stack<std::optional<R>> = urd::ReadsAs<R, CallStack>;

/// Whether R reads a call stack that may be absent: an optional of a call stack's view or child.
template<typename R>
concept OptionalCallStack = optional_call_stack<std::remove_cvref_t<R>>;
}  // namespace detail

/// Whether R reads a planning state: a state view, or the variant of those a program state's builder gives.
template<typename R, typename Kind>
concept ReadsAsStateView = tyr::planning::StateViewConcept<R, Kind> || detail::state_views<std::remove_cvref_t<R>, Kind>;

/// A program state's reads, through any of its views (index, record, builder): its planning state, its activation and
/// its call stack, each a view of its kind or a builder's child of it, read through urd::visit_view.
template<typename S, typename Kind>
concept ProgramStateViewConcept = tyr::TaskKind<Kind> && requires(const std::remove_reference_t<S>& state) {
    { state.get_state() } -> ReadsAsStateView<Kind>;
    { state.get_module_state() } -> urd::ReadsAs<ModuleState>;
    { state.get_call_stack() } -> detail::OptionalCallStack;
    state.context();
};

}  // namespace runir::kr::ps::ext

#endif
