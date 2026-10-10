#include "runir/kr/dl/semantics/declarations.hpp"
#include "runir/kr/ps/ext/execution_declarations.hpp"

#include <concepts>
#include <runir/kr/ps/ext/execution_data.hpp>
#include <runir/kr/ps/ext/execution_repository.hpp>
#include <runir/kr/ps/ext/execution_view.hpp>
#include <runir/kr/ps/icp/execution_view.hpp>
#include <tyr/planning/ground/state_repository.hpp>
#include <tyr/planning/lifted/state_repository.hpp>
#include <utility>
#include <yggdrasil/core/types.hpp>

namespace runir::tests
{

namespace
{

template<typename Repository, typename Entity>
constexpr bool flat_view_is_nothrow = noexcept(ygg::make_view(std::declval<ygg::Index<Entity>>(), std::declval<const Repository&>()));

template<typename Repository, typename Entity>
constexpr bool indexed_access_is_nothrow = noexcept(std::declval<const Repository&>()[std::declval<ygg::Index<Entity>>()]);

using Constructors = kr::dl::ExtConstructorRepository;
using Register = kr::dl::Register<kr::dl::ConceptTag>;
using Denotations = kr::dl::semantics::DenotationRepository;
using Registers = kr::dl::semantics::RegisterValues;
static_assert(flat_view_is_nothrow<Constructors, Register>);
static_assert(indexed_access_is_nothrow<Constructors, Register>);
static_assert(flat_view_is_nothrow<Denotations, Registers>);
static_assert(indexed_access_is_nothrow<Denotations, Registers>);
static_assert(flat_view_is_nothrow<kr::ps::ext::Repository, kr::ps::ext::MemoryState>);
static_assert(indexed_access_is_nothrow<kr::ps::ext::Repository, kr::ps::ext::MemoryState>);
static_assert(flat_view_is_nothrow<kr::ps::ext::ExecutionRepository<tyr::GroundTag>, kr::ps::ext::ModuleState<tyr::GroundTag>>);
static_assert(!indexed_access_is_nothrow<kr::ps::ext::ExecutionRepository<tyr::GroundTag>, kr::ps::ext::ModuleState<tyr::GroundTag>>);
static_assert(flat_view_is_nothrow<kr::ps::icp::ExecutionRepository<tyr::GroundTag>, kr::ps::icp::Histories>);
static_assert(!indexed_access_is_nothrow<kr::ps::icp::ExecutionRepository<tyr::GroundTag>, kr::ps::icp::Histories>);

template<typename Entity, tyr::TaskKind Kind>
using View = ygg::View<ygg::Index<Entity>, kr::ps::ext::ExecutionRepository<Kind>>;

template<typename Entity, typename Kind>
concept IndexedDataView = tyr::TaskKind<Kind> && std::constructible_from<ygg::Index<Entity>, ygg::uint_t> && std::totally_ordered<ygg::Index<Entity>>
                          && std::totally_ordered<ygg::Data<Entity>> && std::totally_ordered<View<Entity, Kind>>;

template<typename SharedView>
concept RegisterValuesContract =
    std::same_as<SharedView, kr::dl::semantics::RegisterValuesView> && std::totally_ordered<ygg::Index<kr::dl::semantics::RegisterValues>>
    && std::totally_ordered<ygg::Data<kr::dl::semantics::RegisterValues>> && std::totally_ordered<SharedView>
    && requires(ygg::Data<kr::dl::semantics::RegisterValues>& data, const SharedView& view) {
           data.index;
           data.concept_values;
           data.role_values;
           data.clear();
           view.get_index();
           view.template get<kr::dl::ConceptTag>();
           view.template get<kr::dl::RoleTag>();
       };

template<typename SharedView>
concept CallArgumentsContract =
    std::same_as<SharedView, kr::dl::semantics::CallArgumentsView> && std::totally_ordered<ygg::Index<kr::dl::semantics::CallArguments>>
    && std::totally_ordered<ygg::Data<kr::dl::semantics::CallArguments>> && std::totally_ordered<SharedView>
    && requires(ygg::Data<kr::dl::semantics::CallArguments>& data, const SharedView& view) {
           data.index;
           data.concept_arguments;
           data.role_arguments;
           data.boolean_arguments;
           data.numerical_arguments;
           data.clear();
           view.get_index();
           view.template get<kr::dl::ConceptTag>();
           view.template get<kr::dl::RoleTag>();
           view.template get<kr::dl::BooleanTag>();
           view.template get<kr::dl::NumericalTag>();
       };

template<typename Kind>
concept ModuleStateContract = tyr::TaskKind<Kind> && IndexedDataView<kr::ps::ext::ModuleState<Kind>, Kind>
                              && std::same_as<View<kr::ps::ext::ModuleState<Kind>, Kind>, kr::ps::ext::ModuleStateView<Kind>>
                              && requires(ygg::Data<kr::ps::ext::ModuleState<Kind>>& data, const kr::ps::ext::ModuleStateView<Kind>& view) {
                                     data.index;
                                     data.state;
                                     data.module_;
                                     data.memory_state;
                                     data.registers;
                                     data.arguments;
                                     data.clear();
                                     view.get_index();
                                     view.get_state();
                                     view.get_module();
                                     view.get_memory_state();
                                     view.get_registers();
                                     view.get_arguments();
                                 };

template<typename Kind>
concept CallStackContract =
    tyr::TaskKind<Kind> && IndexedDataView<kr::ps::ext::CallStack, Kind> && std::same_as<View<kr::ps::ext::CallStack, Kind>, kr::ps::ext::CallStackView<Kind>>
    && requires(ygg::Data<kr::ps::ext::CallStack>& data, const kr::ps::ext::CallStackView<Kind>& view) {
           data.index;
           data.module_;
           data.return_memory_state;
           data.registers;
           data.arguments;
           data.caller;
           data.clear();
           view.get_index();
           view.get_module();
           view.get_return_memory_state();
           view.get_registers();
           view.get_arguments();
           view.get_caller();
       };

template<tyr::TaskKind Kind>
using ProgramState = kr::ps::ext::ProgramState<Kind>;

template<typename Kind>
concept ProgramStateContract =
    tyr::TaskKind<Kind> && IndexedDataView<ProgramState<Kind>, Kind> && std::same_as<View<ProgramState<Kind>, Kind>, kr::ps::ext::ProgramStateView<Kind>>
    && requires(ygg::Data<ProgramState<Kind>>& data, const View<ProgramState<Kind>, Kind>& view) {
           data.index;
           data.program;
           data.call_stack;
           data.module_state;
           data.clear();
           view.get_index();
           view.get_program();
           view.get_state();
           view.get_call_stack();
           view.get_module_state();
       };

template<tyr::TaskKind Kind>
consteval bool execution_contracts()
{
    return ModuleStateContract<Kind> && CallStackContract<Kind> && ProgramStateContract<Kind>;
}

static_assert(RegisterValuesContract<kr::dl::semantics::RegisterValuesView>);
static_assert(CallArgumentsContract<kr::dl::semantics::CallArgumentsView>);
static_assert(execution_contracts<tyr::GroundTag>());
static_assert(execution_contracts<tyr::LiftedTag>());

}  // namespace

}
