#include "module.hpp"

#include "pyrunir/kr/ps/base/dl/module.hpp"

namespace runir::kr::ps::base
{

void bind_module_definitions(nb::module_& m)
{
    bind_indices(m);
    bind_views(m);
    bind_repository(m);
    bind_sketch_executor(m);
    bind_syntactic_complexity(m);

    auto dl = m.def_submodule("dl");
    runir::kr::ps::base::dl::bind_module_definitions(dl);
}

}  // namespace runir::kr::ps::base
