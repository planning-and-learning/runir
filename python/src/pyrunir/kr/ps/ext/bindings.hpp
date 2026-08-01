#ifndef PYRUNIR_KR_PS_EXT_BINDINGS_HPP_
#define PYRUNIR_KR_PS_EXT_BINDINGS_HPP_

#include "module.hpp"

#include <runir/kr/ps/ext/declarations.hpp>

namespace runir::kr::ps::ext
{

using RepositoryBinding = nb::class_<Repository>;

void bind_memory_state(nb::module_& m, RepositoryBinding& repository);
void bind_module_symbol(nb::module_& m, RepositoryBinding& repository);
void bind_module(nb::module_& m, RepositoryBinding& repository);
void bind_module_program(nb::module_& m, RepositoryBinding& repository);
void bind_condition(nb::module_& m, RepositoryBinding& repository);
void bind_effect(nb::module_& m, RepositoryBinding& repository);
void bind_rule_variant(nb::module_& m, RepositoryBinding& repository);
void bind_rule(nb::module_& m, RepositoryBinding& repository);
void bind_repository(nb::module_& m, RepositoryBinding& repository);
void bind_module_program_executor(nb::module_& m);

}  // namespace runir::kr::ps::ext

#endif
