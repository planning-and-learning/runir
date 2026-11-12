#ifndef RUNIR_KR_DL_CNF_GRAMMAR_DETAIL_REPOSITORY_INSTANTIATIONS_HPP_
#define RUNIR_KR_DL_CNF_GRAMMAR_DETAIL_REPOSITORY_INSTANTIATIONS_HPP_

/// Extern declarations of intern for every cnf grammar constructor type, instantiated in the library
/// (src/kr/dl/cnf_grammar/constructor_repository.cpp). Include only from sources that intern many of these types; see rule 9 of the
/// template rules (https://planning-and-learning.github.io/brokkr/template_rules.html).

#include "runir/kr/dl/cnf_grammar/declarations.hpp"

#include <yggdrasil/formalism/intern_instantiations.hpp>

/// The instantiations (PREFIX extern: declarations).
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage): it expands to explicit instantiations, declarations, which no function gives
#define RUNIR_KR_DL_CNF_GRAMMAR_INTERNS(PREFIX) YGG_FORMALISM_INTERNS(PREFIX, 61, false, runir::kr::dl::cnf_grammar::ConstructorRepositoryTypes)

RUNIR_KR_DL_CNF_GRAMMAR_INTERNS(extern)

#endif
