#ifndef PYRUNIR_KR_GP_DL_MODULE_HPP_
#define PYRUNIR_KR_GP_DL_MODULE_HPP_

#include "../module.hpp"

namespace runir::kr::gp::dl
{

void bind_module_definitions(nb::module_& m);
void bind_parser(nb::module_& m);
void bind_policy_factory(nb::module_& m);

}  // namespace runir::kr::gp::dl

#endif
