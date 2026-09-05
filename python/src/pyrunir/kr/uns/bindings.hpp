#ifndef PYRUNIR_KR_UNS_BINDINGS_HPP_
#define PYRUNIR_KR_UNS_BINDINGS_HPP_

#include "module.hpp"

#include <runir/kr/uns/declarations.hpp>

namespace runir::kr::uns
{

using RepositoryBinding = nb::class_<Repository>;
void bind_classifier_literal(nb::module_& m);
void bind_classifier_clause(nb::module_& m);
void bind_classifier(nb::module_& m);
void bind_repository(nb::module_& m, RepositoryBinding& repository);

}  // namespace runir::kr::uns

#endif
