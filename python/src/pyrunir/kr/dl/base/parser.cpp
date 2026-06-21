#include "module.hpp"

#include <nanobind/stl/string.h>
#include <runir/kr/dl/grammar/base/parser.hpp>

namespace runir::kr::dl::base
{

void bind_parser(nb::module_& m)
{
    m.def("parse_grammar",
          &runir::kr::dl::grammar::base::parse_grammar,
          nb::arg("description"),
          nb::arg("domain"),
          nb::arg("repository"),
          nb::keep_alive<0, 3>());
}

}  // namespace runir::kr::dl::base
