#include "translate_impl.hpp"

namespace runir::kr::dl::cnf_grammar
{

template FamilyGrammarView<runir::kr::ExtFamilyTag>
translate<runir::kr::ExtFamilyTag>(const runir::kr::dl::grammar::FamilyGrammarView<runir::kr::BaseFamilyTag>&,
                                   ConstructorRepositoryFor<runir::kr::ExtFamilyTag>&);

}  // namespace runir::kr::dl::cnf_grammar
