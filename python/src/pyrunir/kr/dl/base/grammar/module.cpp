#include "module.hpp"

#include "bindings.hpp"

namespace runir::kr::dl::base
{

void bind_grammar_module_definitions(nb::module_& m)
{
    bind_grammar_concept(m);
    bind_grammar_role(m);
    bind_grammar_boolean(m);
    bind_grammar_numerical(m);
    bind_grammar_constructor(m);
    bind_grammar_non_terminal(m);
    bind_grammar_constructor_or_non_terminal(m);
    bind_grammar_constructor_repository(m);
    bind_grammar_grammar(m);
    bind_grammar_factory(m);
}

}  // namespace runir::kr::dl::base
