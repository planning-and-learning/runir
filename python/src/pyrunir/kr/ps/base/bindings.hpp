#ifndef PYRUNIR_KR_PS_BASE_BINDINGS_HPP_
#define PYRUNIR_KR_PS_BASE_BINDINGS_HPP_

#include "module.hpp"

#include <runir/kr/ps/base/declarations.hpp>

namespace runir::kr::ps::base
{

using RepositoryBinding = nb::class_<Repository>;

void bind_condition(nb::module_& m, RepositoryBinding& repository);
void bind_effect(nb::module_& m, RepositoryBinding& repository);
void bind_rule(nb::module_& m, RepositoryBinding& repository);
void bind_sketch(nb::module_& m, RepositoryBinding& repository);
void bind_repository(nb::module_& m, RepositoryBinding& repository);
void bind_sketch_executor(nb::module_& m);
void bind_syntactic_complexity(nb::module_& m);

}  // namespace runir::kr::ps::base

#endif
