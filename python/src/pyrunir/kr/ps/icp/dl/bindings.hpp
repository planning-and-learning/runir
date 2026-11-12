#ifndef PYRUNIR_KR_PS_ICP_DL_BINDINGS_HPP_
#define PYRUNIR_KR_PS_ICP_DL_BINDINGS_HPP_

#include "module.hpp"

namespace runir::kr::ps::icp::dl
{

void bind_features(nb::module_& m, RepositoryBinding& repository);
void bind_conditions(nb::module_& m, RepositoryBinding& repository);
void bind_effects(nb::module_& m, RepositoryBinding& repository);
void bind_parser(nb::module_& m);

}  // namespace runir::kr::ps::icp::dl

#endif
