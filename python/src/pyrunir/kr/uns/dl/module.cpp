#include "module.hpp"

#include "bindings.hpp"

namespace runir::kr::uns::dl
{

void bind_module_definitions(nb::module_& m)
{
    bind_feature(m);
    bind_parser(m);
    bind_classifier_factory(m);
}

}  // namespace runir::kr::uns::dl
