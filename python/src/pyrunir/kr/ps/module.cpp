#include "module.hpp"

#include "pyrunir/kr/ps/base/module.hpp"

namespace runir::kr::ps
{

void bind_module_definitions(nb::module_& m)
{
    auto base = m.def_submodule("base");
    runir::kr::ps::base::bind_module_definitions(base);
}

}  // namespace runir::kr::ps
