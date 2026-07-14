#ifndef RUNIR_KR_PS_EXT_DL_PARSER_PARSERS_HPP_
#define RUNIR_KR_PS_EXT_DL_PARSER_PARSERS_HPP_

#include "runir/kr/ps/ext/dl/ast/ast.hpp"

#include <boost/spirit/home/x3.hpp>
#include <vector>

namespace runir::kr::ps::ext::dl::parser
{
namespace x3 = boost::spirit::x3;
namespace base_ast = runir::kr::ps::base::dl::ast;
namespace dl_ = runir::kr::dl;

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
struct SketchRuleClass;
struct DoRuleClass;
struct CallRuleClass;
struct RuleClass;
struct RuleEntryClass;
struct ModuleClass;
struct ModuleProgramClass;
struct ModuleRootClass;
struct ModuleProgramRootClass;

using identifier_type = x3::rule<IdentifierClass, ast::Identifier>;
using quoted_string_type = x3::rule<QuotedStringClass, ast::Identifier>;
using symbol_section_type = x3::rule<SymbolSectionClass, ast::Identifier>;
using source_memory_section_type = x3::rule<SourceMemorySectionClass, ast::Identifier>;
using target_memory_section_type = x3::rule<TargetMemorySectionClass, ast::Identifier>;
using entry_section_type = x3::rule<EntrySectionClass, ast::Identifier>;
using action_section_type = x3::rule<ActionSectionClass, ast::Identifier>;
using callee_section_type = x3::rule<CalleeSectionClass, ast::Identifier>;
using concept_argument_type = x3::rule<ConceptArgumentClass, ast::Argument<dl_::ConceptTag>>;
using role_argument_type = x3::rule<RoleArgumentClass, ast::Argument<dl_::RoleTag>>;
using boolean_argument_type = x3::rule<BooleanArgumentClass, ast::Argument<dl_::BooleanTag>>;
using numerical_argument_type = x3::rule<NumericalArgumentClass, ast::Argument<dl_::NumericalTag>>;
using argument_type = x3::rule<ArgumentClass, ast::ArgumentVariant>;
using arguments_section_type = x3::rule<ArgumentsSectionClass, std::vector<ast::ArgumentVariant>>;
using concept_register_type = x3::rule<ConceptRegisterClass, ast::Register<dl_::ConceptTag>>;
using role_register_type = x3::rule<RoleRegisterClass, ast::Register<dl_::RoleTag>>;
using register_type = x3::rule<RegisterClass, ast::RegisterVariant>;
using registers_section_type = x3::rule<RegistersSectionClass, std::vector<ast::RegisterVariant>>;
using concept_feature_type = x3::rule<ConceptFeatureClass, ast::Feature<dl_::ConceptTag>>;
using role_feature_type = x3::rule<RoleFeatureClass, ast::Feature<dl_::RoleTag>>;
using boolean_feature_type = x3::rule<BooleanFeatureClass, ast::Feature<dl_::BooleanTag>>;
using numerical_feature_type = x3::rule<NumericalFeatureClass, ast::Feature<dl_::NumericalTag>>;
using feature_type = x3::rule<FeatureClass, ast::FeatureVariant>;
using features_section_type = x3::rule<FeaturesSectionClass, std::vector<ast::FeatureVariant>>;
using symbol_expression_type = x3::rule<SymbolExpressionClass, ast::SymbolExpression>;
using named_value_type = x3::rule<NamedValueClass, ast::NamedValue>;
using positive_condition_type = x3::rule<PositiveConditionClass, base_ast::Positive>;
using negative_condition_type = x3::rule<NegativeConditionClass, base_ast::Negative>;
using equal_zero_condition_type = x3::rule<EqualZeroConditionClass, base_ast::EqualZero>;
using greater_zero_condition_type = x3::rule<GreaterZeroConditionClass, base_ast::GreaterZero>;
using condition_observation_type = x3::rule<ConditionObservationClass, base_ast::ConditionObservation<runir::kr::ExtFamilyTag>>;
using condition_type = x3::rule<ConditionClass, ast::Condition>;
using conditions_section_type = x3::rule<ConditionsSectionClass, std::vector<ast::Condition>>;
using positive_effect_type = x3::rule<PositiveEffectClass, base_ast::Positive>;
using negative_effect_type = x3::rule<NegativeEffectClass, base_ast::Negative>;
using unchanged_effect_type = x3::rule<UnchangedEffectClass, base_ast::Unchanged>;
using increases_effect_type = x3::rule<IncreasesEffectClass, base_ast::Increases>;
using decreases_effect_type = x3::rule<DecreasesEffectClass, base_ast::Decreases>;
using effect_observation_type = x3::rule<EffectObservationClass, base_ast::EffectObservation<runir::kr::ExtFamilyTag>>;
using effect_type = x3::rule<EffectClass, ast::Effect>;
using effects_section_type = x3::rule<EffectsSectionClass, std::vector<ast::Effect>>;
using arguments_expression_section_type = x3::rule<ArgumentsExpressionSectionClass, std::vector<ast::SymbolExpression>>;
using concept_load_rule_type = x3::rule<ConceptLoadRuleClass, ast::LoadRule<dl_::ConceptTag>>;
using role_load_rule_type = x3::rule<RoleLoadRuleClass, ast::LoadRule<dl_::RoleTag>>;
using sketch_rule_type = x3::rule<SketchRuleClass, ast::SketchRule>;
using do_rule_type = x3::rule<DoRuleClass, ast::DoRule>;
using call_rule_type = x3::rule<CallRuleClass, ast::CallRule>;
using rule_type = x3::rule<RuleClass, ast::Rule>;
using rule_entry_type = x3::rule<RuleEntryClass, ast::RuleEntry>;
using rules_section_type = x3::rule<RulesSectionClass, std::vector<ast::RuleEntry>>;
using memory_section_type = x3::rule<MemorySectionClass, std::vector<ast::NamedValue>>;
using module_type = x3::rule<ModuleClass, ast::Module>;
using module_program_type = x3::rule<ModuleProgramClass, ast::ModuleProgram>;
using module_root_type = x3::rule<ModuleRootClass, ast::Module>;
using module_program_root_type = x3::rule<ModuleProgramRootClass, ast::ModuleProgram>;

BOOST_SPIRIT_DECLARE(module_type, module_program_type, module_root_type, module_program_root_type)

module_type const& module_parser();
module_program_type const& module_program_parser();
module_root_type const& module_root_parser();
module_program_root_type const& module_program_root_parser();

}  // namespace runir::kr::ps::ext::dl::parser

#endif
