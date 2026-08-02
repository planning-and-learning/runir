#include "bindings.hpp"

#include <runir/kr/dl/grammar/constructor_or_non_terminal_view.hpp>
#include <runir/kr/dl/grammar/constructor_repository.hpp>
#include <runir/kr/dl/grammar/formatter.hpp>
#include <yggdrasil/python/bindings.hpp>
#include <yggdrasil/python/type_casters.hpp>

namespace runir::kr::dl::base
{
namespace
{

template<CategoryTag Category>
void bind_choice_data(nb::module_& m, const char* name)
{
    using Data = ygg::Data<runir::kr::dl::grammar::ConstructorOrNonTerminal<runir::kr::BaseFamilyTag, Category>>;
    auto cls = nb::class_<Data>(m, name).def(nb::init<>()).def_rw("index", &Data::index).def_rw("value", &Data::value);
    ygg::add_comparison(cls);
}

template<CategoryTag Category>
void bind_choice_view(nb::module_& m, const char* name)
{
    using Type = runir::kr::dl::grammar::ConstructorOrNonTerminal<runir::kr::BaseFamilyTag, Category>;
    using View = ygg::View<ygg::Index<Type>, runir::kr::dl::grammar::ConstructorRepositoryFor<runir::kr::BaseFamilyTag>>;
    auto cls = nb::class_<View>(m, name).def("get_index", &View::get_index).def("get_variant", &View::get_variant);
    ygg::add_print(cls);
    ygg::add_comparison(cls);
    ygg::add_hash(cls);
}

}  // namespace

void bind_grammar_constructor_or_non_terminal(nb::module_& m)
{
    using Concept = runir::kr::dl::grammar::ConstructorOrNonTerminal<runir::kr::BaseFamilyTag, ConceptTag>;
    using Role = runir::kr::dl::grammar::ConstructorOrNonTerminal<runir::kr::BaseFamilyTag, RoleTag>;
    using Boolean = runir::kr::dl::grammar::ConstructorOrNonTerminal<runir::kr::BaseFamilyTag, BooleanTag>;
    using Numerical = runir::kr::dl::grammar::ConstructorOrNonTerminal<runir::kr::BaseFamilyTag, NumericalTag>;

    ygg::bind_index<ygg::Index<Concept>>(m, "ConceptConstructorOrNonTerminalIndex");
    ygg::bind_index<ygg::Index<Role>>(m, "RoleConstructorOrNonTerminalIndex");
    ygg::bind_index<ygg::Index<Boolean>>(m, "BooleanConstructorOrNonTerminalIndex");
    ygg::bind_index<ygg::Index<Numerical>>(m, "NumericalConstructorOrNonTerminalIndex");

    bind_choice_data<ConceptTag>(m, "ConceptConstructorOrNonTerminalData");
    bind_choice_data<RoleTag>(m, "RoleConstructorOrNonTerminalData");
    bind_choice_data<BooleanTag>(m, "BooleanConstructorOrNonTerminalData");
    bind_choice_data<NumericalTag>(m, "NumericalConstructorOrNonTerminalData");

    bind_choice_view<ConceptTag>(m, "ConceptConstructorOrNonTerminal");
    bind_choice_view<RoleTag>(m, "RoleConstructorOrNonTerminal");
    bind_choice_view<BooleanTag>(m, "BooleanConstructorOrNonTerminal");
    bind_choice_view<NumericalTag>(m, "NumericalConstructorOrNonTerminal");
}

}  // namespace runir::kr::dl::base
