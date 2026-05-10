#include "module.hpp"

#include "cnf_grammar/module.hpp"
#include "grammar/module.hpp"
#include "semantics/module.hpp"

namespace runir::kr::dl
{

void bind_module_definitions(nb::module_& m)
{
    auto semantics_module = m.def_submodule("semantics");
    bind_semantics_module_definitions(semantics_module);

    auto grammar_module = m.def_submodule("grammar");
    bind_grammar_module_definitions(grammar_module);

    auto cnf_module = m.def_submodule("cnf_grammar");
    bind_cnf_grammar_module_definitions(cnf_module);
}

}  // namespace runir::kr::dl
