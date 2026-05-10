#include "module.hpp"

#include "pyrunir/graphs/module.hpp"
#include "pyrunir/kr/module.hpp"

namespace runir
{

void bind_module_definitions(nb::module_& m)
{
    auto graphs = m.def_submodule("graphs");
    graphs::bind_module_definitions(graphs);

    auto kr = m.def_submodule("kr");
    kr::bind_module_definitions(kr);
}

}  // namespace runir
