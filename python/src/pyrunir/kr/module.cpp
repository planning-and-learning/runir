#include "module.hpp"

#include "pyrunir/kr/dl/module.hpp"

namespace runir::kr
{

void bind_module_definitions(nb::module_& m)
{
    auto dl = m.def_submodule("dl");
    runir::kr::dl::bind_module_definitions(dl);
}

}  // namespace runir::kr
