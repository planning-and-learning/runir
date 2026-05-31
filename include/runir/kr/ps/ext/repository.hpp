#ifndef RUNIR_KR_PS_EXT_REPOSITORY_HPP_
#define RUNIR_KR_PS_EXT_REPOSITORY_HPP_

#include "runir/kr/dl/ext/repository.hpp"
#include "runir/kr/ps/base/repository.hpp"
#include "runir/kr/ps/ext/argument_data.hpp"
#include "runir/kr/ps/ext/argument_view.hpp"
#include "runir/kr/ps/ext/canonicalization.hpp"
#include "runir/kr/ps/ext/condition_data.hpp"
#include "runir/kr/ps/ext/condition_view.hpp"
#include "runir/kr/ps/ext/dl/condition_view.hpp"
#include "runir/kr/ps/ext/dl/effect_view.hpp"
#include "runir/kr/ps/ext/dl/feature_data.hpp"
#include "runir/kr/ps/ext/dl/feature_view.hpp"
#include "runir/kr/ps/ext/effect_data.hpp"
#include "runir/kr/ps/ext/effect_view.hpp"
#include "runir/kr/ps/ext/feature_data.hpp"
#include "runir/kr/ps/ext/feature_view.hpp"
#include "runir/kr/ps/ext/memory_state_data.hpp"
#include "runir/kr/ps/ext/memory_state_view.hpp"
#include "runir/kr/ps/ext/memory_transition_data.hpp"
#include "runir/kr/ps/ext/memory_transition_view.hpp"
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

#include <memory>
#include <yggdrasil/core/type_list.hpp>

namespace runir::kr::ps::ext
{

using ConcreteRuleTypes = ygg::MapTypeListT<Rule, ygg::TypeList<LoadTag, SketchTag, DoTag, CallTag>>;
using RuleTypes = ygg::ConcatTypeListsT<ygg::TypeList<RuleVariant>, ConcreteRuleTypes>;
using FeatureTypes = ygg::TypeList<Feature<runir::kr::dl::ConceptTag>,
                                   Feature<runir::kr::ps::dl::BooleanFeature>,
                                   Feature<runir::kr::ps::dl::NumericalFeature>,
                                   ConcreteFeature<runir::kr::DlTag, runir::kr::dl::ConceptTag>,
                                   ConcreteFeature<runir::kr::DlTag, runir::kr::ps::dl::BooleanFeature>,
                                   ConcreteFeature<runir::kr::DlTag, runir::kr::ps::dl::NumericalFeature>>;
using ConditionTypes = ygg::TypeList<ConditionVariant,
                                     ConcreteConditionVariant<runir::kr::DlTag>,
                                     ConcreteCondition<runir::kr::DlTag, runir::kr::dl::ConceptTag, runir::kr::ps::dl::EqualZero>,
                                     ConcreteCondition<runir::kr::DlTag, runir::kr::dl::ConceptTag, runir::kr::ps::dl::GreaterZero>,
                                     ConcreteCondition<runir::kr::DlTag, runir::kr::ps::dl::BooleanFeature, runir::kr::ps::dl::Positive>,
                                     ConcreteCondition<runir::kr::DlTag, runir::kr::ps::dl::BooleanFeature, runir::kr::ps::dl::Negative>,
                                     ConcreteCondition<runir::kr::DlTag, runir::kr::ps::dl::NumericalFeature, runir::kr::ps::dl::EqualZero>,
                                     ConcreteCondition<runir::kr::DlTag, runir::kr::ps::dl::NumericalFeature, runir::kr::ps::dl::GreaterZero>>;
using EffectTypes = ygg::TypeList<EffectVariant,
                                  ConcreteEffectVariant<runir::kr::DlTag>,
                                  ConcreteEffect<runir::kr::DlTag, runir::kr::dl::ConceptTag, runir::kr::ps::dl::Increases>,
                                  ConcreteEffect<runir::kr::DlTag, runir::kr::dl::ConceptTag, runir::kr::ps::dl::Decreases>,
                                  ConcreteEffect<runir::kr::DlTag, runir::kr::dl::ConceptTag, runir::kr::ps::dl::Unchanged>,
                                  ConcreteEffect<runir::kr::DlTag, runir::kr::ps::dl::BooleanFeature, runir::kr::ps::dl::Positive>,
                                  ConcreteEffect<runir::kr::DlTag, runir::kr::ps::dl::BooleanFeature, runir::kr::ps::dl::Negative>,
                                  ConcreteEffect<runir::kr::DlTag, runir::kr::ps::dl::BooleanFeature, runir::kr::ps::dl::Unchanged>,
                                  ConcreteEffect<runir::kr::DlTag, runir::kr::ps::dl::NumericalFeature, runir::kr::ps::dl::Increases>,
                                  ConcreteEffect<runir::kr::DlTag, runir::kr::ps::dl::NumericalFeature, runir::kr::ps::dl::Decreases>,
                                  ConcreteEffect<runir::kr::DlTag, runir::kr::ps::dl::NumericalFeature, runir::kr::ps::dl::Unchanged>>;
using ArgumentTypes = ygg::TypeList<Argument<runir::kr::dl::ConceptTag>, Argument<runir::kr::dl::RoleTag>, Argument<runir::kr::dl::BooleanTag>, Argument<runir::kr::dl::NumericalTag>>;
using ProgramTypes = ygg::TypeList<Register, MemoryState, MemoryTransition, Module, ModuleProgram>;
using RepositoryTypes =
    ygg::ConcatTypeListsT<runir::kr::ps::base::RepositoryTypes, FeatureTypes, ConditionTypes, EffectTypes, RuleTypes, ArgumentTypes, ProgramTypes>;
using Repository = runir::kr::ps::BasicRepository<runir::kr::ExtFamilyTag, RepositoryTypes, runir::kr::dl::ext::ConstructorRepositoryPtr>;
using RepositoryPtr = std::shared_ptr<Repository>;
using RepositoryFactory = runir::kr::ps::BasicRepositoryFactory<runir::kr::ExtFamilyTag, RepositoryTypes, runir::kr::dl::ext::ConstructorRepositoryPtr>;

using RegisterView = ygg::View<ygg::Index<Register>, Repository>;
using MemoryStateView = ygg::View<ygg::Index<MemoryState>, Repository>;
using MemoryTransitionView = ygg::View<ygg::Index<MemoryTransition>, Repository>;
using ModuleView = ygg::View<ygg::Index<Module>, Repository>;
using ModuleProgramView = ygg::View<ygg::Index<ModuleProgram>, Repository>;
using RuleVariantView = ygg::View<ygg::Index<RuleVariant>, Repository>;

template<RuleKind Kind>
using RuleView = ygg::View<ygg::Index<Rule<Kind>>, Repository>;

}  // namespace runir::kr::ps::ext

#endif
