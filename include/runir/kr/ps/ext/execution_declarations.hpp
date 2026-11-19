#ifndef RUNIR_KR_PS_EXT_EXECUTION_DECLARATIONS_HPP_
#define RUNIR_KR_PS_EXT_EXECUTION_DECLARATIONS_HPP_

#include "runir/kr/declarations.hpp"
#include "runir/kr/dl/semantics/declarations.hpp"
#include "runir/kr/ps/declarations.hpp"

#include <boost/mp11.hpp>
#include <tyr/planning/declarations.hpp>
#include <urd/elements/concepts.hpp>
#include <urd/elements/flat.hpp>
#include <urd/repr/slots/slots.hpp>
#include <urd/store.hpp>

namespace runir::kr::ps::ext
{

/// An activation of a module: its module, memory state, registers and arguments (no planning state), shared by the
/// program states that run in it.
struct ModuleState
{
    static constexpr auto name = "ModuleState";
};

/// A frame of the call stack: the suspended activation (its memory state the one to return to) and its own caller.
struct CallStack
{
    static constexpr auto name = "CallStack";
};

/// A program's state: its planning state, activation and call stack.
template<tyr::TaskKind Kind>
struct ProgramState
{
    static constexpr std::string_view name = urd::joined_name<"ProgramState", Kind::name>;
};

/// The records of a program's control, whatever the task's kind: activations and frames.
using ControlRepositoryTypes = boost::mp11::mp_list<ModuleState, CallStack>;

template<tyr::TaskKind Kind>
using ExecutionRepositoryTypes = boost::mp11::mp_list<ProgramState<Kind>>;

using ControlRepository = runir::kr::ps::ControlRepository<ExtFamilyTag>;

using ModuleStateView = urd::index_view<ModuleState, ControlRepository>;

using CallStackView = urd::index_view<CallStack, ControlRepository>;

template<tyr::TaskKind Kind>
using ProgramStateView = urd::index_view<ProgramState<Kind>, ExecutionRepository<ExtFamilyTag, Kind>>;

using ModuleStateBuilderView = urd::borrowed_view<ModuleState, ControlRepository, urd::builder<ModuleState, ControlRepository>>;

template<tyr::TaskKind Kind>
using ProgramStateBuilderView = urd::borrowed_view<ProgramState<Kind>,
                                                   ExecutionRepository<ExtFamilyTag, Kind>,
                                                   urd::builder<ProgramState<Kind>, ExecutionRepository<runir::kr::ExtFamilyTag, Kind>>>;

}  // namespace runir::kr::ps::ext

#endif
