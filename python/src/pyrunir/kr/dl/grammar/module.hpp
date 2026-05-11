#ifndef PYRUNIR_KR_DL_GRAMMAR_MODULE_HPP_
#define PYRUNIR_KR_DL_GRAMMAR_MODULE_HPP_

#include "../module.hpp"

namespace runir::kr::dl
{

void bind_grammar_module_definitions(nb::module_& m);
void bind_grammar_indices(nb::module_& m);
void bind_grammar_datas(nb::module_& m);
void bind_grammar_views(nb::module_& m);
void bind_grammar_constructor_repository(nb::module_& m);
void bind_grammar_grammar(nb::module_& m);
void bind_grammar_factory(nb::module_& m);
void bind_grammar_parser(nb::module_& m);

}

#endif
