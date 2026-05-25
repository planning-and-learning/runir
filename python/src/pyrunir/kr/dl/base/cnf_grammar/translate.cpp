#include "module.hpp"

#include <runir/kr/dl/cnf_grammar/base/translate.hpp>

namespace runir::kr::dl::base
{

void bind_cnf_grammar_translate(nb::module_& m)
{
    using TranslateFn = runir::kr::dl::cnf_grammar::base::GrammarView (*)(const runir::kr::dl::grammar::base::GrammarView&,
                                                                          runir::kr::dl::cnf_grammar::base::ConstructorRepository&);

    m.def("translate",
          static_cast<TranslateFn>(&runir::kr::dl::cnf_grammar::base::translate),
          nb::arg("grammar"),
          nb::arg("repository"),
          nb::keep_alive<0, 2>());
}

}  // namespace runir::kr::dl::base
