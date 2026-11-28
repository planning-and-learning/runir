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
#include <tyr/planning/states/state_repository.hpp>
#include <urd/elements/flat.hpp>
#include <variant>
#include <yggdrasil/formalism/declarations.hpp>

/// Getters shared by the index, data and builder views of a program state, as for a module state. A builder may move
/// when search buffers grow; create a builder view only while its owner remains in place.
template<tyr::TaskKind Kind, typename V>
struct urd::view_mixin<runir::kr::ps::ext::ProgramState<Kind>, V>
{
private:
    const V& self() const noexcept { return static_cast<const V&>(*this); }

public:
    auto state() const { return self().template member<&urd::record_of<V>::state>(); }

    auto module_state() const noexcept { return self().template member<&urd::record_of<V>::module_state>(); }

    auto call_stack() const { return self().template member<&urd::record_of<V>::call_stack>(); }
};

#endif
