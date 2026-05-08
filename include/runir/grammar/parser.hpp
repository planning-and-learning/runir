#ifndef RUNIR_GRAMMAR_PARSER_HPP_
#define RUNIR_GRAMMAR_PARSER_HPP_

#include "runir/grammar/constructor_repository.hpp"
#include "runir/grammar/parser/parser.hpp"

#include <string>
#include <tyr/formalism/planning/repository.hpp>

namespace runir::grammar
{

GrammarView parse_grammar(const std::string& description, tyr::formalism::planning::DomainView domain, ConstructorRepository& repository);

}

#endif
