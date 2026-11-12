#ifndef RUNIR_KR_DL_DETAIL_REPOSITORY_INSTANTIATIONS_HPP_
#define RUNIR_KR_DL_DETAIL_REPOSITORY_INSTANTIATIONS_HPP_

/// Extern declarations of intern for every dl constructor type, instantiated in the library (src/kr/dl/repository.cpp).
/// Include only from sources that intern many of these types; see rule 9 of the
/// template rules (https://planning-and-learning.github.io/brokkr/template_rules.html).

#include "runir/kr/dl/declarations.hpp"

#include <yggdrasil/formalism/intern_instantiations.hpp>

/// The instantiations (PREFIX extern: declarations).
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage): it expands to explicit instantiations, declarations, which no function gives
#define RUNIR_KR_DL_INTERNS(PREFIX) YGG_FORMALISM_INTERNS(PREFIX, 113, false, runir::kr::dl::ConstructorRepositoryTypes)

RUNIR_KR_DL_INTERNS(extern)

#endif
