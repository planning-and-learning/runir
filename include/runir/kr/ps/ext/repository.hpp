#ifndef RUNIR_KR_PS_EXT_REPOSITORY_HPP_
#define RUNIR_KR_PS_EXT_REPOSITORY_HPP_

#include "runir/kr/dl/ext/repository.hpp"
#include "runir/kr/ps/base/repository.hpp"
#include "runir/kr/ps/ext/canonicalization.hpp"
#include "runir/kr/ps/ext/condition_data.hpp"
#include "runir/kr/ps/ext/condition_view.hpp"
#include "runir/kr/ps/ext/dl/feature_data.hpp"
#include "runir/kr/ps/ext/dl/feature_view.hpp"
#include "runir/kr/ps/ext/effect_data.hpp"
#include "runir/kr/ps/ext/effect_view.hpp"
#include "runir/kr/ps/ext/feature_data.hpp"
#include "runir/kr/ps/ext/feature_view.hpp"
#include "runir/kr/ps/ext/memory_state_data.hpp"
#include "runir/kr/ps/ext/memory_state_view.hpp"
#include "runir/kr/ps/ext/module_data.hpp"
#include "runir/kr/ps/ext/module_view.hpp"
#include "runir/kr/ps/ext/register_data.hpp"
#include "runir/kr/ps/ext/register_view.hpp"
#include "runir/kr/ps/ext/rule_data.hpp"
#include "runir/kr/ps/ext/rule_variant_data.hpp"
#include "runir/kr/ps/ext/rule_variant_view.hpp"
#include "runir/kr/ps/ext/rule_view.hpp"

#include <tyr/common/type_list.hpp>

namespace runir::kr::ps::ext
{

using ConcreteRuleTypes = tyr::MapTypeListT<Rule, tyr::TypeList<LoadTag, SketchTag, DoTag, CallTag>>;
using RuleTypes = tyr::ConcatTypeListsT<tyr::TypeList<RuleVariant>, ConcreteRuleTypes>;
using FeatureTypes = tyr::TypeList<Feature<runir::kr::dl::ConceptTag>,
                                   Feature<runir::kr::ps::dl::BooleanFeature>,
                                   Feature<runir::kr::ps::dl::NumericalFeature>,
                                   ConcreteFeature<runir::kr::DlTag, runir::kr::dl::ConceptTag>,
                                   ConcreteFeature<runir::kr::DlTag, runir::kr::ps::dl::BooleanFeature>,
                                   ConcreteFeature<runir::kr::DlTag, runir::kr::ps::dl::NumericalFeature>>;
using ConditionTypes = tyr::TypeList<ConditionVariant>;
using EffectTypes = tyr::TypeList<EffectVariant>;
using ProgramTypes = tyr::TypeList<Register, MemoryState, Module>;
using RepositoryTypes = tyr::ConcatTypeListsT<runir::kr::ps::base::RepositoryTypes, FeatureTypes, ConditionTypes, EffectTypes, RuleTypes, ProgramTypes>;
using Repository = runir::kr::ps::BasicRepository<runir::kr::ExtFamilyTag, RepositoryTypes, runir::kr::dl::ext::ConstructorRepositoryPtr>;
using RepositoryFactory = runir::kr::ps::BasicRepositoryFactory<runir::kr::ExtFamilyTag, RepositoryTypes, runir::kr::dl::ext::ConstructorRepositoryPtr>;

using RegisterView = tyr::View<tyr::Index<Register>, Repository>;
using MemoryStateView = tyr::View<tyr::Index<MemoryState>, Repository>;
using ModuleView = tyr::View<tyr::Index<Module>, Repository>;
using RuleVariantView = tyr::View<tyr::Index<RuleVariant>, Repository>;

template<RuleKind Kind>
using RuleView = tyr::View<tyr::Index<Rule<Kind>>, Repository>;

}  // namespace runir::kr::ps::ext

#endif
