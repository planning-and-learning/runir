#include "module.hpp"

#include "pyrunir/kr/gp/dl/module.hpp"

namespace runir::kr::gp
{

void bind_module_definitions(nb::module_& m)
{
    auto dl = m.def_submodule("dl");
    runir::kr::gp::dl::bind_module_definitions(dl);

    bind_indices(m);
    bind_views(m);
    bind_repository(m);
}

}  // namespace runir::kr::gp
