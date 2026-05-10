#ifndef PYRUNIR_KR_DL_CNF_GRAMMAR_MODULE_HPP_
#define PYRUNIR_KR_DL_CNF_GRAMMAR_MODULE_HPP_

#include "../module.hpp"

namespace runir::kr::dl
{

void bind_cnf_grammar_module_definitions(nb::module_& m);
void bind_cnf_grammar_indices(nb::module_& m);
void bind_cnf_grammar_datas(nb::module_& m);
void bind_cnf_grammar_views(nb::module_& m);
void bind_cnf_grammar_constructor_repository(nb::module_& m);
void bind_cnf_grammar_grammar(nb::module_& m);
void bind_cnf_grammar_translate(nb::module_& m);
void bind_cnf_grammar_generate(nb::module_& m);

}

#endif
