#include "module.hpp"

#include "pyrunir/kr/ps/base/dl/module.hpp"

namespace runir::kr::ps
{

void bind_module_definitions(nb::module_& m)
{
    auto base = m.def_submodule("base");
    auto dl = base.def_submodule("dl");
    runir::kr::ps::base::dl::bind_module_definitions(dl);

    bind_indices(m);
    bind_views(m);
    bind_repository(m);
    bind_sketch_executor(m);
    bind_syntactic_complexity(m);
}

}  // namespace runir::kr::ps
