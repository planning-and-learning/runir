#ifndef PYRUNIR_KR_DL_MODULE_HPP_
#define PYRUNIR_KR_DL_MODULE_HPP_

#include <nanobind/nanobind.h>

namespace runir::kr::dl
{

namespace nb = nanobind;

void bind_module_definitions(nb::module_& m);

/// The serialization hooks of the types this module binds (pyyggdrasil.serialization), once they are bound.
void bind_concept_serialization();
void bind_role_serialization();
void bind_boolean_serialization();
void bind_numerical_serialization();
void bind_constructor_serialization();
void bind_query_serialization();
void bind_denotation_serialization();

}

#endif
