#include "module.hpp"

namespace runir::kr::ps::base::dl
{

void bind_module_definitions(nb::module_& m)
{
    bind_indices(m);
    bind_evaluation_contexts(m);
    bind_views(m);
    bind_parser(m);
    bind_sketch_factory(m);
    bind_structural_termination(m);
}

}  // namespace runir::kr::ps::base::dl
