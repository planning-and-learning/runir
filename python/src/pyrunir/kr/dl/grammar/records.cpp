#include "../record_bindings.hpp"
#include "bindings.hpp"

#include <runir/kr/dl/grammar/constructor_repository.hpp>
#include <runir/kr/dl/grammar/formatter.hpp>

namespace runir::kr::dl
{

void bind_grammar_records(nb::module_& m) { python::bind_records<grammar::GrammarFamily, grammar::ConstructorRepository>(m); }

}  // namespace runir::kr::dl
