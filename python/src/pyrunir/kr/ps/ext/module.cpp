#include "module.hpp"

#include "pyrunir/kr/ps/ext/dl/module.hpp"

namespace runir::kr::ps::ext
{

void bind_module_definitions(nb::module_& m)
{
    bind_indices(m);
    bind_datas(m);
    bind_views(m);
    bind_repository(m);
    bind_module_program_executor(m);

    auto dl = m.def_submodule("dl");
    runir::kr::ps::ext::dl::bind_module_definitions(dl);
}

}  // namespace runir::kr::ps::ext
