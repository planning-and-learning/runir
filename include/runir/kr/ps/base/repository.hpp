#ifndef RUNIR_KR_PS_BASE_REPOSITORY_HPP_
#define RUNIR_KR_PS_BASE_REPOSITORY_HPP_

#include "runir/kr/dl/repository.hpp"
#include "runir/kr/ps/base/condition_data.hpp"
#include "runir/kr/ps/base/declarations.hpp"
#include "runir/kr/ps/base/dl/condition_data.hpp"
#include "runir/kr/ps/base/dl/condition_view.hpp"
#include "runir/kr/ps/base/dl/effect_data.hpp"
#include "runir/kr/ps/base/dl/effect_view.hpp"
#include "runir/kr/ps/base/dl/feature_data.hpp"
#include "runir/kr/ps/base/dl/feature_view.hpp"
#include "runir/kr/ps/base/effect_data.hpp"
#include "runir/kr/ps/base/feature_data.hpp"
#include "runir/kr/ps/condition_view.hpp"
#include "runir/kr/ps/effect_view.hpp"
#include "runir/kr/ps/feature_view.hpp"
#include "runir/kr/ps/repository.hpp"
#include "runir/kr/ps/rule_data.hpp"
#include "runir/kr/ps/rule_view.hpp"
#include "runir/kr/ps/sketch_data.hpp"
#include "runir/kr/ps/sketch_view.hpp"

#include <tyr/common/type_list.hpp>
#include <tyr/formalism/symbol_repository.hpp>

namespace runir::kr::ps::base
{

using FeatureTypes = tyr::TypeList<Feature<dl::BooleanFeature>,
                                   Feature<dl::NumericalFeature>,
                                   ConcreteFeature<runir::kr::DlTag, dl::BooleanFeature>,
                                   ConcreteFeature<runir::kr::DlTag, dl::NumericalFeature>>;

using ConditionTypes = tyr::TypeList<ConditionVariant,
                                     ConcreteConditionVariant<runir::kr::DlTag>,
                                     ConcreteCondition<runir::kr::DlTag, dl::BooleanFeature, dl::Positive>,
                                     ConcreteCondition<runir::kr::DlTag, dl::BooleanFeature, dl::Negative>,
                                     ConcreteCondition<runir::kr::DlTag, dl::NumericalFeature, dl::EqualZero>,
                                     ConcreteCondition<runir::kr::DlTag, dl::NumericalFeature, dl::GreaterZero>>;

using EffectTypes = tyr::TypeList<EffectVariant,
                                  ConcreteEffectVariant<runir::kr::DlTag>,
                                  ConcreteEffect<runir::kr::DlTag, dl::BooleanFeature, dl::Positive>,
                                  ConcreteEffect<runir::kr::DlTag, dl::BooleanFeature, dl::Negative>,
                                  ConcreteEffect<runir::kr::DlTag, dl::BooleanFeature, dl::Unchanged>,
                                  ConcreteEffect<runir::kr::DlTag, dl::NumericalFeature, dl::Increases>,
                                  ConcreteEffect<runir::kr::DlTag, dl::NumericalFeature, dl::Decreases>,
                                  ConcreteEffect<runir::kr::DlTag, dl::NumericalFeature, dl::Unchanged>>;

using SketchTypes = tyr::TypeList<Rule, Sketch>;
using RepositoryTypes = tyr::ConcatTypeListsT<FeatureTypes, ConditionTypes, EffectTypes, SketchTypes>;
using SymbolRepository = tyr::ApplyTypeListT<tyr::formalism::SymbolRepository, RepositoryTypes>;

using Repository = runir::kr::ps::BasicRepository<FamilyTag, RepositoryTypes, runir::kr::dl::ConstructorRepositoryPtr>;
using RepositoryPtr = std::shared_ptr<Repository>;
using RepositoryFactory = runir::kr::ps::BasicRepositoryFactory<FamilyTag, RepositoryTypes, runir::kr::dl::ConstructorRepositoryPtr>;
using RepositoryFactoryPtr = std::shared_ptr<RepositoryFactory>;

using SketchView = tyr::View<tyr::Index<Sketch>, Repository>;
using RuleView = tyr::View<tyr::Index<Rule>, Repository>;

}  // namespace runir::kr::ps::base

#endif
