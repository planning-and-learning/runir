#include "../non_terminal_bindings.hpp"
#include "bindings.hpp"

#include <runir/kr/dl/cnf_grammar/constructor_repository.hpp>
#include <runir/kr/dl/cnf_grammar/formatter.hpp>
#include <runir/kr/dl/cnf_grammar/non_terminal_view.hpp>

namespace runir::kr::dl
{

void bind_cnf_grammar_non_terminal(nb::module_& m)
{
    urd::python::bind_index<cnf_grammar::NonTerminal<ConceptTag>>(m);
    urd::python::bind_data<cnf_grammar::NonTerminal<ConceptTag>>(m);
    urd::python::bind_index<cnf_grammar::NonTerminal<RoleTag>>(m);
    urd::python::bind_data<cnf_grammar::NonTerminal<RoleTag>>(m);
    urd::python::bind_index<cnf_grammar::NonTerminal<BooleanTag>>(m);
    urd::python::bind_data<cnf_grammar::NonTerminal<BooleanTag>>(m);
    urd::python::bind_index<cnf_grammar::NonTerminal<NumericalTag>>(m);
    urd::python::bind_data<cnf_grammar::NonTerminal<NumericalTag>>(m);
    python::bind_non_terminal_views<cnf_grammar::NonTerminal, cnf_grammar::ConstructorRepository>(m);
}

}  // namespace runir::kr::dl
