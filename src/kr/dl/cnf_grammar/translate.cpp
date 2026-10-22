#include "runir/kr/dl/cnf_grammar/detail/repository_instantiations.hpp"
#include "runir/kr/dl/grammar/detail/repository_instantiations.hpp"
#include "translate_impl.hpp"

namespace runir::kr::dl::cnf_grammar
{

GrammarView translate(const runir::kr::dl::grammar::GrammarView& grammar, ConstructorRepository& repository)
{
    return detail::translate_impl(grammar, repository);
}

}  // namespace runir::kr::dl::cnf_grammar
