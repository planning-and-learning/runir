#include "module.hpp"

#include "bindings.hpp"

namespace runir::kr::dl::base
{

void bind_cnf_grammar_module_definitions(nb::module_& m)
{
    bind_cnf_grammar_concept(m);
    bind_cnf_grammar_role(m);
    bind_cnf_grammar_boolean(m);
    bind_cnf_grammar_numerical(m);
    bind_cnf_grammar_constructor(m);
    bind_cnf_grammar_non_terminal(m);
    bind_cnf_grammar_constructor_repository(m);
    bind_cnf_grammar_grammar(m);
    bind_cnf_grammar_translate(m);
    bind_cnf_grammar_generate(m);
}

}  // namespace runir::kr::dl::base
