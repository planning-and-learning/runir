#include "runir/kr/dl/semantics/declarations.hpp"
#include "runir/kr/ps/ext/execution_declarations.hpp"

#include <concepts>
#include <runir/kr/ps/execution_repository.hpp>
#include <runir/kr/ps/ext/execution_data.hpp>
#include <runir/kr/ps/ext/execution_view.hpp>
#include <runir/kr/ps/ext/repository.hpp>
#include <runir/kr/ps/icp/execution_view.hpp>
#include <tyr/planning/state_builder.hpp>
#include <tyr/planning/state_repository.hpp>
#include <urd/elements/flat.hpp>
#include <utility>
#include <yggdrasil/core/config.hpp>

namespace runir::tests::ps_ext_execution
{

namespace
{

template<typename Repository, typename Entity>
constexpr bool flat_view_is_nothrow = noexcept(std::declval<const Repository&>().view(std::declval<urd::index<Entity>>()));

template<typename Repository, typename Entity>
constexpr bool indexed_access_is_nothrow = noexcept(std::declval<const Repository&>()[std::declval<urd::index<Entity>>()]);

using Constructors = kr::dl::ConstructorRepository;
using Register = kr::dl::Register<kr::dl::ConceptTag>;
using Denotations = kr::dl::semantics::DenotationRepository;
using Registers = kr::dl::semantics::RegisterValues;
static_assert(flat_view_is_nothrow<Constructors, Register>);
static_assert(flat_view_is_nothrow<Denotations, Registers>);
static_assert(flat_view_is_nothrow<kr::ps::ext::Repository, kr::ps::MemoryState<kr::ExtFamilyTag>>);
static_assert(flat_view_is_nothrow<kr::ps::ext::ControlRepository, kr::ps::ext::ModuleState>);
static_assert(!indexed_access_is_nothrow<kr::ps::ext::ControlRepository, kr::ps::ext::ModuleState>);
static_assert(flat_view_is_nothrow<kr::ps::icp::ControlRepository, kr::ps::icp::Histories>);
static_assert(!indexed_access_is_nothrow<kr::ps::icp::ControlRepository, kr::ps::icp::Histories>);
static_assert(flat_view_is_nothrow<kr::ps::ExecutionRepository<kr::ExtFamilyTag, tyr::GroundTag>, kr::ps::ext::ProgramState<tyr::GroundTag>>);

template<typename Entity, tyr::TaskKind Kind>
using View = urd::index_view<Entity, kr::ps::ExecutionRepository<kr::ExtFamilyTag, Kind>>;

template<typename Entity, typename Kind>
concept IndexedDataView = tyr::TaskKind<Kind> && std::constructible_from<urd::index<Entity>, ygg::uint_t> && std::totally_ordered<urd::index<Entity>>
                          && std::totally_ordered<urd::data<Entity>> && std::totally_ordered<View<Entity, Kind>>;

template<typename SharedView>
concept RegisterValuesContract =
    std::same_as<SharedView, kr::dl::semantics::RegisterValuesView> && std::totally_ordered<urd::index<kr::dl::semantics::RegisterValues>>
    && std::totally_ordered<urd::data<kr::dl::semantics::RegisterValues>> && std::totally_ordered<SharedView>
    && requires(urd::data<kr::dl::semantics::RegisterValues>& data, const SharedView& view) {
           data.index;
           data.concept_values;
           data.role_values;
           view.index();
           view.template values<kr::dl::ConceptTag>();
           view.template values<kr::dl::RoleTag>();
       };

template<typename SharedView>
concept CallArgumentsContract =
    std::same_as<SharedView, kr::dl::semantics::CallArgumentsView> && std::totally_ordered<urd::index<kr::dl::semantics::CallArguments>>
    && std::totally_ordered<urd::data<kr::dl::semantics::CallArguments>> && std::totally_ordered<SharedView>
    && requires(urd::data<kr::dl::semantics::CallArguments>& data, const SharedView& view) {
           data.index;
           data.concept_arguments;
           data.role_arguments;
           data.boolean_arguments;
           data.numerical_arguments;
           view.index();
           view.template arguments<kr::dl::ConceptTag>();
           view.template arguments<kr::dl::RoleTag>();
           view.template arguments<kr::dl::BooleanTag>();
           view.template arguments<kr::dl::NumericalTag>();
       };

/// A control record's view: of the task's control repository, whatever its kind.
template<typename Entity, typename SharedView>
concept IndexedControlView =
    std::same_as<SharedView, urd::index_view<Entity, kr::ps::ext::ControlRepository>> && std::constructible_from<urd::index<Entity>, ygg::uint_t>
    && std::totally_ordered<urd::index<Entity>> && std::totally_ordered<urd::data<Entity>> && std::totally_ordered<SharedView>;

template<typename SharedView>
concept ModuleStateContract =
    IndexedControlView<kr::ps::ext::ModuleState, SharedView> && requires(urd::data<kr::ps::ext::ModuleState>& data, const SharedView& view) {
        data.index;
        data.mod;
        data.memory_state;
        data.registers;
        data.arguments;
        view.index();
        view.get_mod();
        view.get_memory_state();
        view.get_registers();
        view.get_arguments();
    };

template<typename SharedView>
concept CallStackContract =
    IndexedControlView<kr::ps::ext::CallStack, SharedView> && requires(urd::data<kr::ps::ext::CallStack>& data, const SharedView& view) {
        data.index;
        data.module_state;
        data.caller;
        view.index();
        view.get_module_state();
        view.get_caller();
    };

template<tyr::TaskKind Kind>
using ProgramState = kr::ps::ext::ProgramState<Kind>;

template<typename Kind>
concept ProgramStateContract =
    tyr::TaskKind<Kind> && IndexedDataView<ProgramState<Kind>, Kind> && std::same_as<View<ProgramState<Kind>, Kind>, kr::ps::ext::ProgramStateView<Kind>>
    && requires(urd::data<ProgramState<Kind>>& data, const View<ProgramState<Kind>, Kind>& view) {
           data.index;
           data.state;
           data.module_state;
           data.call_stack;
           view.index();
           view.get_state();
           view.get_module_state();
           view.get_call_stack();
       };

template<tyr::TaskKind Kind>
consteval bool execution_contracts()
{ return ProgramStateContract<Kind>; }

static_assert(RegisterValuesContract<kr::dl::semantics::RegisterValuesView>);
static_assert(CallArgumentsContract<kr::dl::semantics::CallArgumentsView>);
static_assert(ModuleStateContract<kr::ps::ext::ModuleStateView>);
static_assert(CallStackContract<kr::ps::ext::CallStackView>);
static_assert(execution_contracts<tyr::GroundTag>());
static_assert(execution_contracts<tyr::LiftedTag>());

}  // namespace

}
