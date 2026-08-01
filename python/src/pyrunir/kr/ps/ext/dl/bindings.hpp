#ifndef PYRUNIR_KR_PS_EXT_DL_BINDINGS_HPP_
#define PYRUNIR_KR_PS_EXT_DL_BINDINGS_HPP_

#include "module.hpp"

namespace runir::kr::ps::ext::dl
{

void bind_feature(nb::module_& m, RepositoryBinding& repository);
void bind_condition(nb::module_& m, RepositoryBinding& repository);
void bind_effect(nb::module_& m, RepositoryBinding& repository);
void bind_module_factory(nb::module_& m);
void bind_parser(nb::module_& m);
void bind_structural_termination(nb::module_& m);

}  // namespace runir::kr::ps::ext::dl

#endif
