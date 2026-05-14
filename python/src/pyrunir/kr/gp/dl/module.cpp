#include "module.hpp"

namespace runir::kr::gp::dl
{

void bind_module_definitions(nb::module_& m)
{
    bind_indices(m);
    bind_views(m);
    bind_parser(m);
    bind_policy_factory(m);
}

}  // namespace runir::kr::gp::dl
