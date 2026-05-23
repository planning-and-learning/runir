#include "module.hpp"

#include <nanobind/stl/string.h>
#include <runir/kr/dl/grammar/parser/base/parser.hpp>

namespace runir::kr::dl::ext
{

void bind_parser(nb::module_& m)
{
    m.def(
        "parse_concept",
        [](const std::string& description) { (void) runir::kr::dl::grammar::parser::base::parse_concept_ast<runir::kr::dl::ExtFamilyTag>(description); },
        nb::arg("description"));
    m.def(
        "parse_role",
        [](const std::string& description) { (void) runir::kr::dl::grammar::parser::base::parse_role_ast<runir::kr::dl::ExtFamilyTag>(description); },
        nb::arg("description"));
}

}  // namespace runir::kr::dl::ext
