#include "module.hpp"

namespace runir::kr::uns::dl
{

void bind_module_definitions(nb::module_& m)
{
    bind_indices(m);
    bind_views(m);
    bind_parser(m);
    bind_classifier_factory(m);
}

}  // namespace runir::kr::uns::dl
