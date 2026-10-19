#include "translate_impl.hpp"

namespace runir::kr::dl::cnf_grammar
{

template FamilyGrammarView<runir::kr::BaseFamilyTag>
translate<runir::kr::BaseFamilyTag>(const runir::kr::dl::grammar::FamilyGrammarView<runir::kr::BaseFamilyTag>&,
                                    ConstructorRepositoryFor<runir::kr::BaseFamilyTag>&);

FamilyGrammarView<runir::kr::BaseFamilyTag> translate(const runir::kr::dl::grammar::FamilyGrammarView<runir::kr::BaseFamilyTag>& grammar,
                                                      ConstructorRepositoryFor<runir::kr::BaseFamilyTag>& repository)
{
    return runir::kr::dl::cnf_grammar::translate<runir::kr::BaseFamilyTag>(grammar, repository);
}

}  // namespace runir::kr::dl::cnf_grammar
