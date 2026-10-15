#ifndef RUNIR_KR_DL_GRAMMAR_PARSER_PARSER_HPP_
#define RUNIR_KR_DL_GRAMMAR_PARSER_PARSER_HPP_

#include "runir/kr/dl/grammar/ast/ast.hpp"
#include "runir/kr/parser/declarations.hpp"

#include <string>

namespace runir::kr::dl::grammar::parser
{

/// The full DL syntax.
struct FullDialectTag
{
};

/// The DL syntax without transitive closures and searches (r_transitive_closure, r_reflexive_transitive_closure,
/// n_distance, q_must_pass). The arguments of n_distance and q_must_pass are parsed in it: a search evaluates them per
/// state and its edges per expanded vertex, so a nested fixpoint or search would multiply its cost.
struct SearchFreeDialectTag
{
};

template<runir::kr::dl::FamilyTag Family, typename Dialect = FullDialectTag>
void parse_concept_ast(const std::string& description,
                       runir::kr::dl::grammar::ast::ConceptConstructor<Family>& result,
                       runir::kr::parser::ErrorHandlerType& error_handler);

template<runir::kr::dl::FamilyTag Family, typename Dialect = FullDialectTag>
runir::kr::dl::grammar::ast::ConceptConstructor<Family> parse_concept_ast(const std::string& description);

template<runir::kr::dl::FamilyTag Family, typename Dialect = FullDialectTag>
void parse_role_ast(const std::string& description,
                    runir::kr::dl::grammar::ast::RoleConstructor<Family>& result,
                    runir::kr::parser::ErrorHandlerType& error_handler);

template<runir::kr::dl::FamilyTag Family, typename Dialect = FullDialectTag>
runir::kr::dl::grammar::ast::RoleConstructor<Family> parse_role_ast(const std::string& description);

template<runir::kr::dl::FamilyTag Family, typename Dialect = FullDialectTag>
void parse_boolean_ast(const std::string& description,
                       runir::kr::dl::grammar::ast::BooleanConstructor<Family>& result,
                       runir::kr::parser::ErrorHandlerType& error_handler);

template<runir::kr::dl::FamilyTag Family, typename Dialect = FullDialectTag>
runir::kr::dl::grammar::ast::BooleanConstructor<Family> parse_boolean_ast(const std::string& description);

template<runir::kr::dl::FamilyTag Family, typename Dialect = FullDialectTag>
void parse_numerical_ast(const std::string& description,
                         runir::kr::dl::grammar::ast::NumericalConstructor<Family>& result,
                         runir::kr::parser::ErrorHandlerType& error_handler);

template<runir::kr::dl::FamilyTag Family, typename Dialect = FullDialectTag>
runir::kr::dl::grammar::ast::NumericalConstructor<Family> parse_numerical_ast(const std::string& description);

template<runir::kr::dl::FamilyTag Family, typename Dialect = FullDialectTag>
void parse_grammar_ast(const std::string& description,
                       runir::kr::dl::grammar::ast::Grammar<Family>& result,
                       runir::kr::parser::ErrorHandlerType& error_handler);

template<runir::kr::dl::FamilyTag Family, typename Dialect = FullDialectTag>
runir::kr::dl::grammar::ast::Grammar<Family> parse_grammar_ast(const std::string& description);

}  // namespace runir::kr::dl::grammar::parser

#endif
