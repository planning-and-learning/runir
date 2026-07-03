#include "runir/kr/dl/grammar/parser/parse.hpp"
#include "runir/kr/dl/grammar/parser/uns/grammar_parsers.hpp"
#include "runir/kr/dl/grammar/parser/uns/parser.hpp"

namespace runir::kr::dl::grammar::parser::uns
{

runir::kr::dl::grammar::ast::Grammar<runir::kr::UnsFamilyTag> parse_grammar_ast(const std::string& description)
{
    return runir::kr::dl::grammar::parser::parse_ast<runir::kr::dl::grammar::ast::Grammar<runir::kr::UnsFamilyTag>>(description,
                                                                                                                    grammar::grammar_root_parser(),
                                                                                                                    "Failed to parse DL uns description.");
}

runir::kr::dl::grammar::ast::Constructor<runir::kr::UnsFamilyTag, runir::kr::dl::ConceptTag> parse_concept_ast(const std::string& description)
{
    return runir::kr::dl::grammar::parser::parse_ast<runir::kr::dl::grammar::ast::Constructor<runir::kr::UnsFamilyTag, runir::kr::dl::ConceptTag>>(
        description,
        grammar::concept_root_parser(),
        "Failed to parse DL uns description.");
}

runir::kr::dl::grammar::ast::Constructor<runir::kr::UnsFamilyTag, runir::kr::dl::RoleTag> parse_role_ast(const std::string& description)
{
    return runir::kr::dl::grammar::parser::parse_ast<runir::kr::dl::grammar::ast::Constructor<runir::kr::UnsFamilyTag, runir::kr::dl::RoleTag>>(
        description,
        grammar::role_root_parser(),
        "Failed to parse DL uns description.");
}

runir::kr::dl::grammar::ast::Constructor<runir::kr::UnsFamilyTag, runir::kr::dl::BooleanTag> parse_boolean_ast(const std::string& description)
{
    return runir::kr::dl::grammar::parser::parse_ast<runir::kr::dl::grammar::ast::Constructor<runir::kr::UnsFamilyTag, runir::kr::dl::BooleanTag>>(
        description,
        grammar::boolean_root_parser(),
        "Failed to parse DL uns description.");
}

runir::kr::dl::grammar::ast::Constructor<runir::kr::UnsFamilyTag, runir::kr::dl::NumericalTag> parse_numerical_ast(const std::string& description)
{
    return runir::kr::dl::grammar::parser::parse_ast<runir::kr::dl::grammar::ast::Constructor<runir::kr::UnsFamilyTag, runir::kr::dl::NumericalTag>>(
        description,
        grammar::numerical_root_parser(),
        "Failed to parse DL uns description.");
}

}  // namespace runir::kr::dl::grammar::parser::uns
