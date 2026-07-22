#include <concepts>
#include <runir/kr/ps/ext/execution_data.hpp>
#include <runir/kr/ps/ext/execution_index.hpp>
#include <runir/kr/ps/ext/execution_repository.hpp>
#include <runir/kr/ps/ext/execution_view.hpp>
#include <tyr/planning/ground/state_repository.hpp>
#include <tyr/planning/lifted/state_repository.hpp>

namespace runir::tests
{

namespace
{

template<typename Entity, tyr::planning::TaskKind Kind>
using View = ygg::View<ygg::Index<Entity>, kr::ps::ext::ExecutionRepository<Kind>>;

template<typename Entity, typename Kind>
concept IndexedDataView = tyr::planning::TaskKind<Kind> && std::constructible_from<ygg::Index<Entity>, ygg::uint_t> && std::totally_ordered<ygg::Index<Entity>>
                          && std::totally_ordered<ygg::Data<Entity>> && std::totally_ordered<View<Entity, Kind>>;

template<typename Kind>
concept RegisterValuesContract = tyr::planning::TaskKind<Kind> && IndexedDataView<kr::ps::ext::RegisterValues, Kind>
                                 && std::same_as<View<kr::ps::ext::RegisterValues, Kind>, kr::ps::ext::RegisterValuesView<Kind>>
                                 && requires(ygg::Data<kr::ps::ext::RegisterValues>& data, const View<kr::ps::ext::RegisterValues, Kind>& view) {
                                        data.index;
                                        data.concept_values;
                                        data.role_values;
                                        data.clear();
                                        view.get_index();
                                        view.get_concept_values();
                                        view.get_role_values();
                                    };

template<typename Kind>
concept CallArgumentsContract = tyr::planning::TaskKind<Kind> && IndexedDataView<kr::ps::ext::CallArguments, Kind>
                                && std::same_as<View<kr::ps::ext::CallArguments, Kind>, kr::ps::ext::CallArgumentsView<Kind>>
                                && requires(ygg::Data<kr::ps::ext::CallArguments>& data, const View<kr::ps::ext::CallArguments, Kind>& view) {
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
concept CallStackContract = tyr::planning::TaskKind<Kind> && IndexedDataView<kr::ps::ext::CallStack, Kind>
                            && std::same_as<View<kr::ps::ext::CallStack, Kind>, kr::ps::ext::CallStackView<Kind>>
                            && requires(ygg::Data<kr::ps::ext::CallStack>& data, const View<kr::ps::ext::CallStack, Kind>& view) {
                                   data.index;
                                   data.module;
                                   data.memory_state;
                                   data.registers;
                                   data.arguments;
                                   data.caller;
                                   data.clear();
                                   view.get_index();
                                   view.get_module();
                                   view.get_memory_state();
                                   view.get_registers();
                                   view.get_arguments();
                                   view.get_caller();
                               };

template<tyr::planning::TaskKind Kind>
using ExecutionState = kr::ps::ext::ExecutionState<Kind>;

template<typename Kind>
concept ExecutionStateContract = tyr::planning::TaskKind<Kind> && IndexedDataView<ExecutionState<Kind>, Kind>
                                 && std::same_as<View<ExecutionState<Kind>, Kind>, kr::ps::ext::ExecutionStateView<Kind>>
                                 && requires(ygg::Data<ExecutionState<Kind>>& data, const View<ExecutionState<Kind>, Kind>& view) {
                                        data.index;
                                        data.program;
                                        data.state;
                                        data.call_stack;
                                        data.phase;
                                        data.clear();
                                        view.get_index();
                                        view.get_program();
                                        view.get_state();
                                        view.get_call_stack();
                                        view.get_phase();
                                    };

template<tyr::planning::TaskKind Kind>
consteval bool execution_contracts()
{
    return RegisterValuesContract<Kind> && CallArgumentsContract<Kind> && CallStackContract<Kind> && ExecutionStateContract<Kind>;
}

static_assert(execution_contracts<tyr::planning::GroundTag>());
static_assert(execution_contracts<tyr::planning::LiftedTag>());

}  // namespace

}
