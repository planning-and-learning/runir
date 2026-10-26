#ifndef RUNIR_KR_PS_EXT_DL_PARSER_PARSERS_HPP_
#define RUNIR_KR_PS_EXT_DL_PARSER_PARSERS_HPP_

#include "runir/kr/ps/ext/dl/ast/ast.hpp"

#include <boost/spirit/home/x3.hpp>
#include <vector>

namespace runir::kr::ps::ext::dl::parser
{

struct IdentifierClass;
struct QuotedStringClass;
struct SymbolSectionClass;
struct SourceMemorySectionClass;
struct TargetMemorySectionClass;
struct EntrySectionClass;
struct ActionSectionClass;
struct CalleeSectionClass;
struct ArgumentsSectionClass;
struct RegistersSectionClass;
struct FeaturesSectionClass;
struct ConditionsSectionClass;
struct EffectsSectionClass;
struct ArgumentsExpressionSectionClass;
struct RulesSectionClass;
struct MemorySectionClass;
struct ConceptArgumentClass;
struct RoleArgumentClass;
struct BooleanArgumentClass;
struct NumericalArgumentClass;
struct ArgumentClass;
struct ConceptRegisterClass;
struct RoleRegisterClass;
struct RegisterClass;
struct ConceptFeatureClass;
struct RoleFeatureClass;
struct BooleanFeatureClass;
struct NumericalFeatureClass;
struct BooleanHeadFeatureClass;
struct NumericalHeadFeatureClass;
struct QueryFeatureClass;
struct FeatureClass;
struct SymbolExpressionClass;
struct NamedValueClass;
struct PositiveConditionClass;
struct NegativeConditionClass;
struct EqualZeroConditionClass;
struct GreaterZeroConditionClass;
struct ConditionObservationClass;
struct ConditionClass;
struct PositiveEffectClass;
struct NegativeEffectClass;
struct UnchangedEffectClass;
struct IncreasesEffectClass;
struct DecreasesEffectClass;
struct EffectObservationClass;
struct EffectClass;
struct ConceptLoadRuleClass;
struct RoleLoadRuleClass;
struct ConceptChooseRuleClass;
struct RoleChooseRuleClass;
struct OrderTermClass;
struct OrderSectionClass;
struct SketchRuleClass;
struct DoRuleClass;
struct CallRuleClass;
struct ActionRuleClass;
struct BacktrackRuleClass;
struct RuleClass;
struct RuleEntryClass;
struct DatalogVariableClass;
struct DatalogTermClass;
struct DatalogAtomClass;
struct DatalogGoalAtomClass;
struct DatalogNegationClass;
struct DatalogComparisonClass;
struct DatalogArithmeticClass;
struct DatalogConditionClass;
struct DatalogAggregateClass;
struct DatalogLiteralClass;
struct DatalogRuleClass;
struct DatalogHeadClass;
struct DatalogSectionClass;
struct ModuleClass;
struct ProgramClass;
struct ModuleRootClass;
struct ProgramRootClass;

using identifier_type = boost::spirit::x3::rule<IdentifierClass, ast::Identifier>;
using quoted_string_type = boost::spirit::x3::rule<QuotedStringClass, ast::Identifier>;
using symbol_section_type = boost::spirit::x3::rule<SymbolSectionClass, ast::Identifier>;
using source_memory_section_type = boost::spirit::x3::rule<SourceMemorySectionClass, ast::Identifier>;
using target_memory_section_type = boost::spirit::x3::rule<TargetMemorySectionClass, ast::Identifier>;
using entry_section_type = boost::spirit::x3::rule<EntrySectionClass, ast::Identifier>;
using action_section_type = boost::spirit::x3::rule<ActionSectionClass, ast::Identifier>;
using callee_section_type = boost::spirit::x3::rule<CalleeSectionClass, ast::Identifier>;
using concept_argument_type = boost::spirit::x3::rule<ConceptArgumentClass, ast::Argument<runir::kr::dl::ConceptTag>>;
using role_argument_type = boost::spirit::x3::rule<RoleArgumentClass, ast::Argument<runir::kr::dl::RoleTag>>;
using boolean_argument_type = boost::spirit::x3::rule<BooleanArgumentClass, ast::Argument<runir::kr::dl::BooleanTag>>;
using numerical_argument_type = boost::spirit::x3::rule<NumericalArgumentClass, ast::Argument<runir::kr::dl::NumericalTag>>;
using argument_type = boost::spirit::x3::rule<ArgumentClass, ast::ArgumentVariant>;
using arguments_section_type = boost::spirit::x3::rule<ArgumentsSectionClass, std::vector<ast::ArgumentVariant>>;
using concept_register_type = boost::spirit::x3::rule<ConceptRegisterClass, ast::Register<runir::kr::dl::ConceptTag>>;
using role_register_type = boost::spirit::x3::rule<RoleRegisterClass, ast::Register<runir::kr::dl::RoleTag>>;
using register_type = boost::spirit::x3::rule<RegisterClass, ast::RegisterVariant>;
using registers_section_type = boost::spirit::x3::rule<RegistersSectionClass, std::vector<ast::RegisterVariant>>;
using concept_feature_type = boost::spirit::x3::rule<ConceptFeatureClass, ast::Feature<runir::kr::dl::ConceptTag>>;
using role_feature_type = boost::spirit::x3::rule<RoleFeatureClass, ast::Feature<runir::kr::dl::RoleTag>>;
using boolean_feature_type = boost::spirit::x3::rule<BooleanFeatureClass, ast::Feature<runir::kr::dl::BooleanTag>>;
using numerical_feature_type = boost::spirit::x3::rule<NumericalFeatureClass, ast::Feature<runir::kr::dl::NumericalTag>>;
using boolean_head_feature_type = boost::spirit::x3::rule<BooleanHeadFeatureClass, ast::HeadFeature<runir::kr::dl::BooleanTag>>;
using numerical_head_feature_type = boost::spirit::x3::rule<NumericalHeadFeatureClass, ast::HeadFeature<runir::kr::dl::NumericalTag>>;
using query_feature_type = boost::spirit::x3::rule<QueryFeatureClass, ast::QueryFeature>;
using feature_type = boost::spirit::x3::rule<FeatureClass, ast::FeatureVariant>;
using features_section_type = boost::spirit::x3::rule<FeaturesSectionClass, std::vector<ast::FeatureVariant>>;
using symbol_expression_type = boost::spirit::x3::rule<SymbolExpressionClass, ast::SymbolExpression>;
using named_value_type = boost::spirit::x3::rule<NamedValueClass, ast::NamedValue>;
using positive_condition_type = boost::spirit::x3::rule<PositiveConditionClass, runir::kr::ps::base::dl::ast::Positive>;
using negative_condition_type = boost::spirit::x3::rule<NegativeConditionClass, runir::kr::ps::base::dl::ast::Negative>;
using equal_zero_condition_type = boost::spirit::x3::rule<EqualZeroConditionClass, runir::kr::ps::base::dl::ast::EqualZero>;
using greater_zero_condition_type = boost::spirit::x3::rule<GreaterZeroConditionClass, runir::kr::ps::base::dl::ast::GreaterZero>;
using condition_observation_type = boost::spirit::x3::rule<ConditionObservationClass, runir::kr::ps::base::dl::ast::ConditionObservation>;
using condition_type = boost::spirit::x3::rule<ConditionClass, ast::Condition>;
using conditions_section_type = boost::spirit::x3::rule<ConditionsSectionClass, std::vector<ast::Condition>>;
using positive_effect_type = boost::spirit::x3::rule<PositiveEffectClass, runir::kr::ps::base::dl::ast::Positive>;
using negative_effect_type = boost::spirit::x3::rule<NegativeEffectClass, runir::kr::ps::base::dl::ast::Negative>;
using unchanged_effect_type = boost::spirit::x3::rule<UnchangedEffectClass, runir::kr::ps::base::dl::ast::Unchanged>;
using increases_effect_type = boost::spirit::x3::rule<IncreasesEffectClass, runir::kr::ps::base::dl::ast::Increases>;
using decreases_effect_type = boost::spirit::x3::rule<DecreasesEffectClass, runir::kr::ps::base::dl::ast::Decreases>;
using effect_observation_type = boost::spirit::x3::rule<EffectObservationClass, runir::kr::ps::base::dl::ast::EffectObservation>;
using effect_type = boost::spirit::x3::rule<EffectClass, ast::Effect>;
using effects_section_type = boost::spirit::x3::rule<EffectsSectionClass, std::vector<ast::Effect>>;
using arguments_expression_section_type = boost::spirit::x3::rule<ArgumentsExpressionSectionClass, std::vector<ast::SymbolExpression>>;
using concept_load_rule_type = boost::spirit::x3::rule<ConceptLoadRuleClass, ast::LoadRule<runir::kr::dl::ConceptTag>>;
using role_load_rule_type = boost::spirit::x3::rule<RoleLoadRuleClass, ast::LoadRule<runir::kr::dl::RoleTag>>;
using concept_choose_rule_type = boost::spirit::x3::rule<ConceptChooseRuleClass, ast::ChooseRule<runir::kr::dl::ConceptTag>>;
using role_choose_rule_type = boost::spirit::x3::rule<RoleChooseRuleClass, ast::ChooseRule<runir::kr::dl::RoleTag>>;
using order_term_type = boost::spirit::x3::rule<OrderTermClass, ast::OrderTerm>;
using order_section_type = boost::spirit::x3::rule<OrderSectionClass, std::vector<ast::OrderTerm>>;
using sketch_rule_type = boost::spirit::x3::rule<SketchRuleClass, ast::SketchRule>;
using do_rule_type = boost::spirit::x3::rule<DoRuleClass, ast::DoRule>;
using call_rule_type = boost::spirit::x3::rule<CallRuleClass, ast::CallRule>;
using action_rule_type = boost::spirit::x3::rule<ActionRuleClass, ast::ActionRule>;
using backtrack_rule_type = boost::spirit::x3::rule<BacktrackRuleClass, ast::BacktrackRule>;
using rule_type = boost::spirit::x3::rule<RuleClass, ast::Rule>;
using rule_entry_type = boost::spirit::x3::rule<RuleEntryClass, ast::RuleEntry>;
using rules_section_type = boost::spirit::x3::rule<RulesSectionClass, std::vector<ast::RuleEntry>>;
using memory_section_type = boost::spirit::x3::rule<MemorySectionClass, std::vector<ast::NamedValue>>;
using datalog_variable_type = boost::spirit::x3::rule<DatalogVariableClass, ast::Identifier>;
using datalog_term_type = boost::spirit::x3::rule<DatalogTermClass, ast::Identifier>;
using datalog_atom_type = boost::spirit::x3::rule<DatalogAtomClass, ast::DatalogAtom>;
using datalog_goal_atom_type = boost::spirit::x3::rule<DatalogGoalAtomClass, ast::DatalogGoalAtom>;
using datalog_negation_type = boost::spirit::x3::rule<DatalogNegationClass, ast::DatalogNegation>;
using datalog_comparison_type = boost::spirit::x3::rule<DatalogComparisonClass, ast::DatalogComparison>;
using datalog_arithmetic_type = boost::spirit::x3::rule<DatalogArithmeticClass, ast::DatalogArithmetic>;
using datalog_condition_type = boost::spirit::x3::rule<DatalogConditionClass, ast::DatalogCondition>;
using datalog_aggregate_type = boost::spirit::x3::rule<DatalogAggregateClass, ast::DatalogAggregate>;
using datalog_literal_type = boost::spirit::x3::rule<DatalogLiteralClass, ast::DatalogLiteral>;
using datalog_rule_type = boost::spirit::x3::rule<DatalogRuleClass, ast::DatalogRule>;
using datalog_section_type = boost::spirit::x3::rule<DatalogSectionClass, std::vector<ast::DatalogRule>>;
using datalog_head_type = boost::spirit::x3::rule<DatalogHeadClass, ast::DatalogHead>;
using module_type = boost::spirit::x3::rule<ModuleClass, ast::Module>;
using program_type = boost::spirit::x3::rule<ProgramClass, ast::Program>;
using module_root_type = boost::spirit::x3::rule<ModuleRootClass, ast::Module>;
using program_root_type = boost::spirit::x3::rule<ProgramRootClass, ast::Program>;

BOOST_SPIRIT_DECLARE(module_type, program_type, module_root_type, program_root_type)

module_type const& module_parser();
program_type const& program_parser();
module_root_type const& module_root_parser();
program_root_type const& program_root_parser();

}  // namespace runir::kr::ps::ext::dl::parser

#endif
