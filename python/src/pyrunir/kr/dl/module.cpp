#include "module.hpp"

#include "bindings.hpp"
#include "cnf_grammar/module.hpp"
#include "grammar/module.hpp"
#include "pyrunir/module.hpp"
#include "semantics/module.hpp"

namespace runir::kr::dl
{

void bind_module_definitions(nb::module_& m)
{
    bind_argument(m);
    bind_register(m);

    auto semantics_module = m.def_submodule("semantics");
    bind_semantics_module_definitions(semantics_module);
    m.attr("ConstructorRepository") = semantics_module.attr("ConstructorRepository");

    auto grammar_module = m.def_submodule("grammar");
    bind_grammar_module_definitions(grammar_module);

    auto cnf_module = m.def_submodule("cnf_grammar");
    bind_cnf_grammar_module_definitions(cnf_module);

    bind_concept_serialization();
    bind_role_serialization();
    bind_boolean_serialization();
    bind_numerical_serialization();
    bind_constructor_serialization();
    bind_query_serialization();
    bind_denotation_serialization();
}

}  // namespace runir::kr::dl
