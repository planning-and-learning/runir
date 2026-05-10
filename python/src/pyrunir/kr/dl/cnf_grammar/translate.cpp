#include "module.hpp"

#include <runir/knowledge_representation/dl/cnf_grammar/translate.hpp>

namespace runir::kr::dl
{

namespace cnf = runir::kr::dl::cnf_grammar;

using namespace nanobind::literals;

void bind_cnf_grammar_translate(nb::module_& m) { m.def("translate", &cnf::translate, "grammar"_a, "repository"_a, nb::keep_alive<0, 2>()); }

}  // namespace runir::kr::dl
