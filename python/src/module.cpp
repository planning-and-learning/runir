#include "pyrunir/module.hpp"
#include <yggdrasil/python/type_casters.hpp>

#include <nanobind/nanobind.h>

namespace nb = nanobind;

namespace runir
{

NB_MODULE(_pyrunir, m) { bind_module_definitions(m); }

}  // namespace runir
