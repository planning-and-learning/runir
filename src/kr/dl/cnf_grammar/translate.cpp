#include "runir/kr/dl/cnf_grammar/detail/repository_instantiations_base.hpp"
#include "runir/kr/dl/grammar/detail/repository_instantiations_base.hpp"
#include "translate_impl.hpp"

namespace runir::kr::dl::cnf_grammar
{

FamilyGrammarView translate(const runir::kr::dl::grammar::FamilyGrammarView& grammar, ConstructorRepositoryFor& repository)
{
    return detail::translate_impl(grammar, repository);
}

}  // namespace runir::kr::dl::cnf_grammar
