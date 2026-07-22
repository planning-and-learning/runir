#include <concepts>
#include <runir/kr/ps/ext/module_data.hpp>
#include <runir/kr/ps/ext/module_index.hpp>
#include <runir/kr/ps/ext/module_view.hpp>
#include <runir/kr/ps/ext/repository.hpp>

namespace runir::tests
{

using Entity = kr::ps::ext::Module;
using Index = ygg::Index<Entity>;
using Data = ygg::Data<Entity>;
using View = ygg::View<Index, kr::ps::ext::Repository>;

static_assert(std::constructible_from<Index, ygg::uint_t>);
static_assert(std::totally_ordered<Index>);
static_assert(std::totally_ordered<Data>);
static_assert(std::totally_ordered<View>);
static_assert(std::same_as<View, kr::ps::ext::ModuleView>);
static_assert(std::same_as<kr::ps::ext::ConceptRegisterView, ygg::View<ygg::Index<kr::dl::Register<kr::dl::ConceptTag>>, kr::dl::ExtConstructorRepository>>);
static_assert(std::same_as<kr::ps::ext::RoleRegisterView, ygg::View<ygg::Index<kr::dl::Register<kr::dl::RoleTag>>, kr::dl::ExtConstructorRepository>>);
static_assert(requires(Data& data) {
    data.index;
    data.symbol;
    data.concept_arguments;
    data.role_arguments;
    data.boolean_arguments;
    data.numerical_arguments;
    data.concept_registers;
    data.role_registers;
    data.concept_features;
    data.role_features;
    data.boolean_features;
    data.numerical_features;
    data.entry_memory_state;
    data.memory_states;
    data.memory_transitions;
    data.clear();
});
static_assert(requires(const View& view) {
    view.get_index();
    view.get_symbol();
    view.get_name();
    view.template get_arguments<kr::dl::ConceptTag>();
    view.template get_arguments<kr::dl::RoleTag>();
    view.template get_arguments<kr::dl::BooleanTag>();
    view.template get_arguments<kr::dl::NumericalTag>();
    view.template get_registers<kr::dl::ConceptTag>();
    view.template get_registers<kr::dl::RoleTag>();
    view.template get_features<kr::dl::ConceptTag>();
    view.template get_features<kr::dl::RoleTag>();
    view.template get_features<kr::ps::dl::BooleanFeature>();
    view.template get_features<kr::ps::dl::NumericalFeature>();
    view.get_entry_memory_state();
    view.get_memory_states();
    view.get_memory_transitions();
});

}
