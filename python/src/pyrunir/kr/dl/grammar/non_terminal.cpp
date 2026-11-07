#include "bindings.hpp"
#include "pyrunir/module.hpp"

#include <runir/kr/dl/grammar/constructor_repository.hpp>
#include <runir/kr/dl/grammar/formatter.hpp>
#include <runir/kr/dl/grammar/non_terminal_view.hpp>
#include <yggdrasil/python/bindings.hpp>

namespace runir::kr::dl
{
namespace
{

template<CategoryTag Category>
void bind_non_terminal_data(nb::module_& m, const char* name)
{
    using Data = urd::data<runir::kr::dl::grammar::NonTerminal<Category>>;
    auto cls = nb::class_<Data>(m, name)
                   .def(nb::init<>())
                   .def(nb::init<::cista::offset::string>(), nb::arg("name"))
                   .def_rw("index", &Data::index)
                   .def_rw("name", &Data::name);
    urd::python::add_comparison(cls);
}

template<CategoryTag Category>
void bind_non_terminal_view(nb::module_& m, const char* name)
{
    using Type = runir::kr::dl::grammar::NonTerminal<Category>;
    using View = urd::index_view<Type, runir::kr::dl::grammar::ConstructorRepository>;
    auto cls = nb::class_<View>(m, name).def("get_index", &View::index).def("get_name", &View::get_name);
    ygg::add_print(cls);
    urd::python::add_comparison(cls);
    urd::python::add_hash(cls);
}

}  // namespace

void bind_grammar_non_terminal(nb::module_& m)
{
    using Concept = runir::kr::dl::grammar::NonTerminal<ConceptTag>;
    using Role = runir::kr::dl::grammar::NonTerminal<RoleTag>;
    using Boolean = runir::kr::dl::grammar::NonTerminal<BooleanTag>;
    using Numerical = runir::kr::dl::grammar::NonTerminal<NumericalTag>;

    urd::python::bind_index<urd::index<Concept>>(m, "ConceptNonTerminalIndex");
    urd::python::bind_index<urd::index<Role>>(m, "RoleNonTerminalIndex");
    urd::python::bind_index<urd::index<Boolean>>(m, "BooleanNonTerminalIndex");
    urd::python::bind_index<urd::index<Numerical>>(m, "NumericalNonTerminalIndex");

    bind_non_terminal_data<ConceptTag>(m, "ConceptNonTerminalData");
    bind_non_terminal_data<RoleTag>(m, "RoleNonTerminalData");
    bind_non_terminal_data<BooleanTag>(m, "BooleanNonTerminalData");
    bind_non_terminal_data<NumericalTag>(m, "NumericalNonTerminalData");

    bind_non_terminal_view<ConceptTag>(m, "ConceptNonTerminal");
    bind_non_terminal_view<RoleTag>(m, "RoleNonTerminal");
    bind_non_terminal_view<BooleanTag>(m, "BooleanNonTerminal");
    bind_non_terminal_view<NumericalTag>(m, "NumericalNonTerminal");
}

}  // namespace runir::kr::dl
