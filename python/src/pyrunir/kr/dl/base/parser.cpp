#include "module.hpp"

#include <nanobind/stl/string.h>
#include <runir/kr/dl/grammar/base/parser.hpp>
#include <runir/kr/dl/grammar/parser/base/parser.hpp>

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
    m.def(
        "parse_concept",
        [](const std::string& description) { (void) runir::kr::dl::grammar::parser::base::parse_concept_ast(description); },
        nb::arg("description"));
    m.def(
        "parse_role",
        [](const std::string& description) { (void) runir::kr::dl::grammar::parser::base::parse_role_ast(description); },
        nb::arg("description"));
}

}  // namespace runir::kr::dl::base
