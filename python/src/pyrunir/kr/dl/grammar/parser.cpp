#include "module.hpp"

#include <nanobind/stl/string.h>
#include <runir/knowledge_representation/dl/grammar/parser.hpp>

namespace runir::kr::dl
{

namespace grammar = runir::kr::dl::grammar;

using namespace nanobind::literals;

void bind_grammar_parser(nb::module_& m) { m.def("parse", &grammar::parse_grammar, "description"_a, "domain"_a, "repository"_a, nb::keep_alive<0, 3>()); }

}  // namespace runir::kr::dl
