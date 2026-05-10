#include "pyrunir/graphs/module.hpp"
#include "pyrunir/kr/dl/module.hpp"

#include <nanobind/nanobind.h>

namespace nb = nanobind;

namespace runir
{

NB_MODULE(pyrunir, m)
{
    auto graphs = m.def_submodule("graphs");
    python::bind_graphs(graphs);

    auto kr = m.def_submodule("kr");
    auto dl = kr.def_submodule("dl");
    kr::dl::bind_module_definitions(dl);
}

}  // namespace runir
