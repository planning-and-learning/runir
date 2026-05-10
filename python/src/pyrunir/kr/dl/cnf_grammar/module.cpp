#include "module.hpp"

namespace runir::kr::dl
{

void bind_cnf_grammar_module_definitions(nb::module_& m)
{
    bind_cnf_grammar_indices(m);
    bind_cnf_grammar_datas(m);
    bind_cnf_grammar_views(m);
    bind_cnf_grammar_constructor_repository(m);
    bind_cnf_grammar_grammar(m);
    bind_cnf_grammar_translate(m);
    bind_cnf_grammar_generate(m);
}

}  // namespace runir::kr::dl
