#include "runir/kr/ps/icp/dl/parser/parser.hpp"

#include "runir/kr/parser/diagnostics.hpp"
#include "runir/kr/parser/parser.hpp"
#include "runir/kr/ps/ext/dl/parser/parser_def.hpp"
#include "runir/kr/ps/icp/dl/ast/ast_adapted.hpp"

#include <sstream>

namespace runir::kr::ps::icp::dl::parser
{

namespace
{

using boost::spirit::x3::attr;
using boost::spirit::x3::lit;
using runir::kr::parser::keyword;
using runir::kr::ps::ext::dl::parser::action_section;
using runir::kr::ps::ext::dl::parser::concept_feature_section_def;
using runir::kr::ps::ext::dl::parser::concept_load_rule;
using runir::kr::ps::ext::dl::parser::concept_register_section_def;
using runir::kr::ps::ext::dl::parser::conditions_section;
using runir::kr::ps::ext::dl::parser::entry_section;
using runir::kr::ps::ext::dl::parser::features_section;
using runir::kr::ps::ext::dl::parser::identifier;
using runir::kr::ps::ext::dl::parser::memory_section;
using runir::kr::ps::ext::dl::parser::optional_effects_section;
using runir::kr::ps::ext::dl::parser::registers_section;
using runir::kr::ps::ext::dl::parser::role_load_rule;
using runir::kr::ps::ext::dl::parser::source_memory_section;
using runir::kr::ps::ext::dl::parser::symbol_section;
using runir::kr::ps::ext::dl::parser::target_memory_section;
using ygg::diagnostics::context;

#define ICP_RULE(name, Type)                                     \
    struct name##_class : boost::spirit::x3::annotate_on_success \
    {                                                            \
    };                                                           \
    const boost::spirit::x3::rule<name##_class, Type> name = #name;
ICP_RULE(argument_object, ast::ArgumentObject)
ICP_RULE(register_object, ast::RegisterObject)
ICP_RULE(object, ast::Object)
ICP_RULE(xcondition, ast::XCondition)
ICP_RULE(xeffect, ast::XEffect)
ICP_RULE(crule, ast::Crule)
ICP_RULE(rule, ast::Rule)
ICP_RULE(rule_entry, ast::RuleEntry)
ICP_RULE(reset_pair, ast::ResetPair)
ICP_RULE(module, ast::Module)
ICP_RULE(program, ast::Program)
#undef ICP_RULE
struct ModuleRoot : runir::kr::parser::ErrorHandlerBase
{
};

struct ProgramRoot : runir::kr::parser::ErrorHandlerBase
{
};

const boost::spirit::x3::rule<ModuleRoot, ast::Module> module_root = "ICP module";
const boost::spirit::x3::rule<ProgramRoot, ast::Program> program_root = "ICP program";

const auto argument_object_def = context("action argument")[(lit('(') >> keyword(":argument")) > identifier > lit(')')];
const auto register_object_def = concept_register_section_def;
const auto object_def = argument_object | register_object;
const auto xcondition_def = context("extended condition")[(lit('(') >> ((keyword("belongs") >> attr(true)) | (keyword("not-belongs") >> attr(false)))) > object
                                                          > concept_feature_section_def > lit(')')];
const auto xeffect_def = context(
    "extended effect")[(lit('(') >> ((keyword("enter") >> attr(true)) | (keyword("exit") >> attr(false)))) > object > concept_feature_section_def > lit(')')];
const auto arguments = context(":arguments")[(lit('(') >> keyword(":arguments")) > *identifier > lit(')')];
const auto xconditions = context(":xconditions")[(lit('(') >> keyword(":xconditions")) > *xcondition > lit(')')];
const auto xeffects = context(":xeffects")[(lit('(') >> keyword(":xeffects")) > *xeffect > lit(')')];
const auto crule_def = context("concept rule")[(lit('(') >> keyword(":crule")) > action_section > arguments > conditions_section > xconditions > xeffects
                                               > optional_effects_section > lit(')')];
const auto rule_def = concept_load_rule | role_load_rule | crule;
const auto rule_entry_def = context(
    "rule")[(lit('(') >> keyword(":rule")) > symbol_section
            > context(":expression")[(lit('(') >> keyword(":expression")) > source_memory_section > target_memory_section > +rule > lit(')')] > lit(')')];
const auto rules = context(":rules")[(lit('(') >> keyword(":rules")) > *rule_entry > lit(')')];
const auto reset_pair_def = context("reset precedence")[(lit('(') >> keyword(":prec")) > identifier > identifier > lit(')')];
const auto resets = context(":reset-spo")[(lit('(') >> keyword(":reset-spo")) > *reset_pair > lit(')')];
const auto empty_arguments = context(":arguments")[(lit('(') >> keyword(":arguments")) > lit(')')];
const auto module_def = context("ICP module")[(lit('(') >> keyword(":module")) > symbol_section > empty_arguments > registers_section > entry_section
                                              > memory_section > features_section > rules > resets > lit(')')];
const auto program_def = context("ICP program")[(lit('(') >> keyword(":program")) > entry_section > module > lit(')')];
const auto module_root_def = module > boost::spirit::x3::eoi;
const auto program_root_def = program > boost::spirit::x3::eoi;
BOOST_SPIRIT_DEFINE(argument_object,
                    register_object,
                    object,
                    xcondition,
                    xeffect,
                    crule,
                    rule,
                    rule_entry,
                    reset_pair,
                    module,
                    program,
                    module_root,
                    program_root)

template<typename Parser, typename Ast>
void parse_ast(const std::string& description, const Parser& parser, Ast& result, runir::kr::parser::ErrorHandlerType& error_handler)
{
    auto first = description.cbegin();
    if (!runir::kr::parser::parse_full(first, description.cend(), parser, result, error_handler))
        throw runir::kr::parser::DiagnosticContext::parse_error(error_handler, "Failed to parse ICP description.", first);
}

}  // namespace

void parse_module_ast(const std::string& description, ast::Module& result, runir::kr::parser::ErrorHandlerType& error_handler)
{ parse_ast(description, module_root, result, error_handler); }

void parse_program_ast(const std::string& description, ast::Program& result, runir::kr::parser::ErrorHandlerType& error_handler)
{ parse_ast(description, program_root, result, error_handler); }

ast::Module parse_module_ast(const std::string& description)
{
    auto result = ast::Module {};
    auto errors = std::ostringstream {};
    auto handler = runir::kr::parser::ErrorHandlerType(description.cbegin(), description.cend(), errors);
    parse_module_ast(description, result, handler);
    return result;
}

ast::Program parse_program_ast(const std::string& description)
{
    auto result = ast::Program {};
    auto errors = std::ostringstream {};
    auto handler = runir::kr::parser::ErrorHandlerType(description.cbegin(), description.cend(), errors);
    parse_program_ast(description, result, handler);
    return result;
}

}  // namespace runir::kr::ps::icp::dl::parser
