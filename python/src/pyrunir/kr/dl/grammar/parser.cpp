#include "bindings.hpp"

#include <nanobind/stl/string.h>
#include <runir/kr/dl/grammar/constructor_repository.hpp>
#include <runir/kr/dl/grammar/parser.hpp>
#include <tyr/formalism/planning/repository.hpp>

namespace runir::kr::dl
{

void bind_grammar_parser(nb::module_& m)
{ m.def("parse_grammar", &runir::kr::dl::grammar::parse_grammar, nb::arg("description"), nb::arg("domain"), nb::arg("repository"), nb::keep_alive<0, 3>()); }

}  // namespace runir::kr::dl
