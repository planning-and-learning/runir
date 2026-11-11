#ifndef RUNIR_KR_DL_GRAMMAR_DETAIL_REPOSITORY_INSTANTIATIONS_HPP_
#define RUNIR_KR_DL_GRAMMAR_DETAIL_REPOSITORY_INSTANTIATIONS_HPP_

/// Extern declarations of intern for every grammar constructor type, instantiated in the library
/// (src/kr/dl/grammar/constructor_repository.cpp). Include only from sources that intern many of these types; see yggdrasil's
/// docs/TEMPLATE_RULES.md, rule 9.

#include "runir/kr/dl/grammar/declarations.hpp"

#include <yggdrasil/formalism/intern_instantiations.hpp>

/// The instantiations (PREFIX extern: declarations).
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage): it expands to explicit instantiations, declarations, which no function gives
#define RUNIR_KR_DL_GRAMMAR_INTERNS(PREFIX) YGG_FORMALISM_INTERNS(PREFIX, 58, false, runir::kr::dl::grammar::ConstructorRepositoryTypes)

RUNIR_KR_DL_GRAMMAR_INTERNS(extern)

#endif
