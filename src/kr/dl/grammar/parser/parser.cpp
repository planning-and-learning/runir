#include "runir/kr/dl/grammar/parser/parser.hpp"

#include "runir/kr/dl/grammar/parser/parse.hpp"
#include "runir/kr/dl/grammar/parser/parsers.hpp"

namespace runir::kr::dl::grammar::parser
{

template<runir::kr::dl::FamilyTag Family>
runir::kr::dl::grammar::ast::ConceptConstructor<Family> parse_concept_ast(const std::string& description)
{
    return parse_ast<runir::kr::dl::grammar::ast::ConceptConstructor<Family>>(description, concept_root_parser<Family>(), "Failed to parse DL concept description.");
}

template<runir::kr::dl::FamilyTag Family>
runir::kr::dl::grammar::ast::RoleConstructor<Family> parse_role_ast(const std::string& description)
{
    return parse_ast<runir::kr::dl::grammar::ast::RoleConstructor<Family>>(description, role_root_parser<Family>(), "Failed to parse DL role description.");
}

template<runir::kr::dl::FamilyTag Family>
runir::kr::dl::grammar::ast::BooleanConstructor<Family> parse_boolean_ast(const std::string& description)
{
    return parse_ast<runir::kr::dl::grammar::ast::BooleanConstructor<Family>>(description, boolean_root_parser<Family>(), "Failed to parse DL boolean description.");
}

template<runir::kr::dl::FamilyTag Family>
runir::kr::dl::grammar::ast::NumericalConstructor<Family> parse_numerical_ast(const std::string& description)
{
    return parse_ast<runir::kr::dl::grammar::ast::NumericalConstructor<Family>>(description, numerical_root_parser<Family>(), "Failed to parse DL numerical description.");
}

template<runir::kr::dl::FamilyTag Family>
runir::kr::dl::grammar::ast::Grammar<Family> parse_grammar_ast(const std::string& description)
{
    return parse_ast<runir::kr::dl::grammar::ast::Grammar<Family>>(description, grammar_root_parser<Family>(), "Failed to parse grammar description.");
}

template runir::kr::dl::grammar::ast::ConceptConstructor<runir::kr::BaseFamilyTag> parse_concept_ast<runir::kr::BaseFamilyTag>(const std::string&);
template runir::kr::dl::grammar::ast::RoleConstructor<runir::kr::BaseFamilyTag> parse_role_ast<runir::kr::BaseFamilyTag>(const std::string&);
template runir::kr::dl::grammar::ast::BooleanConstructor<runir::kr::BaseFamilyTag> parse_boolean_ast<runir::kr::BaseFamilyTag>(const std::string&);
template runir::kr::dl::grammar::ast::NumericalConstructor<runir::kr::BaseFamilyTag> parse_numerical_ast<runir::kr::BaseFamilyTag>(const std::string&);
template runir::kr::dl::grammar::ast::Grammar<runir::kr::BaseFamilyTag> parse_grammar_ast<runir::kr::BaseFamilyTag>(const std::string&);

template runir::kr::dl::grammar::ast::ConceptConstructor<runir::kr::ExtFamilyTag> parse_concept_ast<runir::kr::ExtFamilyTag>(const std::string&);
template runir::kr::dl::grammar::ast::RoleConstructor<runir::kr::ExtFamilyTag> parse_role_ast<runir::kr::ExtFamilyTag>(const std::string&);
template runir::kr::dl::grammar::ast::BooleanConstructor<runir::kr::ExtFamilyTag> parse_boolean_ast<runir::kr::ExtFamilyTag>(const std::string&);
template runir::kr::dl::grammar::ast::NumericalConstructor<runir::kr::ExtFamilyTag> parse_numerical_ast<runir::kr::ExtFamilyTag>(const std::string&);
template runir::kr::dl::grammar::ast::Grammar<runir::kr::ExtFamilyTag> parse_grammar_ast<runir::kr::ExtFamilyTag>(const std::string&);

template runir::kr::dl::grammar::ast::ConceptConstructor<runir::kr::UnsFamilyTag> parse_concept_ast<runir::kr::UnsFamilyTag>(const std::string&);
template runir::kr::dl::grammar::ast::RoleConstructor<runir::kr::UnsFamilyTag> parse_role_ast<runir::kr::UnsFamilyTag>(const std::string&);
template runir::kr::dl::grammar::ast::BooleanConstructor<runir::kr::UnsFamilyTag> parse_boolean_ast<runir::kr::UnsFamilyTag>(const std::string&);
template runir::kr::dl::grammar::ast::NumericalConstructor<runir::kr::UnsFamilyTag> parse_numerical_ast<runir::kr::UnsFamilyTag>(const std::string&);
template runir::kr::dl::grammar::ast::Grammar<runir::kr::UnsFamilyTag> parse_grammar_ast<runir::kr::UnsFamilyTag>(const std::string&);

}  // namespace runir::kr::dl::grammar::parser
