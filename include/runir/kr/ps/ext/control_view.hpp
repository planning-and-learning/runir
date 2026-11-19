#ifndef RUNIR_KR_PS_EXT_CONTROL_VIEW_HPP_
#define RUNIR_KR_PS_EXT_CONTROL_VIEW_HPP_

#include "runir/kr/ps/ext/execution_data.hpp"

#include <urd/elements/view.hpp>

/// The views of the control records (no task kind): declared before any builder or view names one of their children.

/// Getters shared by the index, data and builder views of a module state (an activation): each child as its record or
/// builder holds it (a builder's: the variant of the views of its forms).
template<typename V>
struct urd::view_mixin<runir::kr::ps::ext::ModuleState, V>
{
private:
    const V& self() const noexcept { return static_cast<const V&>(*this); }

public:
    auto get_mod() const noexcept { return self().template member<&urd::record_of<V>::mod>(); }

    auto get_memory_state() const noexcept { return self().template member<&urd::record_of<V>::memory_state>(); }

    auto get_registers() const noexcept { return self().template member<&urd::record_of<V>::registers>(); }

    auto get_arguments() const noexcept { return self().template member<&urd::record_of<V>::arguments>(); }
};

/// Getters shared by the views of a call stack frame: the suspended activation (its memory state the one to return to)
/// and its caller.
template<typename V>
struct urd::view_mixin<runir::kr::ps::ext::CallStack, V>
{
private:
    const V& self() const noexcept { return static_cast<const V&>(*this); }

public:
    auto get_module_state() const noexcept { return self().template member<&urd::record_of<V>::module_state>(); }

    auto get_caller() const { return self().template member<&urd::record_of<V>::caller>(); }
};

#endif
