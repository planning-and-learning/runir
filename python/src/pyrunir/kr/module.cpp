#include "module.hpp"

#include "pyrunir/kr/dl/module.hpp"
#include "pyrunir/kr/ps/module.hpp"

namespace runir::kr
{

void bind_module_definitions(nb::module_& m)
{
    auto dl = m.def_submodule("dl");
    runir::kr::dl::bind_module_definitions(dl);

    auto ps = m.def_submodule("ps");
    runir::kr::ps::bind_module_definitions(ps);
}

}  // namespace runir::kr
