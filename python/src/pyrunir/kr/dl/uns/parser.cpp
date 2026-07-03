#include "module.hpp"

#include <nanobind/stl/string.h>
#include <runir/kr/dl/grammar/parser/parser.hpp>

namespace runir::kr::dl::uns
{

void bind_parser(nb::module_& m)
{
    m.def(
        "parse_boolean",
        [](const std::string& description) { (void) runir::kr::dl::grammar::parser::parse_boolean_ast<runir::kr::UnsFamilyTag>(description); },
        nb::arg("description"));
    m.def(
        "parse_numerical",
        [](const std::string& description) { (void) runir::kr::dl::grammar::parser::parse_numerical_ast<runir::kr::UnsFamilyTag>(description); },
        nb::arg("description"));
}

}  // namespace runir::kr::dl::uns
