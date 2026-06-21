#include "module.hpp"

#include <nanobind/stl/string.h>
#include <runir/kr/dl/grammar/parser/uns/parser.hpp>

namespace runir::kr::dl::uns
{

void bind_parser(nb::module_& m)
{
    m.def(
        "parse_boolean",
        [](const std::string& description) { (void) runir::kr::dl::grammar::parser::uns::parse_boolean_ast(description); },
        nb::arg("description"));
    m.def(
        "parse_numerical",
        [](const std::string& description) { (void) runir::kr::dl::grammar::parser::uns::parse_numerical_ast(description); },
        nb::arg("description"));
}

}  // namespace runir::kr::dl::uns
