#ifndef PYRUNIR_KR_DL_NON_TERMINAL_BINDINGS_HPP_
#define PYRUNIR_KR_DL_NON_TERMINAL_BINDINGS_HPP_

#include "module.hpp"
#include "pyrunir/kr/dl/conversions.hpp"
#include "pyrunir/module.hpp"

#include <runir/kr/dl/declarations.hpp>
#include <urd/python/bindings.hpp>

namespace runir::kr::dl::python
{

// The nonterminals of one grammar (its NonTerminal and ConstructorRepository), bound by bind_data and bind_view in each
// grammar's module: NonTerminalConceptIndex, NonTerminalConceptData and the view NonTerminalConcept, ...

/// The views of the nonterminals of every constructor category (their records are bound first, bind_data).
template<template<CategoryTag> typename NonTerminal, typename Repository>
void bind_non_terminal_views(nb::module_& m)
{
    urd::python::bind_view<urd::index_view<NonTerminal<ConceptTag>, Repository>>(m);
    urd::python::bind_builder<NonTerminal<ConceptTag>, Repository>(m);
    urd::python::bind_view<urd::index_view<NonTerminal<RoleTag>, Repository>>(m);
    urd::python::bind_builder<NonTerminal<RoleTag>, Repository>(m);
    urd::python::bind_view<urd::index_view<NonTerminal<BooleanTag>, Repository>>(m);
    urd::python::bind_builder<NonTerminal<BooleanTag>, Repository>(m);
    urd::python::bind_view<urd::index_view<NonTerminal<NumericalTag>, Repository>>(m);
    urd::python::bind_builder<NonTerminal<NumericalTag>, Repository>(m);
}

}  // namespace runir::kr::dl::python

#endif
