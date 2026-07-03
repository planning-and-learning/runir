#include "runir/kr/dl/grammar/parser/ext/parser.hpp"

#include "runir/kr/dl/grammar/parser/ext/parsers.hpp"
#include "runir/kr/dl/grammar/parser/parse.hpp"

namespace runir::kr::dl::grammar::parser::ext
{

runir::kr::dl::grammar::parser::base::ast::ConceptConstructor<runir::kr::ExtFamilyTag> parse_concept_ast(const std::string& description)
{
    return runir::kr::dl::grammar::parser::parse_ast<runir::kr::dl::grammar::parser::base::ast::ConceptConstructor<runir::kr::ExtFamilyTag>>(
        description,
        ext_concept_root_parser(),
        "Failed to parse DL ext constructor description.");
}

runir::kr::dl::grammar::parser::base::ast::RoleConstructor<runir::kr::ExtFamilyTag> parse_role_ast(const std::string& description)
{
    return runir::kr::dl::grammar::parser::parse_ast<runir::kr::dl::grammar::parser::base::ast::RoleConstructor<runir::kr::ExtFamilyTag>>(
        description,
        ext_role_root_parser(),
        "Failed to parse DL ext constructor description.");
}

}  // namespace runir::kr::dl::grammar::parser::ext
