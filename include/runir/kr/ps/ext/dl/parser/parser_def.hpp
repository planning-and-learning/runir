#ifndef RUNIR_KR_PS_EXT_DL_PARSER_PARSER_DEF_HPP_
#define RUNIR_KR_PS_EXT_DL_PARSER_PARSER_DEF_HPP_

#include "runir/kr/dl/grammar/parser/parsers.hpp"
#include "runir/kr/parser/error_handler.hpp"
#include "runir/kr/ps/base/dl/ast/ast_adapted.hpp"
#include "runir/kr/ps/ext/dl/ast/ast_adapted.hpp"
#include "runir/kr/ps/ext/dl/parser/parsers.hpp"

#include <boost/spirit/home/x3/support/utility/annotate_on_success.hpp>
#include <string>

namespace runir::kr::ps::ext::dl::parser
{
namespace x3 = boost::spirit::x3;
namespace dl_parser = runir::kr::dl::grammar::parser;

using x3::attr;
using x3::eoi;
using x3::lexeme;
using x3::lit;
using x3::omit;
using x3::raw;

using x3::ascii::alnum;
using x3::ascii::alpha;
using x3::ascii::char_;

struct IdentifierText
{
};
struct QuotedStringText
{
};

identifier_type const identifier = "identifier";
quoted_string_type const quoted_string = "quoted_string";
symbol_section_type const symbol_section = "symbol_section";
source_memory_section_type const source_memory_section = "source_memory_section";
target_memory_section_type const target_memory_section = "target_memory_section";
entry_section_type const entry_section = "entry_section";
action_section_type const action_section = "action_section";
callee_section_type const callee_section = "callee_section";
concept_argument_type const concept_argument = "concept_argument";
role_argument_type const role_argument = "role_argument";
boolean_argument_type const boolean_argument = "boolean_argument";
numerical_argument_type const numerical_argument = "numerical_argument";
argument_type const argument = "argument";
arguments_section_type const arguments_section = "arguments_section";
concept_register_type const concept_register = "concept_register";
role_register_type const role_register = "role_register";
register_type const register_ = "register";
registers_section_type const registers_section = "registers_section";
concept_feature_type const concept_feature = "concept_feature";
role_feature_type const role_feature = "role_feature";
boolean_feature_type const boolean_feature = "boolean_feature";
numerical_feature_type const numerical_feature = "numerical_feature";
feature_type const feature = "feature";
features_section_type const features_section = "features_section";
symbol_expression_type const symbol_expression = "symbol_expression";
concept_load_expression_type const concept_load_expression = "concept_load_expression";
role_load_expression_type const role_load_expression = "role_load_expression";
named_value_type const named_value = "named_value";
positive_condition_type const positive_condition = "positive_condition";
negative_condition_type const negative_condition = "negative_condition";
equal_zero_condition_type const equal_zero_condition = "equal_zero_condition";
greater_zero_condition_type const greater_zero_condition = "greater_zero_condition";
condition_observation_type const condition_observation = "condition_observation";
condition_type const condition = "condition";
conditions_section_type const conditions_section = "conditions_section";
positive_effect_type const positive_effect = "positive_effect";
negative_effect_type const negative_effect = "negative_effect";
unchanged_effect_type const unchanged_effect = "unchanged_effect";
increases_effect_type const increases_effect = "increases_effect";
decreases_effect_type const decreases_effect = "decreases_effect";
effect_observation_type const effect_observation = "effect_observation";
effect_type const effect = "effect";
effects_section_type const effects_section = "effects_section";
arguments_expression_section_type const arguments_expression_section = "arguments_expression_section";
concept_load_rule_type const concept_load_rule = "concept_load_rule";
role_load_rule_type const role_load_rule = "role_load_rule";
sketch_rule_type const sketch_rule = "sketch_rule";
do_rule_type const do_rule = "do_rule";
call_rule_type const call_rule = "call_rule";
rule_type const rule = "rule";
rule_entry_type const rule_entry = "rule_entry";
rules_section_type const rules_section = "rules_section";
memory_section_type const memory_section = "memory_section";
module_type const module = "module";
module_program_type const module_program = "module_program";
module_root_type const module_root = "module_root";
module_program_root_type const module_program_root = "module_program_root";

const auto identifier_def = x3::rule<IdentifierText, std::string> { "identifier_text" } =
    raw[lexeme[(alpha | char_('_')) >> *(alnum | char_('_') | char_('-'))]];
const auto quoted_string_def = x3::rule<QuotedStringText, std::string> { "quoted_string_text" } =
    lexeme[lit('"') >> *((lit('\\') >> char_) | (char_ - '"')) >> lit('"')];

const auto symbol_section_def = lit("(") > lit(":symbol") > identifier > lit(")");
const auto source_memory_section_def = lit("(") > lit(":source-memory") > identifier > lit(")");
const auto target_memory_section_def = lit("(") > lit(":target-memory") > identifier > lit(")");
const auto entry_section_def = lit("(") > lit(":entry") > identifier > lit(")");
const auto action_section_def = lit("(") > lit(":action") > quoted_string > lit(")");
const auto callee_section_def = lit("(") > lit(":callee") > identifier > lit(")");

const auto concept_argument_def = omit[lit("(:concept")] >> identifier >> omit[lit(")")];
const auto role_argument_def = omit[lit("(:role")] >> identifier >> omit[lit(")")];
const auto boolean_argument_def = omit[lit("(:boolean")] >> identifier >> omit[lit(")")];
const auto numerical_argument_def = omit[lit("(:numerical")] >> identifier >> omit[lit(")")];
const auto argument_def = concept_argument | role_argument | boolean_argument | numerical_argument;
const auto arguments_section_def = lit("(") > lit(":arguments") > *argument > lit(")");

const auto concept_register_def = omit[lit("(:concept")] >> identifier >> omit[lit(")")];
const auto role_register_def = omit[lit("(:role")] >> identifier >> omit[lit(")")];
const auto register__def = concept_register | role_register;
const auto registers_section_def = lit("(") > lit(":registers") > *register_ > lit(")");

const auto concept_feature_def = omit[lit("(:concept")] >> symbol_section >> omit[lit("(") > lit(":expression")]
                                 >> dl_parser::concept_parser<runir::kr::ExtFamilyTag>() >> omit[lit(")") >> lit(")")];
const auto role_feature_def = omit[lit("(:role")] >> symbol_section >> omit[lit("(") > lit(":expression")] >> dl_parser::role_parser<runir::kr::ExtFamilyTag>()
                              >> omit[lit(")") >> lit(")")];
const auto boolean_feature_def = omit[lit("(:boolean")] >> symbol_section >> omit[lit("(") > lit(":expression")]
                                 >> dl_parser::boolean_parser<runir::kr::ExtFamilyTag>() >> omit[lit(")") >> lit(")")];
const auto numerical_feature_def = omit[lit("(:numerical")] >> symbol_section >> omit[lit("(") > lit(":expression")]
                                   >> dl_parser::numerical_parser<runir::kr::ExtFamilyTag>() >> omit[lit(")") >> lit(")")];
const auto feature_def = concept_feature | role_feature | boolean_feature | numerical_feature;
const auto features_section_def = lit("(") > lit(":features") > *feature > lit(")");

const auto symbol_expression_def = identifier;
const auto concept_load_expression_def = dl_parser::concept_parser<runir::kr::ExtFamilyTag>() | symbol_expression;
const auto role_load_expression_def = dl_parser::role_parser<runir::kr::ExtFamilyTag>() | symbol_expression;
const auto concept_expression_section_def = omit[lit("(:concept")] >> concept_load_expression >> omit[lit(")")];
const auto role_expression_section_def = omit[lit("(:role")] >> role_load_expression >> omit[lit(")")];
const auto concept_register_section_def = omit[lit("(") >> lit(":register") >> lit("(:concept")] >> identifier >> omit[lit(")") >> lit(")")];
const auto role_register_section_def = omit[lit("(") >> lit(":register") >> lit("(:role")] >> identifier >> omit[lit(")") >> lit(")")];
const auto arguments_expression_section_def = lit("(") > lit(":arguments") > *symbol_expression > lit(")");

const auto positive_condition_def = lit(base_ast::Positive::keyword) >> attr(base_ast::Positive {});
const auto negative_condition_def = lit(base_ast::Negative::keyword) >> attr(base_ast::Negative {});
const auto equal_zero_condition_def = lit(base_ast::EqualZero::keyword) >> attr(base_ast::EqualZero {});
const auto greater_zero_condition_def = lit(base_ast::GreaterZero::keyword) >> attr(base_ast::GreaterZero {});
const auto condition_observation_def = positive_condition | negative_condition | equal_zero_condition | greater_zero_condition;
const auto condition_def = omit[lit("(")] >> condition_observation >> identifier >> omit[lit(")")];
const auto conditions_section_def = lit("(") > lit(":conditions") > *condition > lit(")");

const auto positive_effect_def = lit(base_ast::Positive::keyword) >> attr(base_ast::Positive {});
const auto negative_effect_def = lit(base_ast::Negative::keyword) >> attr(base_ast::Negative {});
const auto unchanged_effect_def = lit(base_ast::Unchanged::keyword) >> attr(base_ast::Unchanged {});
const auto increases_effect_def = lit(base_ast::Increases::keyword) >> attr(base_ast::Increases {});
const auto decreases_effect_def = lit(base_ast::Decreases::keyword) >> attr(base_ast::Decreases {});
const auto effect_observation_def = positive_effect | negative_effect | unchanged_effect | increases_effect | decreases_effect;
const auto effect_def = omit[lit("(")] >> effect_observation >> identifier >> omit[lit(")")];
const auto effects_section_def = lit("(") > lit(":effects") > *effect > lit(")");

const auto concept_load_rule_def =
    omit[lit("(:load")] >> conditions_section >> concept_expression_section_def >> concept_register_section_def >> omit[lit(")")];
const auto role_load_rule_def = omit[lit("(:load")] >> conditions_section >> role_expression_section_def >> role_register_section_def >> omit[lit(")")];
const auto sketch_rule_def = omit[lit("(:sketch")] >> conditions_section >> effects_section >> omit[lit(")")];
const auto do_rule_def = omit[lit("(:do")] >> conditions_section >> action_section >> arguments_expression_section >> effects_section >> omit[lit(")")];
const auto call_rule_def = omit[lit("(:call")] >> conditions_section >> callee_section >> arguments_expression_section >> omit[lit(")")];
const auto rule_def = concept_load_rule | role_load_rule | sketch_rule | do_rule | call_rule;

const auto rule_entry_def = (lit("(") >> lit(":rule")) > symbol_section > lit("(") > lit(":expression") > source_memory_section > target_memory_section > +rule
                            > lit(")") > lit(")");
const auto rules_section_def = lit("(") > lit(":rules") > *rule_entry > lit(")");

const auto named_value_def = identifier;
const auto memory_section_def = lit("(") > lit(":memory") > +named_value > lit(")");

const auto module_def = (lit("(") >> lit(":module")) > symbol_section > arguments_section > registers_section > entry_section > memory_section
                        > features_section > rules_section > lit(")");
const auto module_program_def = (lit("(") >> lit(":program")) > entry_section > +module > lit(")");
const auto module_root_def = module > eoi;
const auto module_program_root_def = module_program > eoi;

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
                    feature,
                    features_section,
                    symbol_expression,
                    concept_load_expression,
                    role_load_expression,
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
                    arguments_expression_section,
                    concept_load_rule,
                    role_load_rule,
                    sketch_rule,
                    do_rule,
                    call_rule,
                    rule,
                    rule_entry,
                    rules_section,
                    memory_section,
                    module,
                    module_program,
                    module_root,
                    module_program_root)

struct IdentifierClass : x3::annotate_on_success
{
};
struct QuotedStringClass : x3::annotate_on_success
{
};
struct SymbolSectionClass : x3::annotate_on_success
{
};
struct SourceMemorySectionClass : x3::annotate_on_success
{
};
struct TargetMemorySectionClass : x3::annotate_on_success
{
};
struct EntrySectionClass : x3::annotate_on_success
{
};
struct ActionSectionClass : x3::annotate_on_success
{
};
struct CalleeSectionClass : x3::annotate_on_success
{
};
struct ArgumentsSectionClass : x3::annotate_on_success
{
};
struct RegistersSectionClass : x3::annotate_on_success
{
};
struct FeaturesSectionClass : x3::annotate_on_success
{
};
struct ConditionsSectionClass : x3::annotate_on_success
{
};
struct EffectsSectionClass : x3::annotate_on_success
{
};
struct ArgumentsExpressionSectionClass : x3::annotate_on_success
{
};
struct RulesSectionClass : x3::annotate_on_success
{
};
struct MemorySectionClass : x3::annotate_on_success
{
};
struct ConceptArgumentClass : x3::annotate_on_success
{
};
struct RoleArgumentClass : x3::annotate_on_success
{
};
struct BooleanArgumentClass : x3::annotate_on_success
{
};
struct NumericalArgumentClass : x3::annotate_on_success
{
};
struct ArgumentClass : x3::annotate_on_success
{
};
struct ConceptRegisterClass : x3::annotate_on_success
{
};
struct RoleRegisterClass : x3::annotate_on_success
{
};
struct RegisterClass : x3::annotate_on_success
{
};
struct ConceptFeatureClass : x3::annotate_on_success
{
};
struct RoleFeatureClass : x3::annotate_on_success
{
};
struct BooleanFeatureClass : x3::annotate_on_success
{
};
struct NumericalFeatureClass : x3::annotate_on_success
{
};
struct FeatureClass : x3::annotate_on_success
{
};
struct SymbolExpressionClass : x3::annotate_on_success
{
};
template<runir::kr::dl::CategoryTag Category>
struct LoadExpressionClass : x3::annotate_on_success
{
};
struct NamedValueClass : x3::annotate_on_success
{
};
struct PositiveConditionClass : x3::annotate_on_success
{
};
struct NegativeConditionClass : x3::annotate_on_success
{
};
struct EqualZeroConditionClass : x3::annotate_on_success
{
};
struct GreaterZeroConditionClass : x3::annotate_on_success
{
};
struct ConditionObservationClass : x3::annotate_on_success
{
};
struct ConditionClass : x3::annotate_on_success
{
};
struct PositiveEffectClass : x3::annotate_on_success
{
};
struct NegativeEffectClass : x3::annotate_on_success
{
};
struct UnchangedEffectClass : x3::annotate_on_success
{
};
struct IncreasesEffectClass : x3::annotate_on_success
{
};
struct DecreasesEffectClass : x3::annotate_on_success
{
};
struct EffectObservationClass : x3::annotate_on_success
{
};
struct EffectClass : x3::annotate_on_success
{
};
struct ConceptLoadRuleClass : x3::annotate_on_success
{
};
struct RoleLoadRuleClass : x3::annotate_on_success
{
};
struct SketchRuleClass : x3::annotate_on_success
{
};
struct DoRuleClass : x3::annotate_on_success
{
};
struct CallRuleClass : x3::annotate_on_success
{
};
struct RuleClass : x3::annotate_on_success
{
};
struct RuleEntryClass : x3::annotate_on_success
{
};
struct ModuleClass : x3::annotate_on_success
{
};
struct ModuleProgramClass : x3::annotate_on_success
{
};
struct ModuleRootClass : runir::kr::parser::ErrorHandlerBase
{
};
struct ModuleProgramRootClass : runir::kr::parser::ErrorHandlerBase
{
};

}  // namespace runir::kr::ps::ext::dl::parser

#endif
