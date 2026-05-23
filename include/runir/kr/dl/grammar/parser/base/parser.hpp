#ifndef RUNIR_KR_DL_GRAMMAR_PARSER_BASE_PARSER_HPP_
#define RUNIR_KR_DL_GRAMMAR_PARSER_BASE_PARSER_HPP_

#include "runir/kr/dl/grammar/parser/base/ast.hpp"
#include "runir/kr/dl/grammar/parser/ext/ast.hpp"

#include <string>

namespace runir::kr::dl::grammar::parser::base
{

template<runir::kr::dl::FamilyTag Family>
ast::ConceptConstructor<Family> parse_concept_ast(const std::string& description);

template<runir::kr::dl::FamilyTag Family>
ast::RoleConstructor<Family> parse_role_ast(const std::string& description);

}  // namespace runir::kr::dl::grammar::parser::base

#endif
