#ifndef RUNIR_KR_PS_EXT_REPOSITORY_HPP_
#define RUNIR_KR_PS_EXT_REPOSITORY_HPP_

#include "runir/kr/dl/repository.hpp"
#include "runir/kr/ps/base/repository.hpp"
#include "runir/kr/ps/condition_view.hpp"
#include "runir/kr/ps/effect_view.hpp"
#include "runir/kr/ps/ext/argument_data.hpp"
#include "runir/kr/ps/ext/argument_view.hpp"
#include "runir/kr/ps/ext/canonicalization.hpp"
#include "runir/kr/ps/ext/condition_data.hpp"
#include "runir/kr/ps/ext/dl/condition_view.hpp"
#include "runir/kr/ps/ext/dl/effect_view.hpp"
#include "runir/kr/ps/ext/dl/feature_data.hpp"
#include "runir/kr/ps/ext/dl/feature_view.hpp"
#include "runir/kr/ps/ext/effect_data.hpp"
#include "runir/kr/ps/ext/feature_data.hpp"
#include "runir/kr/ps/ext/memory_state_data.hpp"
#include "runir/kr/ps/ext/memory_state_view.hpp"
#include "runir/kr/ps/ext/module_data.hpp"
#include "runir/kr/ps/ext/module_program_data.hpp"
#include "runir/kr/ps/ext/module_program_view.hpp"
#include "runir/kr/ps/ext/module_view.hpp"
#include "runir/kr/ps/ext/register_data.hpp"
#include "runir/kr/ps/ext/register_view.hpp"
#include "runir/kr/ps/ext/rule_data.hpp"
#include "runir/kr/ps/ext/rule_variant_data.hpp"
#include "runir/kr/ps/ext/rule_variant_view.hpp"
#include "runir/kr/ps/ext/rule_view.hpp"
#include "runir/kr/ps/family_traits.hpp"
#include "runir/kr/ps/feature_view.hpp"

#include <concepts>
#include <memory>
#include <yggdrasil/core/type_list.hpp>

namespace runir::kr::ps::ext
{

using LoadRuleTypes = ygg::TypeList<Rule<LoadTag<runir::kr::dl::ConceptTag>>, Rule<LoadTag<runir::kr::dl::RoleTag>>>;
using ControlRuleTypes = ygg::MapTypeListT<Rule, ygg::TypeList<SketchTag, DoTag, CallTag>>;
using ConcreteRuleTypes = ygg::ConcatTypeListsT<LoadRuleTypes, ControlRuleTypes>;
using RuleTypes = ygg::ConcatTypeListsT<ygg::TypeList<RuleVariant>, ConcreteRuleTypes>;
using FeatureTypes = runir::kr::ps::PsFeatureTypes<runir::kr::ExtFamilyTag>;
using ConditionTypes = runir::kr::ps::PsConditionTypes<runir::kr::ExtFamilyTag>;
using EffectTypes = runir::kr::ps::PsEffectTypes<runir::kr::ExtFamilyTag>;
using ArgumentTypes = ygg::
    TypeList<Argument<runir::kr::dl::ConceptTag>, Argument<runir::kr::dl::RoleTag>, Argument<runir::kr::dl::BooleanTag>, Argument<runir::kr::dl::NumericalTag>>;
using RegisterTypes = ygg::TypeList<Register<runir::kr::dl::ConceptTag>, Register<runir::kr::dl::RoleTag>>;
using ProgramTypes = ygg::TypeList<MemoryState, Module, ModuleProgram>;
using RepositoryTypes = ygg::
    ConcatTypeListsT<runir::kr::ps::base::RepositoryTypes, FeatureTypes, ConditionTypes, EffectTypes, RuleTypes, ArgumentTypes, RegisterTypes, ProgramTypes>;
using Repository =
    runir::kr::ps::BasicRepository<runir::kr::ExtFamilyTag, RepositoryTypes, runir::kr::dl::ConstructorRepositoryPtrFor<runir::kr::ExtFamilyTag>>;
using RepositoryPtr = std::shared_ptr<Repository>;
using RepositoryFactory =
    runir::kr::ps::BasicRepositoryFactory<runir::kr::ExtFamilyTag, RepositoryTypes, runir::kr::dl::ConstructorRepositoryPtrFor<runir::kr::ExtFamilyTag>>;

using ConceptRegisterView = ygg::View<ygg::Index<Register<runir::kr::dl::ConceptTag>>, Repository>;
using RoleRegisterView = ygg::View<ygg::Index<Register<runir::kr::dl::RoleTag>>, Repository>;
using MemoryStateView = ygg::View<ygg::Index<MemoryState>, Repository>;
using ModuleView = ygg::View<ygg::Index<Module>, Repository>;
using ModuleProgramView = ygg::View<ygg::Index<ModuleProgram>, Repository>;
using RuleVariantView = ygg::View<ygg::Index<RuleVariant>, Repository>;

template<RuleKind Kind>
using RuleView = ygg::View<ygg::Index<Rule<Kind>>, Repository>;

template<typename R>
concept LoadRuleView = std::same_as<R, RuleView<LoadTag<runir::kr::dl::ConceptTag>>> || std::same_as<R, RuleView<LoadTag<runir::kr::dl::RoleTag>>>;

}  // namespace runir::kr::ps::ext

#endif
