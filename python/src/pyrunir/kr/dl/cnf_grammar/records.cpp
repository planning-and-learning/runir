#include "../record_bindings.hpp"
#include "bindings.hpp"

#include <runir/kr/dl/cnf_grammar/constructor_repository.hpp>
#include <runir/kr/dl/cnf_grammar/formatter.hpp>

namespace runir::kr::dl
{

void bind_cnf_grammar_records(nb::module_& m) { python::bind_records<cnf_grammar::CnfGrammarFamily, cnf_grammar::ConstructorRepository>(m); }

}  // namespace runir::kr::dl
