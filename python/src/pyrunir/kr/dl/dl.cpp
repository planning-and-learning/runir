#include "dl.hpp"

namespace runir::kr::dl
{

void bind_module_definitions(nb::module_& m)
{
    bind_indices(m);
    bind_datas(m);
    bind_views(m);
    bind_repositories(m);

    auto grammar_module = m.def_submodule("grammar");
    bind_grammar_module_definitions(grammar_module);

    auto cnf_module = m.def_submodule("cnf_grammar");
    bind_cnf_grammar_module_definitions(cnf_module);
}

}  // namespace runir::kr::dl
