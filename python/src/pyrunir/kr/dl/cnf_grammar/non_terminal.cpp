#include "../non_terminal_bindings.hpp"
#include "bindings.hpp"

#include <runir/kr/dl/cnf_grammar/constructor_repository.hpp>
#include <runir/kr/dl/cnf_grammar/formatter.hpp>
#include <runir/kr/dl/cnf_grammar/non_terminal_view.hpp>

namespace runir::kr::dl
{

void bind_cnf_grammar_non_terminal(nb::module_& m) { python::bind_non_terminals<cnf_grammar::NonTerminal, cnf_grammar::ConstructorRepository>(m); }

}  // namespace runir::kr::dl
