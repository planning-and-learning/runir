#ifndef PYRUNIR_KR_DL_NON_TERMINAL_BINDINGS_HPP_
#define PYRUNIR_KR_DL_NON_TERMINAL_BINDINGS_HPP_

#include "module.hpp"
#include "pyrunir/module.hpp"

#include <runir/kr/dl/declarations.hpp>
#include <urd/python/bindings.hpp>
#include <yggdrasil/python/bindings.hpp>

namespace runir::kr::dl::python
{

// The nonterminals of one grammar (its NonTerminal and ConstructorRepository), bound under the same Python names in
// each grammar's module: <Category>NonTerminalIndex, <Category>NonTerminalData and the view <Category>NonTerminal.

template<template<CategoryTag> typename NonTerminal, CategoryTag Category>
void bind_non_terminal_data(nb::module_& m, const char* name)
{
    using Data = urd::data<NonTerminal<Category>>;
    auto cls = nb::class_<Data>(m, name)
                   .def(nb::init<>())
                   .def(nb::init<urd::flat_string>(), nb::arg("name"))
                   .def_rw("index", &Data::index)
                   .def_rw("name", &Data::name);
    urd::python::add_comparison(cls);
}

template<template<CategoryTag> typename NonTerminal, typename Repository, CategoryTag Category>
void bind_non_terminal_view(nb::module_& m, const char* name)
{
    using View = urd::index_view<NonTerminal<Category>, Repository>;
    auto cls = nb::class_<View>(m, name).def("index", &View::index).def("get_name", &View::get_name);
    ygg::add_print(cls);
    urd::python::add_comparison(cls);
    urd::python::add_hash(cls);
}

/// The nonterminals of every constructor category.
template<template<CategoryTag> typename NonTerminal, typename Repository>
void bind_non_terminals(nb::module_& m)
{
    urd::python::bind_index<urd::index<NonTerminal<ConceptTag>>>(m, "ConceptNonTerminalIndex");
    urd::python::bind_index<urd::index<NonTerminal<RoleTag>>>(m, "RoleNonTerminalIndex");
    urd::python::bind_index<urd::index<NonTerminal<BooleanTag>>>(m, "BooleanNonTerminalIndex");
    urd::python::bind_index<urd::index<NonTerminal<NumericalTag>>>(m, "NumericalNonTerminalIndex");

    bind_non_terminal_data<NonTerminal, ConceptTag>(m, "ConceptNonTerminalData");
    bind_non_terminal_data<NonTerminal, RoleTag>(m, "RoleNonTerminalData");
    bind_non_terminal_data<NonTerminal, BooleanTag>(m, "BooleanNonTerminalData");
    bind_non_terminal_data<NonTerminal, NumericalTag>(m, "NumericalNonTerminalData");

    bind_non_terminal_view<NonTerminal, Repository, ConceptTag>(m, "ConceptNonTerminal");
    bind_non_terminal_view<NonTerminal, Repository, RoleTag>(m, "RoleNonTerminal");
    bind_non_terminal_view<NonTerminal, Repository, BooleanTag>(m, "BooleanNonTerminal");
    bind_non_terminal_view<NonTerminal, Repository, NumericalTag>(m, "NumericalNonTerminal");
}

}  // namespace runir::kr::dl::python

#endif
