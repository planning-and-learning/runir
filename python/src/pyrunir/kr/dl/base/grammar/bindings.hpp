#ifndef PYRUNIR_KR_DL_BASE_GRAMMAR_BINDINGS_HPP_
#define PYRUNIR_KR_DL_BASE_GRAMMAR_BINDINGS_HPP_

#include "module.hpp"

namespace runir::kr::dl::base
{

void bind_grammar_concept(nb::module_& m);
void bind_grammar_role(nb::module_& m);
void bind_grammar_boolean(nb::module_& m);
void bind_grammar_numerical(nb::module_& m);
void bind_grammar_constructor(nb::module_& m);
void bind_grammar_non_terminal(nb::module_& m);
void bind_grammar_constructor_or_non_terminal(nb::module_& m);
void bind_grammar_grammar(nb::module_& m);
void bind_grammar_constructor_repository(nb::module_& m);
void bind_grammar_factory(nb::module_& m);

}  // namespace runir::kr::dl::base

#endif
