#include "runir/kr/dl/grammar/parser/base/parser.hpp"

#include "runir/kr/dl/grammar/parser/base/grammar_parsers.hpp"
#include "runir/kr/dl/grammar/parser/base/parsers.hpp"
#include "runir/kr/dl/grammar/parser/parse.hpp"

namespace runir::kr::dl::grammar::parser::base
{

ast::ConceptConstructor<runir::kr::BaseFamilyTag> parse_concept_ast(const std::string& description)
{
    return runir::kr::dl::grammar::parser::parse_ast<ast::ConceptConstructor<runir::kr::BaseFamilyTag>>(description,
                                                                                                        base_concept_root_parser(),
                                                                                                        "Failed to parse DL constructor description.");
}

ast::RoleConstructor<runir::kr::BaseFamilyTag> parse_role_ast(const std::string& description)
{
    return runir::kr::dl::grammar::parser::parse_ast<ast::RoleConstructor<runir::kr::BaseFamilyTag>>(description,
                                                                                                     base_role_root_parser(),
                                                                                                     "Failed to parse DL constructor description.");
}

runir::kr::dl::grammar::ast::Grammar<runir::kr::BaseFamilyTag> parse_grammar_ast(const std::string& description)
{
    return runir::kr::dl::grammar::parser::parse_ast<runir::kr::dl::grammar::ast::Grammar<runir::kr::BaseFamilyTag>>(
        description,
        runir::kr::dl::grammar::parser::base::grammar::grammar_root_parser(),
        "Failed to parse grammar description.");
}

}  // namespace runir::kr::dl::grammar::parser::base
