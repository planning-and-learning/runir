#include "runir/kr/dl/grammar/parser/parser.hpp"

#include "runir/kr/dl/grammar/parser/parse.hpp"
#include "runir/kr/dl/grammar/parser/parsers.hpp"

namespace runir::kr::dl::grammar::parser
{

template<runir::kr::dl::FamilyTag Family>
void parse_concept_ast(const std::string& description,
                       runir::kr::dl::grammar::ast::ConceptConstructor<Family>& result,
                       runir::kr::parser::ErrorHandlerType& error_handler)
{
    parse_ast(description, concept_root_parser<Family>(), result, error_handler, "Failed to parse DL concept description.");
}

template<runir::kr::dl::FamilyTag Family>
runir::kr::dl::grammar::ast::ConceptConstructor<Family> parse_concept_ast(const std::string& description)
{
    return parse_ast<runir::kr::dl::grammar::ast::ConceptConstructor<Family>>(description,
                                                                              concept_root_parser<Family>(),
                                                                              "Failed to parse DL concept description.");
}

template<runir::kr::dl::FamilyTag Family>
void parse_role_ast(const std::string& description,
                    runir::kr::dl::grammar::ast::RoleConstructor<Family>& result,
                    runir::kr::parser::ErrorHandlerType& error_handler)
{
    parse_ast(description, role_root_parser<Family>(), result, error_handler, "Failed to parse DL role description.");
}

template<runir::kr::dl::FamilyTag Family>
runir::kr::dl::grammar::ast::RoleConstructor<Family> parse_role_ast(const std::string& description)
{
    return parse_ast<runir::kr::dl::grammar::ast::RoleConstructor<Family>>(description, role_root_parser<Family>(), "Failed to parse DL role description.");
}

template<runir::kr::dl::FamilyTag Family>
void parse_boolean_ast(const std::string& description,
                       runir::kr::dl::grammar::ast::BooleanConstructor<Family>& result,
                       runir::kr::parser::ErrorHandlerType& error_handler)
{
    parse_ast(description, boolean_root_parser<Family>(), result, error_handler, "Failed to parse DL boolean description.");
}

template<runir::kr::dl::FamilyTag Family>
runir::kr::dl::grammar::ast::BooleanConstructor<Family> parse_boolean_ast(const std::string& description)
{
    return parse_ast<runir::kr::dl::grammar::ast::BooleanConstructor<Family>>(description,
                                                                              boolean_root_parser<Family>(),
                                                                              "Failed to parse DL boolean description.");
}

template<runir::kr::dl::FamilyTag Family>
void parse_numerical_ast(const std::string& description,
                         runir::kr::dl::grammar::ast::NumericalConstructor<Family>& result,
                         runir::kr::parser::ErrorHandlerType& error_handler)
{
    parse_ast(description, numerical_root_parser<Family>(), result, error_handler, "Failed to parse DL numerical description.");
}

template<runir::kr::dl::FamilyTag Family>
runir::kr::dl::grammar::ast::NumericalConstructor<Family> parse_numerical_ast(const std::string& description)
{
    return parse_ast<runir::kr::dl::grammar::ast::NumericalConstructor<Family>>(description,
                                                                                numerical_root_parser<Family>(),
                                                                                "Failed to parse DL numerical description.");
}

template<runir::kr::dl::FamilyTag Family>
void parse_grammar_ast(const std::string& description, runir::kr::dl::grammar::ast::Grammar<Family>& result, runir::kr::parser::ErrorHandlerType& error_handler)
{
    parse_ast(description, grammar_root_parser<Family>(), result, error_handler, "Failed to parse grammar description.");
}

template<runir::kr::dl::FamilyTag Family>
runir::kr::dl::grammar::ast::Grammar<Family> parse_grammar_ast(const std::string& description)
{
    return parse_ast<runir::kr::dl::grammar::ast::Grammar<Family>>(description, grammar_root_parser<Family>(), "Failed to parse grammar description.");
}

#define RUNIR_INSTANTIATE_AST_PARSERS(Family)                                                                                                \
    template void parse_concept_ast<Family>(const std::string&, ast::ConceptConstructor<Family>&, runir::kr::parser::ErrorHandlerType&);     \
    template ast::ConceptConstructor<Family> parse_concept_ast<Family>(const std::string&);                                                  \
    template void parse_role_ast<Family>(const std::string&, ast::RoleConstructor<Family>&, runir::kr::parser::ErrorHandlerType&);           \
    template ast::RoleConstructor<Family> parse_role_ast<Family>(const std::string&);                                                        \
    template void parse_boolean_ast<Family>(const std::string&, ast::BooleanConstructor<Family>&, runir::kr::parser::ErrorHandlerType&);     \
    template ast::BooleanConstructor<Family> parse_boolean_ast<Family>(const std::string&);                                                  \
    template void parse_numerical_ast<Family>(const std::string&, ast::NumericalConstructor<Family>&, runir::kr::parser::ErrorHandlerType&); \
    template ast::NumericalConstructor<Family> parse_numerical_ast<Family>(const std::string&);                                              \
    template void parse_grammar_ast<Family>(const std::string&, ast::Grammar<Family>&, runir::kr::parser::ErrorHandlerType&);                \
    template ast::Grammar<Family> parse_grammar_ast<Family>(const std::string&);

RUNIR_INSTANTIATE_AST_PARSERS(runir::kr::BaseFamilyTag)
RUNIR_INSTANTIATE_AST_PARSERS(runir::kr::ExtFamilyTag)
RUNIR_INSTANTIATE_AST_PARSERS(runir::kr::UnsFamilyTag)

#undef RUNIR_INSTANTIATE_AST_PARSERS

}  // namespace runir::kr::dl::grammar::parser
