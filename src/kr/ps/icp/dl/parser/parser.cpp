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

struct ArgumentObjectClass : boost::spirit::x3::annotate_on_success
{
};
struct RegisterObjectClass : boost::spirit::x3::annotate_on_success
{
};
struct ObjectClass : boost::spirit::x3::annotate_on_success
{
};
struct XConditionClass : boost::spirit::x3::annotate_on_success
{
};
struct XEffectClass : boost::spirit::x3::annotate_on_success
{
};
struct CruleClass : boost::spirit::x3::annotate_on_success
{
};
struct RuleClass : boost::spirit::x3::annotate_on_success
{
};
struct RuleEntryClass : boost::spirit::x3::annotate_on_success
{
};
struct ResetPairClass : boost::spirit::x3::annotate_on_success
{
};
struct ModuleClass : boost::spirit::x3::annotate_on_success
{
};
struct ProgramClass : boost::spirit::x3::annotate_on_success
{
};

constexpr boost::spirit::x3::rule<ArgumentObjectClass, ast::ArgumentObject> argument_object = "argument_object";
constexpr boost::spirit::x3::rule<RegisterObjectClass, ast::RegisterObject> register_object = "register_object";
constexpr boost::spirit::x3::rule<ObjectClass, ast::Object> object = "object";
constexpr boost::spirit::x3::rule<XConditionClass, ast::XCondition> xcondition = "xcondition";
constexpr boost::spirit::x3::rule<XEffectClass, ast::XEffect> xeffect = "xeffect";
constexpr boost::spirit::x3::rule<CruleClass, ast::Crule> crule = "crule";
constexpr boost::spirit::x3::rule<RuleClass, ast::Rule> rule = "rule";
constexpr boost::spirit::x3::rule<RuleEntryClass, ast::RuleEntry> rule_entry = "rule_entry";
constexpr boost::spirit::x3::rule<ResetPairClass, ast::ResetPair> reset_pair = "reset_pair";
constexpr boost::spirit::x3::rule<ModuleClass, ast::Module> module = "module";
constexpr boost::spirit::x3::rule<ProgramClass, ast::Program> program = "program";

struct ModuleRoot : runir::kr::parser::ErrorHandlerBase
{
};

struct ProgramRoot : runir::kr::parser::ErrorHandlerBase
{
};

constexpr boost::spirit::x3::rule<ModuleRoot, ast::Module> module_root = "ICP module";
constexpr boost::spirit::x3::rule<ProgramRoot, ast::Program> program_root = "ICP program";

constexpr auto argument_object_def = context("action argument")[(lit('(') >> keyword(":argument")) > identifier > lit(')')];
constexpr auto register_object_def = concept_register_section_def;
constexpr auto object_def = argument_object | register_object;
constexpr auto xcondition_def = context("extended condition")[(lit('(') >> ((keyword("belongs") >> attr(true)) | (keyword("not-belongs") >> attr(false))))
                                                              > object > concept_feature_section_def > lit(')')];
constexpr auto xeffect_def = context(
    "extended effect")[(lit('(') >> ((keyword("enter") >> attr(true)) | (keyword("exit") >> attr(false)))) > object > concept_feature_section_def > lit(')')];
constexpr auto arguments = context(":arguments")[(lit('(') >> keyword(":arguments")) > *identifier > lit(')')];
constexpr auto xconditions = context(":xconditions")[(lit('(') >> keyword(":xconditions")) > *xcondition > lit(')')];
constexpr auto xeffects = context(":xeffects")[(lit('(') >> keyword(":xeffects")) > *xeffect > lit(')')];
constexpr auto crule_def = context("concept rule")[(lit('(') >> keyword(":crule")) > action_section > arguments > conditions_section > xconditions > xeffects
                                                   > optional_effects_section > lit(')')];
constexpr auto rule_def = concept_load_rule | role_load_rule | crule;
constexpr auto rule_entry_def = context(
    "rule")[(lit('(') >> keyword(":rule")) > symbol_section
            > context(":expression")[(lit('(') >> keyword(":expression")) > source_memory_section > target_memory_section > +rule > lit(')')] > lit(')')];
constexpr auto rules = context(":rules")[(lit('(') >> keyword(":rules")) > *rule_entry > lit(')')];
constexpr auto reset_pair_def = context("reset precedence")[(lit('(') >> keyword(":prec")) > identifier > identifier > lit(')')];
constexpr auto resets = context(":reset-spo")[(lit('(') >> keyword(":reset-spo")) > *reset_pair > lit(')')];
constexpr auto empty_arguments = context(":arguments")[(lit('(') >> keyword(":arguments")) > lit(')')];
constexpr auto module_def = context("ICP module")[(lit('(') >> keyword(":module")) > symbol_section > empty_arguments > registers_section > entry_section
                                                  > memory_section > features_section > rules > resets > lit(')')];
constexpr auto program_def = context("ICP program")[(lit('(') >> keyword(":program")) > entry_section > module > lit(')')];
constexpr auto module_root_def = module > boost::spirit::x3::eoi;
constexpr auto program_root_def = program > boost::spirit::x3::eoi;
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
