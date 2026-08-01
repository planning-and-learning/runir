#include "bindings.hpp"

#include <nanobind/stl/string.h>
#include <runir/kr/dl/cnf_grammar/constructor_repository.hpp>
#include <runir/kr/dl/cnf_grammar/formatter.hpp>
#include <runir/kr/dl/cnf_grammar/non_terminal_view.hpp>
#include <yggdrasil/python/bindings.hpp>

namespace runir::kr::dl::base
{
namespace
{

template<CategoryTag Category>
void bind_non_terminal_data(nb::module_& m, const char* name)
{
    using Data = ygg::Data<runir::kr::dl::cnf_grammar::NonTerminal<runir::kr::BaseFamilyTag, Category>>;
    auto cls = nb::class_<Data>(m, name).def(nb::init<>()).def_rw("index", &Data::index).def_rw("name", &Data::name);
    ygg::add_comparison(cls);
}

template<CategoryTag Category>
void bind_non_terminal_view(nb::module_& m, const char* name)
{
    using Type = runir::kr::dl::cnf_grammar::NonTerminal<runir::kr::BaseFamilyTag, Category>;
    using View = ygg::View<ygg::Index<Type>, runir::kr::dl::cnf_grammar::ConstructorRepositoryFor<runir::kr::BaseFamilyTag>>;
    auto cls = nb::class_<View>(m, name).def("get_index", &View::get_index).def("get_name", &View::get_name);
    ygg::add_print(cls);
    ygg::add_comparison(cls);
    ygg::add_hash(cls);
}

}  // namespace

void bind_cnf_grammar_non_terminal(nb::module_& m)
{
    using Concept = runir::kr::dl::cnf_grammar::NonTerminal<runir::kr::BaseFamilyTag, ConceptTag>;
    using Role = runir::kr::dl::cnf_grammar::NonTerminal<runir::kr::BaseFamilyTag, RoleTag>;
    using Boolean = runir::kr::dl::cnf_grammar::NonTerminal<runir::kr::BaseFamilyTag, BooleanTag>;
    using Numerical = runir::kr::dl::cnf_grammar::NonTerminal<runir::kr::BaseFamilyTag, NumericalTag>;

    ygg::bind_index<ygg::Index<Concept>>(m, "ConceptNonTerminalIndex");
    ygg::bind_index<ygg::Index<Role>>(m, "RoleNonTerminalIndex");
    ygg::bind_index<ygg::Index<Boolean>>(m, "BooleanNonTerminalIndex");
    ygg::bind_index<ygg::Index<Numerical>>(m, "NumericalNonTerminalIndex");

    bind_non_terminal_data<ConceptTag>(m, "ConceptNonTerminalData");
    bind_non_terminal_data<RoleTag>(m, "RoleNonTerminalData");
    bind_non_terminal_data<BooleanTag>(m, "BooleanNonTerminalData");
    bind_non_terminal_data<NumericalTag>(m, "NumericalNonTerminalData");

    bind_non_terminal_view<ConceptTag>(m, "ConceptNonTerminal");
    bind_non_terminal_view<RoleTag>(m, "RoleNonTerminal");
    bind_non_terminal_view<BooleanTag>(m, "BooleanNonTerminal");
    bind_non_terminal_view<NumericalTag>(m, "NumericalNonTerminal");
}

}  // namespace runir::kr::dl::base
