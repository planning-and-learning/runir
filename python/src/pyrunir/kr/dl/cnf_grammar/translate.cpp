#include "module.hpp"

#include <runir/kr/dl/cnf_grammar/translate.hpp>

namespace runir::kr::dl
{

void bind_cnf_grammar_translate(nb::module_& m)
{
    m.def("translate", &runir::kr::dl::cnf_grammar::translate, nb::arg("grammar"), nb::arg("repository"), nb::keep_alive<0, 2>());
}

}  // namespace runir::kr::dl
