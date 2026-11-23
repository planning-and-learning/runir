#include "module.hpp"

#include "pyrunir/datasets/module.hpp"
#include "pyrunir/kr/module.hpp"

namespace runir
{

void bind_module_definitions(nb::module_& m)
{
    auto datasets = m.def_submodule("datasets");
    datasets::bind_module_definitions(datasets);

    auto kr = m.def_submodule("kr");
    kr::bind_module_definitions(kr);
}

}  // namespace runir
