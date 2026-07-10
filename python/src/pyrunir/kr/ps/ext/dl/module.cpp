#include "module.hpp"

namespace runir::kr::ps::ext::dl
{

void bind_module_definitions(nb::module_& m)
{
    bind_indices(m);
    bind_datas(m);
    bind_views(m);
    bind_module_factory(m);
    bind_parser(m);
    bind_structural_termination(m);
}

}  // namespace runir::kr::ps::ext::dl
