#include "runir/kr/dl/cnf_grammar/constructor_repository.hpp"
#include "runir/kr/dl/grammar/constructor_repository.hpp"
#include "translate_impl.hpp"

namespace runir::kr::dl::cnf_grammar
{

GrammarView translate(const runir::kr::dl::grammar::GrammarView& grammar, ConstructorRepository& repository)
{ return detail::translate_impl(grammar, repository); }

}  // namespace runir::kr::dl::cnf_grammar
