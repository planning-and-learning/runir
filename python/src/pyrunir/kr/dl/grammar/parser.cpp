#include "module.hpp"

#include <nanobind/stl/string.h>
#include <runir/kr/dl/grammar/parser.hpp>

namespace runir::kr::dl
{

void bind_grammar_parser(nb::module_& m)
{
    m.def("parse", &runir::kr::dl::grammar::parse_grammar, nb::arg("description"), nb::arg("domain"), nb::arg("repository"), nb::keep_alive<0, 3>());
}

}  // namespace runir::kr::dl
