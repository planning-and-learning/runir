#ifndef PYRUNIR_KR_DATABASE_MODULE_HPP_
#define PYRUNIR_KR_DATABASE_MODULE_HPP_

#include <nanobind/nanobind.h>

namespace runir::kr::database
{

namespace nb = nanobind;

/// The relational operations, plans, queries and their evaluation over pyyggdrasil.formalism's free relations.
void bind_module_definitions(nb::module_& m);

}  // namespace runir::kr::database

#endif
