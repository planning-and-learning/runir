#ifndef RUNIR_KR_DL_GRAMMAR_PARSER_HPP_
#define RUNIR_KR_DL_GRAMMAR_PARSER_HPP_

#include "runir/kr/dl/grammar/declarations.hpp"

#include <string>
#include <tyr/formalism/planning/declarations.hpp>

namespace runir::kr::dl::grammar
{

FamilyGrammarView<runir::kr::BaseFamilyTag>
parse_grammar(const std::string& description, tyr::formalism::planning::DomainView domain, ConstructorRepositoryFor<runir::kr::BaseFamilyTag>& repository);

}  // namespace runir::kr::dl::grammar

#endif
