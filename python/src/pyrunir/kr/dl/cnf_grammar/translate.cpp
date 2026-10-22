#include "bindings.hpp"

#include <runir/kr/dl/cnf_grammar/constructor_repository.hpp>
#include <runir/kr/dl/cnf_grammar/translate.hpp>
#include <runir/kr/dl/grammar/constructor_repository.hpp>

namespace runir::kr::dl
{

void bind_cnf_grammar_translate(nb::module_& m)
{
    using TranslateFn =
        runir::kr::dl::cnf_grammar::GrammarView (*)(const runir::kr::dl::grammar::GrammarView&, runir::kr::dl::cnf_grammar::ConstructorRepository&);

    m.def("translate", static_cast<TranslateFn>(&runir::kr::dl::cnf_grammar::translate), nb::arg("grammar"), nb::arg("repository"), nb::keep_alive<0, 2>());
}

}  // namespace runir::kr::dl
