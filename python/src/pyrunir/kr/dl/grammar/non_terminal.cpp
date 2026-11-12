#include "../non_terminal_bindings.hpp"
#include "bindings.hpp"

#include <runir/kr/dl/grammar/constructor_repository.hpp>
#include <runir/kr/dl/grammar/formatter.hpp>
#include <runir/kr/dl/grammar/non_terminal_view.hpp>

namespace runir::kr::dl
{

void bind_grammar_non_terminal(nb::module_& m) { python::bind_non_terminals<grammar::NonTerminal, grammar::ConstructorRepository>(m); }

}  // namespace runir::kr::dl
