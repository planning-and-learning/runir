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
using boost::spirit::x3::ascii::digit;

struct IdentifierText
{
};
struct QuotedStringText
{
};
struct DatalogVariableText
{
};
struct DatalogTermText
{
};

constexpr identifier_type identifier = "identifier";
constexpr quoted_string_type quoted_string = "quoted string";
constexpr symbol_section_type symbol_section = ":symbol";
constexpr source_memory_section_type source_memory_section = ":source-memory";
constexpr target_memory_section_type target_memory_section = ":target-memory";
constexpr entry_section_type entry_section = ":entry";
constexpr action_section_type action_section = ":action";
constexpr callee_section_type callee_section = ":callee";
constexpr concept_argument_type concept_argument = "concept_argument";
constexpr role_argument_type role_argument = "role_argument";
constexpr boolean_argument_type boolean_argument = "boolean_argument";
constexpr numerical_argument_type numerical_argument = "numerical_argument";
constexpr argument_type argument = "argument";
constexpr arguments_section_type arguments_section = ":arguments";
constexpr concept_register_type concept_register = "concept_register";
constexpr role_register_type role_register = "role_register";
constexpr register_type register_ = "register";
constexpr registers_section_type registers_section = ":registers";
constexpr concept_feature_type concept_feature = "concept_feature";
constexpr role_feature_type role_feature = "role_feature";
constexpr boolean_feature_type boolean_feature = "boolean_feature";
constexpr numerical_feature_type numerical_feature = "numerical_feature";
constexpr concept_head_feature_type concept_head_feature = "concept_head_feature";
constexpr role_head_feature_type role_head_feature = "role_head_feature";
constexpr boolean_head_feature_type boolean_head_feature = "boolean_head_feature";
constexpr numerical_head_feature_type numerical_head_feature = "numerical_head_feature";
constexpr query_feature_type query_feature = "query_feature";
constexpr feature_type feature = "feature";
constexpr features_section_type features_section = ":features";
constexpr symbol_expression_type symbol_expression = "symbol_expression";
constexpr named_value_type named_value = "named_value";
constexpr positive_condition_type positive_condition = "positive_condition";
constexpr negative_condition_type negative_condition = "negative_condition";
constexpr equal_zero_condition_type equal_zero_condition = "equal_zero_condition";
constexpr greater_zero_condition_type greater_zero_condition = "greater_zero_condition";
constexpr condition_observation_type condition_observation = "condition_observation";
constexpr condition_type condition = "condition";
constexpr conditions_section_type conditions_section = ":conditions";
constexpr positive_effect_type positive_effect = "positive_effect";
constexpr negative_effect_type negative_effect = "negative_effect";
constexpr unchanged_effect_type unchanged_effect = "unchanged_effect";
constexpr increases_effect_type increases_effect = "increases_effect";
constexpr decreases_effect_type decreases_effect = "decreases_effect";
constexpr effect_observation_type effect_observation = "effect_observation";
constexpr effect_type effect = "effect";
constexpr order_term_type order_term = "order term";
constexpr order_section_type order_section = ":order";
constexpr effects_section_type effects_section = ":effects";
constexpr arguments_expression_section_type arguments_expression_section = ":arguments";
constexpr concept_load_rule_type concept_load_rule = "concept_load_rule";
constexpr role_load_rule_type role_load_rule = "role_load_rule";
constexpr concept_choose_rule_type concept_choose_rule = "concept_choose_rule";
constexpr role_choose_rule_type role_choose_rule = "role_choose_rule";
constexpr sketch_rule_type sketch_rule = "sketch_rule";
constexpr do_rule_type do_rule = "do_rule";
constexpr call_rule_type call_rule = "call_rule";
constexpr action_rule_type action_rule = "action_rule";
constexpr backtrack_rule_type backtrack_rule = "backtrack_rule";
constexpr rule_type rule = "rule";
constexpr rule_entry_type rule_entry = "rule_entry";
constexpr rules_section_type rules_section = ":rules";
constexpr memory_section_type memory_section = ":memory";
constexpr module_type module = "module";
constexpr program_type program = "program";
constexpr module_root_type module_root = "module_root";
constexpr program_root_type program_root = "program_root";
constexpr datalog_variable_type datalog_variable = "Datalog variable";
constexpr datalog_term_type datalog_term = "Datalog term";
constexpr datalog_atom_type datalog_atom = "Datalog atom";
constexpr datalog_goal_atom_type datalog_goal_atom = "Datalog goal atom";
constexpr datalog_negation_type datalog_negation = "Datalog negation";
constexpr datalog_comparison_type datalog_comparison = "Datalog comparison";
constexpr datalog_arithmetic_type datalog_arithmetic = "Datalog arithmetic";
constexpr datalog_condition_type datalog_condition = "Datalog condition";
constexpr datalog_aggregate_type datalog_aggregate = "Datalog aggregate";
constexpr datalog_literal_type datalog_literal = "Datalog literal";
constexpr datalog_rule_type datalog_rule = "Datalog rule";
constexpr datalog_head_type datalog_head = "Datalog head";
constexpr datalog_section_type datalog_section = ":datalog";

constexpr auto identifier_def = boost::spirit::x3::rule<IdentifierText, std::string> { "identifier_text" } =
    raw[lexeme[(alpha | char_('_')) >> *(alnum | char_('_') | char_('-'))]];
constexpr auto quoted_string_def = boost::spirit::x3::rule<QuotedStringText, std::string> { "quoted_string_text" } =
    lexeme[context("quoted string")[(lit('"') >> *((lit('\\') >> char_) | (char_ - '"'))) > lit('"')]];

constexpr auto symbol_section_def = context(":symbol")[(lit("(") >> keyword(":symbol")) > identifier > lit(")")];
constexpr auto source_memory_section_def = context(":source-memory")[(lit("(") >> keyword(":source-memory")) > identifier > lit(")")];
constexpr auto target_memory_section_def = context(":target-memory")[(lit("(") >> keyword(":target-memory")) > identifier > lit(")")];
constexpr auto entry_section_def = context(":entry")[(lit("(") >> keyword(":entry")) > identifier > lit(")")];
constexpr auto action_section_def = context(":action")[(lit("(") >> keyword(":action")) > quoted_string > lit(")")];
constexpr auto callee_section_def = context(":callee")[(lit("(") >> keyword(":callee")) > identifier > lit(")")];

constexpr auto concept_argument_def = context("concept argument")[keyword("(:concept") > identifier > lit(")")];
constexpr auto role_argument_def = context("role argument")[keyword("(:role") > identifier > lit(")")];
constexpr auto boolean_argument_def = context("boolean argument")[keyword("(:boolean") > identifier > lit(")")];
constexpr auto numerical_argument_def = context("numerical argument")[keyword("(:numerical") > identifier > lit(")")];
constexpr auto argument_def = concept_argument | role_argument | boolean_argument | numerical_argument;
constexpr auto arguments_section_def = context(":arguments")[(lit("(") >> keyword(":arguments")) > *argument > lit(")")];

constexpr auto concept_register_def = context("concept register")[keyword("(:concept") > identifier > lit(")")];
constexpr auto role_register_def = context("role register")[keyword("(:role") > identifier > lit(")")];
constexpr auto register__def = concept_register | role_register;
constexpr auto registers_section_def = context(":registers")[(lit("(") >> keyword(":registers")) > *register_ > lit(")")];

constexpr auto concept_feature_def = context(
    "concept feature")[keyword("(:concept") > symbol_section
                       > context(":expression")[(lit("(") >> keyword(":expression")) > runir::kr::dl::grammar::parser::dl_concept > lit(")")] > lit(")")];
constexpr auto role_feature_def =
    context("role feature")[keyword("(:role") > symbol_section
                            > context(":expression")[(lit("(") >> keyword(":expression")) > runir::kr::dl::grammar::parser::dl_role > lit(")")] > lit(")")];
constexpr auto boolean_feature_def = context(
    "boolean feature")[keyword("(:boolean") > symbol_section
                       > context(":expression")[(lit("(") >> keyword(":expression")) > runir::kr::dl::grammar::parser::dl_boolean > lit(")")] > lit(")")];
constexpr auto numerical_feature_def = context(
    "numerical feature")[keyword("(:numerical") > symbol_section
                         > context(":expression")[(lit("(") >> keyword(":expression")) > runir::kr::dl::grammar::parser::dl_numerical > lit(")")] > lit(")")];
constexpr auto query_feature_def =
    context("query feature")[keyword("(:query") > symbol_section
                             > context(":expression")[(lit("(") >> keyword(":expression")) > runir::kr::dl::grammar::parser::dl_query > lit(")")] > lit(")")];
constexpr auto concept_head_feature_def =
    context("concept feature")[(keyword("(:concept") >> symbol_section >> lit("(") >> keyword(":head")) > identifier > lit(")") > lit(")")];
constexpr auto role_head_feature_def =
    context("role feature")[(keyword("(:role") >> symbol_section >> lit("(") >> keyword(":head")) > identifier > lit(")") > lit(")")];
constexpr auto boolean_head_feature_def =
    context("boolean feature")[(keyword("(:boolean") >> symbol_section >> lit("(") >> keyword(":head")) > identifier > lit(")") > lit(")")];
constexpr auto numerical_head_feature_def =
    context("numerical feature")[(keyword("(:numerical") >> symbol_section >> lit("(") >> keyword(":head")) > identifier > lit(")") > lit(")")];
constexpr auto feature_def = concept_head_feature | concept_feature | role_head_feature | role_feature | boolean_head_feature | boolean_feature
                             | numerical_head_feature | numerical_feature | query_feature;
constexpr auto features_section_def = context(":features")[(lit("(") >> keyword(":features")) > *feature > lit(")")];

constexpr auto symbol_expression_def = identifier;
constexpr auto concept_feature_section_def = context("concept feature reference")[keyword("(:concept") > identifier > lit(")")];
constexpr auto role_feature_section_def = context("role feature reference")[keyword("(:role") > identifier > lit(")")];
constexpr auto query_feature_section_def = context("query feature reference")[keyword("(:query") > identifier > lit(")")];
constexpr auto concept_register_section_def =
    context("concept register reference")[(lit("(") >> keyword(":register") >> keyword("(:concept")) > identifier > lit(")") > lit(")")];
constexpr auto role_register_section_def =
    context("role register reference")[(lit("(") >> keyword(":register") >> keyword("(:role")) > identifier > lit(")") > lit(")")];
constexpr auto arguments_expression_section_def = context(":arguments")[(lit("(") >> keyword(":arguments")) > *symbol_expression > lit(")")];

constexpr auto positive_condition_def = keyword(runir::kr::ps::dl::Positive::keyword) >> attr(runir::kr::ps::base::dl::ast::Positive {});
constexpr auto negative_condition_def = keyword(runir::kr::ps::dl::Negative::keyword) >> attr(runir::kr::ps::base::dl::ast::Negative {});
constexpr auto equal_zero_condition_def = keyword(runir::kr::ps::dl::EqualZero::keyword) >> attr(runir::kr::ps::base::dl::ast::EqualZero {});
constexpr auto greater_zero_condition_def = keyword(runir::kr::ps::dl::GreaterZero::keyword) >> attr(runir::kr::ps::base::dl::ast::GreaterZero {});
constexpr auto condition_observation_def = positive_condition | negative_condition | equal_zero_condition | greater_zero_condition;
constexpr auto condition_def = context("condition")[(lit("(") >> condition_observation) > identifier > lit(")")];
constexpr auto conditions_section_def = context(":conditions")[(lit("(") >> keyword(":conditions")) > *condition > lit(")")];

constexpr auto positive_effect_def = keyword(runir::kr::ps::dl::Positive::keyword) >> attr(runir::kr::ps::base::dl::ast::Positive {});
constexpr auto negative_effect_def = keyword(runir::kr::ps::dl::Negative::keyword) >> attr(runir::kr::ps::base::dl::ast::Negative {});
constexpr auto unchanged_effect_def = keyword(runir::kr::ps::dl::Unchanged::keyword) >> attr(runir::kr::ps::base::dl::ast::Unchanged {});
constexpr auto increases_effect_def = keyword(runir::kr::ps::dl::Increases::keyword) >> attr(runir::kr::ps::base::dl::ast::Increases {});
constexpr auto decreases_effect_def = keyword(runir::kr::ps::dl::Decreases::keyword) >> attr(runir::kr::ps::base::dl::ast::Decreases {});
constexpr auto effect_observation_def = positive_effect | negative_effect | unchanged_effect | increases_effect | decreases_effect;
constexpr auto effect_def = context("effect")[(lit("(") >> effect_observation) > identifier > lit(")")];
constexpr auto effects_section_def = context(":effects")[(lit("(") >> keyword(":effects")) > *effect > lit(")")];
constexpr auto optional_effects_section = effects_section | attr(std::vector<ast::Effect> {});

constexpr auto order_term_def =
    context("order term")[lit("(") > ((keyword("min") >> attr(OrderDirection::MIN)) | (keyword("max") >> attr(OrderDirection::MAX))) > identifier > lit(")")];
constexpr auto order_section_def = context(":order")[(lit("(") >> keyword(":order")) > *order_term > lit(")")];
constexpr auto optional_order_section = order_section | attr(std::vector<ast::OrderTerm> {});

constexpr auto concept_load_rule_def = context("concept load rule")[(keyword("(:load") >> conditions_section >> concept_feature_section_def)
                                                                    > concept_register_section_def > optional_effects_section > lit(")")];
constexpr auto role_load_rule_def = context(
    "role load rule")[(keyword("(:load") >> conditions_section >> role_feature_section_def) > role_register_section_def > optional_effects_section > lit(")")];
constexpr auto concept_choose_rule_def =
    context("concept choose rule")[(keyword("(:choose") >> conditions_section >> concept_feature_section_def) > concept_register_section_def
                                   > optional_effects_section > optional_order_section > lit(")")];
constexpr auto role_choose_rule_def = context("role choose rule")[(keyword("(:choose") >> conditions_section >> role_feature_section_def)
                                                                  > role_register_section_def > optional_effects_section > optional_order_section > lit(")")];
constexpr auto sketch_rule_def = context("sketch rule")[keyword("(:sketch") > conditions_section > effects_section > lit(")")];
constexpr auto do_rule_def =
    context("action rule")[keyword("(:do") > conditions_section > action_section > arguments_expression_section > effects_section > lit(")")];
constexpr auto action_rule_def =
    context("query action rule")[keyword("(:action") > conditions_section > action_section > query_feature_section_def > effects_section > lit(")")];
constexpr auto call_rule_def = context("call rule")[keyword("(:call") > conditions_section > callee_section > arguments_expression_section > lit(")")];
constexpr auto backtrack_rule_def = context("backtrack rule")[keyword("(:backtrack") > conditions_section > lit(")")];
constexpr auto rule_def =
    concept_load_rule | role_load_rule | sketch_rule | do_rule | call_rule | concept_choose_rule | role_choose_rule | action_rule | backtrack_rule;
constexpr auto required_rules = boost::spirit::x3::rule<class RequiredRules, std::vector<ast::Rule>> { "one or more rules" } = +rule;

constexpr auto rule_entry_def = context(
    "rule")[(lit("(") >> keyword(":rule")) > symbol_section
            > context("rule expression")[(lit("(") >> keyword(":expression")) > source_memory_section > -target_memory_section > required_rules > lit(")")]
            > lit(")")];
constexpr auto rules_section_def = context(":rules")[(lit("(") >> keyword(":rules")) > *rule_entry > lit(")")];

constexpr auto named_value_def = identifier;
constexpr auto required_memory_states = boost::spirit::x3::rule<class RequiredMemoryStates, std::vector<ast::NamedValue>> { "one or more memory states" } =
    +named_value;
constexpr auto memory_section_def = context(":memory")[(lit("(") >> keyword(":memory")) > required_memory_states > lit(")")];

// The :datalog section: (:rule head literal...), literals as in docs/datalog.md. Aggregate bodies hold no aggregates.
constexpr auto datalog_variable_def = boost::spirit::x3::rule<DatalogVariableText, std::string> { "variable_text" } =
    raw[lexeme[char_('?') >> (alpha | char_('_')) >> *(alnum | char_('_') | char_('-'))]];
constexpr auto datalog_term_def = boost::spirit::x3::rule<DatalogTermText, std::string> { "term_text" } =
    raw[lexeme[(-char_('?') >> (alpha | char_('_')) >> *(alnum | char_('_') | char_('-'))) | +digit]];
constexpr auto datalog_atom_def = ((lit("(") >> identifier) > *datalog_term) > lit(")");
constexpr auto datalog_goal_atom_def = (((lit("(") >> keyword("goal")) > identifier) > *datalog_term) > lit(")");
constexpr auto datalog_negation_def = ((lit("(") >> keyword("not")) > datalog_atom) > lit(")");
constexpr auto datalog_comparison_operator =
    (lit("!=") >> attr(ast::DatalogComparisonOperator::NOT_EQUAL)) | (lit("<=") >> attr(ast::DatalogComparisonOperator::LESS_EQUAL))
    | (lit(">=") >> attr(ast::DatalogComparisonOperator::GREATER_EQUAL)) | (lit("<") >> attr(ast::DatalogComparisonOperator::LESS))
    | (lit(">") >> attr(ast::DatalogComparisonOperator::GREATER)) | (lit("=") >> attr(ast::DatalogComparisonOperator::EQUAL));
constexpr auto datalog_comparison_def = (((lit("(") >> datalog_comparison_operator) > datalog_term) > datalog_term) > lit(")");
constexpr auto datalog_arithmetic_operator =
    (lit("+") >> attr(ast::DatalogArithmeticOperator::ADD)) | (lit("-") >> attr(ast::DatalogArithmeticOperator::SUBTRACT))
    | (lit("*") >> attr(ast::DatalogArithmeticOperator::MULTIPLY)) | (lit("/") >> attr(ast::DatalogArithmeticOperator::DIVIDE))
    | (keyword("min") >> attr(ast::DatalogArithmeticOperator::MIN)) | (keyword("max") >> attr(ast::DatalogArithmeticOperator::MAX));
// (= ?result (op lhs rhs)); backtracks to a comparison (= lhs rhs) before the inner parenthesis.
constexpr auto datalog_arithmetic_def =
    (((((lit("(") >> lit("=") >> datalog_variable >> lit("(")) > datalog_arithmetic_operator) > datalog_term) > datalog_term) > lit(")")) > lit(")");
constexpr auto datalog_condition_def = datalog_negation | datalog_goal_atom | datalog_arithmetic | datalog_comparison | datalog_atom;
constexpr auto datalog_aggregate_function =
    (keyword("count") >> attr(ast::DatalogAggregateFunction::COUNT)) | (keyword("sum") >> attr(ast::DatalogAggregateFunction::SUM))
    | (keyword("min") >> attr(ast::DatalogAggregateFunction::MIN)) | (keyword("max") >> attr(ast::DatalogAggregateFunction::MAX));
constexpr auto datalog_aggregate_body = (lit("(") > *datalog_condition) > lit(")");
constexpr auto datalog_aggregate_def = ((((lit("(") >> datalog_aggregate_function) > datalog_variable) > -datalog_variable) > datalog_aggregate_body)
                                       > lit(")");
constexpr auto datalog_literal_def = datalog_aggregate | datalog_condition;
constexpr auto datalog_head_def = (((lit("(") >> identifier) > *datalog_term) > -((lit("(") >> keyword("min")) > datalog_term > lit(")"))) > lit(")");
constexpr auto datalog_rule_def = context("Datalog rule")[(((lit("(") >> keyword(":rule")) > datalog_head) > *datalog_literal) > lit(")")];
constexpr auto datalog_section_def = context(":datalog")[((lit("(") >> keyword(":datalog")) > *datalog_rule) > lit(")")];

constexpr auto module_def = context("module")[(lit("(") >> keyword(":module")) > symbol_section > arguments_section > registers_section > entry_section
                                              > memory_section > -datalog_section > features_section > rules_section > lit(")")];
constexpr auto required_modules = boost::spirit::x3::rule<class RequiredModules, std::vector<ast::Module>> { "one or more modules" } = +module;
constexpr auto program_def = context("program")[(lit("(") >> keyword(":program")) > entry_section > required_modules > lit(")")];
constexpr auto module_root_def = context("module")[module > eoi];
constexpr auto program_root_def = context("program")[program > eoi];

BOOST_SPIRIT_DEFINE(datalog_variable,
                    datalog_term,
                    datalog_atom,
                    datalog_goal_atom,
                    datalog_negation,
                    datalog_comparison,
                    datalog_arithmetic,
                    datalog_condition,
                    datalog_aggregate,
                    datalog_literal,
                    datalog_rule,
                    datalog_head,
                    datalog_section)
BOOST_SPIRIT_DEFINE(concept_head_feature, role_head_feature, boolean_head_feature, numerical_head_feature)

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
struct ConceptHeadFeatureClass : boost::spirit::x3::annotate_on_success
{
};
struct RoleHeadFeatureClass : boost::spirit::x3::annotate_on_success
{
};
struct BooleanHeadFeatureClass : boost::spirit::x3::annotate_on_success
{
};
struct NumericalHeadFeatureClass : boost::spirit::x3::annotate_on_success
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
struct DatalogVariableClass : boost::spirit::x3::annotate_on_success
{
};
struct DatalogTermClass : boost::spirit::x3::annotate_on_success
{
};
struct DatalogAtomClass : boost::spirit::x3::annotate_on_success
{
};
struct DatalogGoalAtomClass : boost::spirit::x3::annotate_on_success
{
};
struct DatalogNegationClass : boost::spirit::x3::annotate_on_success
{
};
struct DatalogComparisonClass : boost::spirit::x3::annotate_on_success
{
};
struct DatalogConditionClass : boost::spirit::x3::annotate_on_success
{
};
struct DatalogAggregateClass : boost::spirit::x3::annotate_on_success
{
};
struct DatalogLiteralClass : boost::spirit::x3::annotate_on_success
{
};
struct DatalogRuleClass : boost::spirit::x3::annotate_on_success
{
};
struct DatalogSectionClass : boost::spirit::x3::annotate_on_success
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
