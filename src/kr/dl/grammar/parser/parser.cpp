#include "runir/kr/dl/grammar/parser/parser.hpp"

#include "runir/kr/dl/grammar/parser/parse.hpp"
#include "runir/kr/dl/grammar/parser/parsers.hpp"

namespace runir::kr::dl::grammar::parser
{

template<typename Dialect>
void parse_concept_ast(const std::string& description,
                       runir::kr::dl::grammar::ast::ConceptConstructor& result,
                       runir::kr::parser::ErrorHandlerType& error_handler)
{
    parse_ast(description, concept_root_parser<Dialect>(), result, error_handler, "Failed to parse DL concept description.");
}

template<typename Dialect>
runir::kr::dl::grammar::ast::ConceptConstructor parse_concept_ast(const std::string& description)
{
    return parse_ast<runir::kr::dl::grammar::ast::ConceptConstructor>(description, concept_root_parser<Dialect>(), "Failed to parse DL concept description.");
}

template<typename Dialect>
void parse_role_ast(const std::string& description, runir::kr::dl::grammar::ast::RoleConstructor& result, runir::kr::parser::ErrorHandlerType& error_handler)
{
    parse_ast(description, role_root_parser<Dialect>(), result, error_handler, "Failed to parse DL role description.");
}

template<typename Dialect>
runir::kr::dl::grammar::ast::RoleConstructor parse_role_ast(const std::string& description)
{
    return parse_ast<runir::kr::dl::grammar::ast::RoleConstructor>(description, role_root_parser<Dialect>(), "Failed to parse DL role description.");
}

template<typename Dialect>
void parse_boolean_ast(const std::string& description,
                       runir::kr::dl::grammar::ast::BooleanConstructor& result,
                       runir::kr::parser::ErrorHandlerType& error_handler)
{
    parse_ast(description, boolean_root_parser<Dialect>(), result, error_handler, "Failed to parse DL boolean description.");
}

template<typename Dialect>
runir::kr::dl::grammar::ast::BooleanConstructor parse_boolean_ast(const std::string& description)
{
    return parse_ast<runir::kr::dl::grammar::ast::BooleanConstructor>(description, boolean_root_parser<Dialect>(), "Failed to parse DL boolean description.");
}

template<typename Dialect>
void parse_numerical_ast(const std::string& description,
                         runir::kr::dl::grammar::ast::NumericalConstructor& result,
                         runir::kr::parser::ErrorHandlerType& error_handler)
{
    parse_ast(description, numerical_root_parser<Dialect>(), result, error_handler, "Failed to parse DL numerical description.");
}

template<typename Dialect>
runir::kr::dl::grammar::ast::NumericalConstructor parse_numerical_ast(const std::string& description)
{
    return parse_ast<runir::kr::dl::grammar::ast::NumericalConstructor>(description,
                                                                        numerical_root_parser<Dialect>(),
                                                                        "Failed to parse DL numerical description.");
}

template<typename Dialect>
void parse_grammar_ast(const std::string& description, runir::kr::dl::grammar::ast::Grammar& result, runir::kr::parser::ErrorHandlerType& error_handler)
{
    parse_ast(description, grammar_root_parser<Dialect>(), result, error_handler, "Failed to parse grammar description.");
}

template<typename Dialect>
runir::kr::dl::grammar::ast::Grammar parse_grammar_ast(const std::string& description)
{
    return parse_ast<runir::kr::dl::grammar::ast::Grammar>(description, grammar_root_parser<Dialect>(), "Failed to parse grammar description.");
}

#define RUNIR_INSTANTIATE_AST_PARSERS(Dialect)                                                                                        \
    template void parse_concept_ast<Dialect>(const std::string&, ast::ConceptConstructor&, runir::kr::parser::ErrorHandlerType&);     \
    template ast::ConceptConstructor parse_concept_ast<Dialect>(const std::string&);                                                  \
    template void parse_role_ast<Dialect>(const std::string&, ast::RoleConstructor&, runir::kr::parser::ErrorHandlerType&);           \
    template ast::RoleConstructor parse_role_ast<Dialect>(const std::string&);                                                        \
    template void parse_boolean_ast<Dialect>(const std::string&, ast::BooleanConstructor&, runir::kr::parser::ErrorHandlerType&);     \
    template ast::BooleanConstructor parse_boolean_ast<Dialect>(const std::string&);                                                  \
    template void parse_numerical_ast<Dialect>(const std::string&, ast::NumericalConstructor&, runir::kr::parser::ErrorHandlerType&); \
    template ast::NumericalConstructor parse_numerical_ast<Dialect>(const std::string&);                                              \
    template void parse_grammar_ast<Dialect>(const std::string&, ast::Grammar&, runir::kr::parser::ErrorHandlerType&);                \
    template ast::Grammar parse_grammar_ast<Dialect>(const std::string&);

RUNIR_INSTANTIATE_AST_PARSERS(FullDialectTag)
RUNIR_INSTANTIATE_AST_PARSERS(SearchFreeDialectTag)

#undef RUNIR_INSTANTIATE_AST_PARSERS

}  // namespace runir::kr::dl::grammar::parser
