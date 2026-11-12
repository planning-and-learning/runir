#ifndef PYRUNIR_KR_PS_ICP_BINDINGS_HPP_
#define PYRUNIR_KR_PS_ICP_BINDINGS_HPP_

#include "module.hpp"

#include <runir/kr/ps/icp/declarations.hpp>

namespace runir::kr::ps::icp
{

using RepositoryBinding = nb::class_<Repository>;

void bind_module(nb::module_& m, RepositoryBinding& repository);
void bind_program(nb::module_& m, RepositoryBinding& repository);
void bind_indexical(nb::module_& m, RepositoryBinding& repository);
void bind_rule(nb::module_& m, RepositoryBinding& repository);
void bind_program_executor(nb::module_& m);

}  // namespace runir::kr::ps::icp

#endif
