#ifndef RUNIR_GRAMMAR_PARSER_HPP_
#define RUNIR_GRAMMAR_PARSER_HPP_

#include "runir/knowledge_representation/dl/grammar/constructor_repository.hpp"
#include "runir/knowledge_representation/dl/grammar/parser/parser.hpp"

#include <string>
#include <tyr/formalism/planning/repository.hpp>

namespace runir::kr::dl::grammar
{

GrammarView parse_grammar(const std::string& description, tyr::formalism::planning::DomainView domain, ConstructorRepository& repository);

}

#endif
