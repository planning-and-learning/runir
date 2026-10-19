#include "translate_impl.hpp"

namespace runir::kr::dl::cnf_grammar
{

template FamilyGrammarView<runir::kr::UnsFamilyTag>
translate<runir::kr::UnsFamilyTag>(const runir::kr::dl::grammar::FamilyGrammarView<runir::kr::BaseFamilyTag>&,
                                   ConstructorRepositoryFor<runir::kr::UnsFamilyTag>&);

}  // namespace runir::kr::dl::cnf_grammar
