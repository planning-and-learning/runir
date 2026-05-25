#include "module.hpp"

#include "base/module.hpp"
#include "ext/module.hpp"

namespace runir::kr::dl
{

void bind_module_definitions(nb::module_& m)
{
    auto base_module = m.def_submodule("base");
    base::bind_module_definitions(base_module);

    auto ext_module = m.def_submodule("ext");
    ext::bind_module_definitions(ext_module);
}

}  // namespace runir::kr::dl
