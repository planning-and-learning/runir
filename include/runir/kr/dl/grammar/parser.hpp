#ifndef RUNIR_GRAMMAR_PARSER_HPP_
#define RUNIR_GRAMMAR_PARSER_HPP_

#include "runir/kr/dl/grammar/base/repository.hpp"

#include <string>
#include <tyr/formalism/planning/repository.hpp>

namespace runir::kr::dl::grammar::base
{

GrammarView parse_grammar(const std::string& description, tyr::formalism::planning::DomainView domain, ConstructorRepository& repository);

}  // namespace runir::kr::dl::grammar::base

#endif
