#include "module.hpp"

#include "pyrunir/kr/uns/dl/module.hpp"

namespace runir::kr::uns
{

void bind_module_definitions(nb::module_& m)
{
    bind_indices(m);
    bind_views(m);
    bind_repository(m);

    auto dl = m.def_submodule("dl");
    runir::kr::uns::dl::bind_module_definitions(dl);
}

}  // namespace runir::kr::uns
