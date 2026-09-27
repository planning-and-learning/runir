#include "module.hpp"

#include "pyrunir/kr/ps/base/module.hpp"
#include "pyrunir/kr/ps/ext/module.hpp"
#include "pyrunir/kr/ps/icp/module.hpp"

namespace runir::kr::ps
{

void bind_module_definitions(nb::module_& m)
{
    auto base = m.def_submodule("base");
    runir::kr::ps::base::bind_module_definitions(base);

    auto ext = m.def_submodule("ext");
    runir::kr::ps::ext::bind_module_definitions(ext);

    auto icp = m.def_submodule("icp");
    runir::kr::ps::icp::bind_module_definitions(icp);
}

}  // namespace runir::kr::ps
