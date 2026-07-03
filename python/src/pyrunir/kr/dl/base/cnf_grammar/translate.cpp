#include "module.hpp"

#include <runir/kr/dl/cnf_grammar/translate.hpp>

namespace runir::kr::dl::base
{

void bind_cnf_grammar_translate(nb::module_& m)
{
    using TranslateFn = runir::kr::dl::cnf_grammar::FamilyGrammarView<runir::kr::BaseFamilyTag> (*)(
        const runir::kr::dl::grammar::FamilyGrammarView<runir::kr::BaseFamilyTag>&,
        runir::kr::dl::cnf_grammar::ConstructorRepositoryFor<runir::kr::BaseFamilyTag>&);

    m.def("translate", static_cast<TranslateFn>(&runir::kr::dl::cnf_grammar::translate), nb::arg("grammar"), nb::arg("repository"), nb::keep_alive<0, 2>());
}

}  // namespace runir::kr::dl::base
