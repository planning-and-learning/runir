#ifndef RUNIR_KR_PS_EXT_DL_PARSER_PARSER_DEF_HPP_
#define RUNIR_KR_PS_EXT_DL_PARSER_PARSER_DEF_HPP_

#include "runir/kr/dl/grammar/parser/parsers.hpp"
#include "runir/kr/parser/error_handler.hpp"
#include "runir/kr/ps/base/dl/ast/ast_adapted.hpp"
#include "runir/kr/ps/dl/declarations.hpp"
#include "runir/kr/ps/ext/dl/ast/ast_adapted.hpp"
#include "runir/kr/ps/ext/dl/parser/parsers.hpp"

#include <boost/spirit/home/x3/support/utility/annotate_on_success.hpp>
#include <string>

namespace runir::kr::ps::ext::dl::parser
{

using boost::spirit::x3::attr;
using boost::spirit::x3::eoi;
using boost::spirit::x3::lexeme;
using boost::spirit::x3::lit;
using boost::spirit::x3::raw;
using runir::kr::parser::keyword;
using ygg::diagnostics::context;

using boost::spirit::x3::ascii::alnum;
using boost::spirit::x3::ascii::alpha;
using boost::spirit::x3::ascii::char_;

struct IdentifierText
{
};
struct QuotedStringText
{
};

identifier_type const identifier = "identifier";
quoted_string_type const quoted_string = "quoted string";
symbol_section_type const symbol_section = ":symbol";
source_memory_section_type const source_memory_section = ":source-memory";
target_memory_section_type const target_memory_section = ":target-memory";
entry_section_type const entry_section = ":entry";
action_section_type const action_section = ":action";
callee_section_type const callee_section = ":callee";
concept_argument_type const concept_argument = "concept_argument";
role_argument_type const role_argument = "role_argument";
boolean_argument_type const boolean_argument = "boolean_argument";
numerical_argument_type const numerical_argument = "numerical_argument";
argument_type const argument = "argument";
arguments_section_type const arguments_section = ":arguments";
concept_register_type const concept_register = "concept_register";
role_register_type const role_register = "role_register";
register_type const register_ = "register";
registers_section_type const registers_section = ":registers";
concept_feature_type const concept_feature = "concept_feature";
role_feature_type const role_feature = "role_feature";
boolean_feature_type const boolean_feature = "boolean_feature";
numerical_feature_type const numerical_feature = "numerical_feature";
query_feature_type const query_feature = "query_feature";
feature_type const feature = "feature";
features_section_type const features_section = ":features";
symbol_expression_type const symbol_expression = "symbol_expression";
named_value_type const named_value = "named_value";
positive_condition_type const positive_condition = "positive_condition";
negative_condition_type const negative_condition = "negative_condition";
equal_zero_condition_type const equal_zero_condition = "equal_zero_condition";
greater_zero_condition_type const greater_zero_condition = "greater_zero_condition";
condition_observation_type const condition_observation = "condition_observation";
condition_type const condition = "condition";
conditions_section_type const conditions_section = ":conditions";
positive_effect_type const positive_effect = "positive_effect";
negative_effect_type const negative_effect = "negative_effect";
unchanged_effect_type const unchanged_effect = "unchanged_effect";
increases_effect_type const increases_effect = "increases_effect";
decreases_effect_type const decreases_effect = "decreases_effect";
effect_observation_type const effect_observation = "effect_observation";
effect_type const effect = "effect";
order_term_type const order_term = "order term";
order_section_type const order_section = ":order";
effects_section_type const effects_section = ":effects";
arguments_expression_section_type const arguments_expression_section = ":arguments";
concept_load_rule_type const concept_load_rule = "concept_load_rule";
role_load_rule_type const role_load_rule = "role_load_rule";
concept_choose_rule_type const concept_choose_rule = "concept_choose_rule";
role_choose_rule_type const role_choose_rule = "role_choose_rule";
sketch_rule_type const sketch_rule = "sketch_rule";
do_rule_type const do_rule = "do_rule";
call_rule_type const call_rule = "call_rule";
action_rule_type const action_rule = "action_rule";
backtrack_rule_type const backtrack_rule = "backtrack_rule";
rule_type const rule = "rule";
rule_entry_type const rule_entry = "rule_entry";
rules_section_type const rules_section = ":rules";
memory_section_type const memory_section = ":memory";
module_type const module = "module";
program_type const program = "program";
module_root_type const module_root = "module_root";
program_root_type const program_root = "program_root";

const auto identifier_def = boost::spirit::x3::rule<IdentifierText, std::string> { "identifier_text" } =
    raw[lexeme[(alpha | char_('_')) >> *(alnum | char_('_') | char_('-'))]];
const auto quoted_string_def = boost::spirit::x3::rule<QuotedStringText, std::string> { "quoted_string_text" } =
    lexeme[context("quoted string")[(lit('"') >> *((lit('\\') >> char_) | (char_ - '"'))) > lit('"')]];

const auto symbol_section_def = context(":symbol")[(lit("(") >> keyword(":symbol")) > identifier > lit(")")];
const auto source_memory_section_def = context(":source-memory")[(lit("(") >> keyword(":source-memory")) > identifier > lit(")")];
const auto target_memory_section_def = context(":target-memory")[(lit("(") >> keyword(":target-memory")) > identifier > lit(")")];
const auto entry_section_def = context(":entry")[(lit("(") >> keyword(":entry")) > identifier > lit(")")];
const auto action_section_def = context(":action")[(lit("(") >> keyword(":action")) > quoted_string > lit(")")];
const auto callee_section_def = context(":callee")[(lit("(") >> keyword(":callee")) > identifier > lit(")")];

const auto concept_argument_def = context("concept argument")[keyword("(:concept") > identifier > lit(")")];
const auto role_argument_def = context("role argument")[keyword("(:role") > identifier > lit(")")];
const auto boolean_argument_def = context("boolean argument")[keyword("(:boolean") > identifier > lit(")")];
const auto numerical_argument_def = context("numerical argument")[keyword("(:numerical") > identifier > lit(")")];
const auto argument_def = concept_argument | role_argument | boolean_argument | numerical_argument;
const auto arguments_section_def = context(":arguments")[(lit("(") >> keyword(":arguments")) > *argument > lit(")")];

const auto concept_register_def = context("concept register")[keyword("(:concept") > identifier > lit(")")];
const auto role_register_def = context("role register")[keyword("(:role") > identifier > lit(")")];
const auto register__def = concept_register | role_register;
const auto registers_section_def = context(":registers")[(lit("(") >> keyword(":registers")) > *register_ > lit(")")];

const auto concept_feature_def =
    context("concept feature")[keyword("(:concept") > symbol_section
                               > context(":expression")[(lit("(") >> keyword(":expression"))
                                                        > runir::kr::dl::grammar::parser::concept_parser<runir::kr::ExtFamilyTag>() > lit(")")]
                               > lit(")")];
const auto role_feature_def = context(
    "role feature")[keyword("(:role") > symbol_section > context(
                        ":expression")[(lit("(") >> keyword(":expression")) > runir::kr::dl::grammar::parser::role_parser<runir::kr::ExtFamilyTag>() > lit(")")]
                    > lit(")")];
const auto boolean_feature_def =
    context("boolean feature")[keyword("(:boolean") > symbol_section
                               > context(":expression")[(lit("(") >> keyword(":expression"))
                                                        > runir::kr::dl::grammar::parser::boolean_parser<runir::kr::ExtFamilyTag>() > lit(")")]
                               > lit(")")];
const auto numerical_feature_def =
    context("numerical feature")[keyword("(:numerical") > symbol_section
                                 > context(":expression")[(lit("(") >> keyword(":expression"))
                                                          > runir::kr::dl::grammar::parser::numerical_parser<runir::kr::ExtFamilyTag>() > lit(")")]
                                 > lit(")")];
const auto query_feature_def =
    context("query feature")[keyword("(:query") > symbol_section
                             > context(":expression")[(lit("(") >> keyword(":expression"))
                                                      > runir::kr::dl::grammar::parser::query_parser<runir::kr::ExtFamilyTag>() > lit(")")]
                             > lit(")")];
const auto feature_def = concept_feature | role_feature | boolean_feature | numerical_feature | query_feature;
const auto features_section_def = context(":features")[(lit("(") >> keyword(":features")) > *feature > lit(")")];

const auto symbol_expression_def = identifier;
const auto concept_feature_section_def = context("concept feature reference")[keyword("(:concept") > identifier > lit(")")];
const auto role_feature_section_def = context("role feature reference")[keyword("(:role") > identifier > lit(")")];
const auto query_feature_section_def = context("query feature reference")[keyword("(:query") > identifier > lit(")")];
const auto concept_register_section_def =
    context("concept register reference")[(lit("(") >> keyword(":register") >> keyword("(:concept")) > identifier > lit(")") > lit(")")];
const auto role_register_section_def =
    context("role register reference")[(lit("(") >> keyword(":register") >> keyword("(:role")) > identifier > lit(")") > lit(")")];
const auto arguments_expression_section_def = context(":arguments")[(lit("(") >> keyword(":arguments")) > *symbol_expression > lit(")")];

const auto positive_condition_def = keyword(runir::kr::ps::dl::Positive::keyword) >> attr(runir::kr::ps::base::dl::ast::Positive {});
const auto negative_condition_def = keyword(runir::kr::ps::dl::Negative::keyword) >> attr(runir::kr::ps::base::dl::ast::Negative {});
const auto equal_zero_condition_def = keyword(runir::kr::ps::dl::EqualZero::keyword) >> attr(runir::kr::ps::base::dl::ast::EqualZero {});
const auto greater_zero_condition_def = keyword(runir::kr::ps::dl::GreaterZero::keyword) >> attr(runir::kr::ps::base::dl::ast::GreaterZero {});
const auto condition_observation_def = positive_condition | negative_condition | equal_zero_condition | greater_zero_condition;
const auto condition_def = context("condition")[(lit("(") >> condition_observation) > identifier > lit(")")];
const auto conditions_section_def = context(":conditions")[(lit("(") >> keyword(":conditions")) > *condition > lit(")")];

const auto positive_effect_def = keyword(runir::kr::ps::dl::Positive::keyword) >> attr(runir::kr::ps::base::dl::ast::Positive {});
const auto negative_effect_def = keyword(runir::kr::ps::dl::Negative::keyword) >> attr(runir::kr::ps::base::dl::ast::Negative {});
const auto unchanged_effect_def = keyword(runir::kr::ps::dl::Unchanged::keyword) >> attr(runir::kr::ps::base::dl::ast::Unchanged {});
const auto increases_effect_def = keyword(runir::kr::ps::dl::Increases::keyword) >> attr(runir::kr::ps::base::dl::ast::Increases {});
const auto decreases_effect_def = keyword(runir::kr::ps::dl::Decreases::keyword) >> attr(runir::kr::ps::base::dl::ast::Decreases {});
const auto effect_observation_def = positive_effect | negative_effect | unchanged_effect | increases_effect | decreases_effect;
const auto effect_def = context("effect")[(lit("(") >> effect_observation) > identifier > lit(")")];
const auto effects_section_def = context(":effects")[(lit("(") >> keyword(":effects")) > *effect > lit(")")];
const auto optional_effects_section = effects_section | attr(std::vector<ast::Effect> {});

const auto order_term_def =
    context("order term")[lit("(") > ((keyword("min") >> attr(OrderDirection::MIN)) | (keyword("max") >> attr(OrderDirection::MAX))) > identifier > lit(")")];
const auto order_section_def = context(":order")[(lit("(") >> keyword(":order")) > *order_term > lit(")")];
const auto optional_order_section = order_section | attr(std::vector<ast::OrderTerm> {});

const auto concept_load_rule_def = context("concept load rule")[(keyword("(:load") >> conditions_section >> concept_feature_section_def)
                                                                > concept_register_section_def > optional_effects_section > lit(")")];
const auto role_load_rule_def = context(
    "role load rule")[(keyword("(:load") >> conditions_section >> role_feature_section_def) > role_register_section_def > optional_effects_section > lit(")")];
const auto concept_choose_rule_def =
    context("concept choose rule")[(keyword("(:choose") >> conditions_section >> concept_feature_section_def) > concept_register_section_def
                                   > optional_effects_section > optional_order_section > lit(")")];
const auto role_choose_rule_def = context("role choose rule")[(keyword("(:choose") >> conditions_section >> role_feature_section_def)
                                                              > role_register_section_def > optional_effects_section > optional_order_section > lit(")")];
const auto sketch_rule_def = context("sketch rule")[keyword("(:sketch") > conditions_section > effects_section > lit(")")];
const auto do_rule_def =
    context("action rule")[keyword("(:do") > conditions_section > action_section > arguments_expression_section > effects_section > lit(")")];
const auto action_rule_def =
    context("query action rule")[keyword("(:action") > conditions_section > action_section > query_feature_section_def > effects_section > lit(")")];
const auto call_rule_def = context("call rule")[keyword("(:call") > conditions_section > callee_section > arguments_expression_section > lit(")")];
const auto backtrack_rule_def = context("backtrack rule")[keyword("(:backtrack") > conditions_section > lit(")")];
const auto rule_def = concept_load_rule | role_load_rule | sketch_rule | do_rule | call_rule | concept_choose_rule | role_choose_rule | action_rule
                      | backtrack_rule;
const auto required_rules = boost::spirit::x3::rule<class RequiredRules, std::vector<ast::Rule>> { "one or more rules" } = +rule;

const auto rule_entry_def = context(
    "rule")[(lit("(") >> keyword(":rule")) > symbol_section
            > context("rule expression")[(lit("(") >> keyword(":expression")) > source_memory_section > -target_memory_section > required_rules > lit(")")]
            > lit(")")];
const auto rules_section_def = context(":rules")[(lit("(") >> keyword(":rules")) > *rule_entry > lit(")")];

const auto named_value_def = identifier;
const auto required_memory_states = boost::spirit::x3::rule<class RequiredMemoryStates, std::vector<ast::NamedValue>> { "one or more memory states" } =
    +named_value;
const auto memory_section_def = context(":memory")[(lit("(") >> keyword(":memory")) > required_memory_states > lit(")")];

const auto module_def = context("module")[(lit("(") >> keyword(":module")) > symbol_section > arguments_section > registers_section > entry_section
                                          > memory_section > features_section > rules_section > lit(")")];
const auto required_modules = boost::spirit::x3::rule<class RequiredModules, std::vector<ast::Module>> { "one or more modules" } = +module;
const auto program_def = context("program")[(lit("(") >> keyword(":program")) > entry_section > required_modules > lit(")")];
const auto module_root_def = context("module")[module > eoi];
const auto program_root_def = context("program")[program > eoi];

BOOST_SPIRIT_DEFINE(identifier,
                    quoted_string,
                    symbol_section,
                    source_memory_section,
                    target_memory_section,
                    entry_section,
                    action_section,
                    callee_section,
                    concept_argument,
                    role_argument,
                    boolean_argument,
                    numerical_argument,
                    argument,
                    arguments_section,
                    concept_register,
                    role_register,
                    register_,
                    registers_section,
                    concept_feature,
                    role_feature,
                    boolean_feature,
                    numerical_feature,
                    query_feature,
                    feature,
                    features_section,
                    symbol_expression,
                    named_value,
                    positive_condition,
                    negative_condition,
                    equal_zero_condition,
                    greater_zero_condition,
                    condition_observation,
                    condition,
                    conditions_section,
                    positive_effect,
                    negative_effect,
                    unchanged_effect,
                    increases_effect,
                    decreases_effect,
                    effect_observation,
                    effect,
                    effects_section,
                    order_term,
                    order_section,
                    arguments_expression_section,
                    concept_load_rule,
                    role_load_rule,
                    concept_choose_rule,
                    role_choose_rule,
                    sketch_rule,
                    do_rule,
                    action_rule,
                    call_rule,
                    backtrack_rule,
                    rule,
                    rule_entry,
                    rules_section,
                    memory_section,
                    module,
                    program,
                    module_root,
                    program_root)

struct IdentifierClass : boost::spirit::x3::annotate_on_success
{
};
struct QuotedStringClass : boost::spirit::x3::annotate_on_success
{
};
struct SymbolSectionClass : boost::spirit::x3::annotate_on_success
{
};
struct SourceMemorySectionClass : boost::spirit::x3::annotate_on_success
{
};
struct TargetMemorySectionClass : boost::spirit::x3::annotate_on_success
{
};
struct EntrySectionClass : boost::spirit::x3::annotate_on_success
{
};
struct ActionSectionClass : boost::spirit::x3::annotate_on_success
{
};
struct CalleeSectionClass : boost::spirit::x3::annotate_on_success
{
};
struct ArgumentsSectionClass : boost::spirit::x3::annotate_on_success
{
};
struct RegistersSectionClass : boost::spirit::x3::annotate_on_success
{
};
struct FeaturesSectionClass : boost::spirit::x3::annotate_on_success
{
};
struct ConditionsSectionClass : boost::spirit::x3::annotate_on_success
{
};
struct EffectsSectionClass : boost::spirit::x3::annotate_on_success
{
};
struct ArgumentsExpressionSectionClass : boost::spirit::x3::annotate_on_success
{
};
struct RulesSectionClass : boost::spirit::x3::annotate_on_success
{
};
struct MemorySectionClass : boost::spirit::x3::annotate_on_success
{
};
struct ConceptArgumentClass : boost::spirit::x3::annotate_on_success
{
};
struct RoleArgumentClass : boost::spirit::x3::annotate_on_success
{
};
struct BooleanArgumentClass : boost::spirit::x3::annotate_on_success
{
};
struct NumericalArgumentClass : boost::spirit::x3::annotate_on_success
{
};
struct ArgumentClass : boost::spirit::x3::annotate_on_success
{
};
struct ConceptRegisterClass : boost::spirit::x3::annotate_on_success
{
};
struct RoleRegisterClass : boost::spirit::x3::annotate_on_success
{
};
struct RegisterClass : boost::spirit::x3::annotate_on_success
{
};
struct ConceptFeatureClass : boost::spirit::x3::annotate_on_success
{
};
struct RoleFeatureClass : boost::spirit::x3::annotate_on_success
{
};
struct BooleanFeatureClass : boost::spirit::x3::annotate_on_success
{
};
struct NumericalFeatureClass : boost::spirit::x3::annotate_on_success
{
};
struct QueryFeatureClass : boost::spirit::x3::annotate_on_success
{
};
struct FeatureClass : boost::spirit::x3::annotate_on_success
{
};
struct SymbolExpressionClass : boost::spirit::x3::annotate_on_success
{
};
struct NamedValueClass : boost::spirit::x3::annotate_on_success
{
};
struct PositiveConditionClass : boost::spirit::x3::annotate_on_success
{
};
struct NegativeConditionClass : boost::spirit::x3::annotate_on_success
{
};
struct EqualZeroConditionClass : boost::spirit::x3::annotate_on_success
{
};
struct GreaterZeroConditionClass : boost::spirit::x3::annotate_on_success
{
};
struct ConditionObservationClass : boost::spirit::x3::annotate_on_success
{
};
struct ConditionClass : boost::spirit::x3::annotate_on_success
{
};
struct PositiveEffectClass : boost::spirit::x3::annotate_on_success
{
};
struct NegativeEffectClass : boost::spirit::x3::annotate_on_success
{
};
struct UnchangedEffectClass : boost::spirit::x3::annotate_on_success
{
};
struct IncreasesEffectClass : boost::spirit::x3::annotate_on_success
{
};
struct DecreasesEffectClass : boost::spirit::x3::annotate_on_success
{
};
struct EffectObservationClass : boost::spirit::x3::annotate_on_success
{
};
struct EffectClass : boost::spirit::x3::annotate_on_success
{
};
struct ConceptLoadRuleClass : boost::spirit::x3::annotate_on_success
{
};
struct RoleLoadRuleClass : boost::spirit::x3::annotate_on_success
{
};
struct OrderTermClass : boost::spirit::x3::annotate_on_success
{
};
struct OrderSectionClass : boost::spirit::x3::annotate_on_success
{
};
struct ConceptChooseRuleClass : boost::spirit::x3::annotate_on_success
{
};
struct RoleChooseRuleClass : boost::spirit::x3::annotate_on_success
{
};
struct SketchRuleClass : boost::spirit::x3::annotate_on_success
{
};
struct DoRuleClass : boost::spirit::x3::annotate_on_success
{
};
struct CallRuleClass : boost::spirit::x3::annotate_on_success
{
};
struct ActionRuleClass : boost::spirit::x3::annotate_on_success
{
};
struct BacktrackRuleClass : boost::spirit::x3::annotate_on_success
{
};
struct RuleClass : boost::spirit::x3::annotate_on_success
{
};
struct RuleEntryClass : boost::spirit::x3::annotate_on_success
{
};
struct ModuleClass : boost::spirit::x3::annotate_on_success
{
};
struct ProgramClass : boost::spirit::x3::annotate_on_success
{
};
struct ModuleRootClass : runir::kr::parser::ErrorHandlerBase
{
};
struct ProgramRootClass : runir::kr::parser::ErrorHandlerBase
{
};

}  // namespace runir::kr::ps::ext::dl::parser

#endif
