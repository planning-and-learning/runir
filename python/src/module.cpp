#include "pyrunir/module.hpp"

#include <nanobind/nanobind.h>

namespace nb = nanobind;

namespace runir
{

NB_MODULE(_pyrunir, m)
{
    bind_module_definitions(m);
}

}  // namespace runir
