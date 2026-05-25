#include "module.hpp"

namespace runir::kr::ps::ext
{

void bind_module_definitions(nb::module_& m)
{
    bind_indices(m);
    bind_datas(m);
    bind_views(m);
    bind_repository(m);
    bind_sketch_executor(m);
    bind_module_factory(m);
    bind_parser(m);
}

}  // namespace runir::kr::ps::ext
