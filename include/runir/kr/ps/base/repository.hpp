#ifndef RUNIR_KR_PS_BASE_REPOSITORY_HPP_
#define RUNIR_KR_PS_BASE_REPOSITORY_HPP_

#include "runir/kr/dl/base/repository.hpp"
#include "runir/kr/ps/base/canonicalization.hpp"
#include "runir/kr/ps/base/condition_data.hpp"
#include "runir/kr/ps/base/declarations.hpp"
#include "runir/kr/ps/base/dl/condition_data.hpp"
#include "runir/kr/ps/base/dl/condition_view.hpp"
#include "runir/kr/ps/base/dl/effect_data.hpp"
#include "runir/kr/ps/base/dl/effect_view.hpp"
#include "runir/kr/ps/base/dl/feature_data.hpp"
#include "runir/kr/ps/base/dl/feature_view.hpp"
#include "runir/kr/ps/base/effect_data.hpp"
#include "runir/kr/ps/base/rule_data.hpp"
#include "runir/kr/ps/base/rule_view.hpp"
#include "runir/kr/ps/base/sketch_data.hpp"
#include "runir/kr/ps/base/sketch_view.hpp"
#include "runir/kr/ps/condition_view.hpp"
#include "runir/kr/ps/declarations.hpp"
#include "runir/kr/ps/effect_view.hpp"
#include "runir/kr/ps/feature_view.hpp"
#include "runir/kr/ps/repository.hpp"

#include <memory>
#include <tyr/common/type_list.hpp>

namespace runir::kr::ps::base
{

using FeatureTypes = tyr::TypeList<runir::kr::ps::Feature<runir::kr::BaseFamilyTag, runir::kr::ps::dl::BooleanFeature>,
                                   runir::kr::ps::Feature<runir::kr::BaseFamilyTag, runir::kr::ps::dl::NumericalFeature>,
                                   runir::kr::ps::ConcreteFeature<runir::kr::BaseFamilyTag, runir::kr::DlTag, runir::kr::ps::dl::BooleanFeature>,
                                   runir::kr::ps::ConcreteFeature<runir::kr::BaseFamilyTag, runir::kr::DlTag, runir::kr::ps::dl::NumericalFeature>>;

using ConditionTypes = tyr::TypeList<
    runir::kr::ps::ConditionVariant<runir::kr::BaseFamilyTag>,
    runir::kr::ps::ConcreteConditionVariant<runir::kr::BaseFamilyTag, runir::kr::DlTag>,
    runir::kr::ps::ConcreteCondition<runir::kr::BaseFamilyTag, runir::kr::DlTag, runir::kr::ps::dl::BooleanFeature, runir::kr::ps::dl::Positive>,
    runir::kr::ps::ConcreteCondition<runir::kr::BaseFamilyTag, runir::kr::DlTag, runir::kr::ps::dl::BooleanFeature, runir::kr::ps::dl::Negative>,
    runir::kr::ps::ConcreteCondition<runir::kr::BaseFamilyTag, runir::kr::DlTag, runir::kr::ps::dl::NumericalFeature, runir::kr::ps::dl::EqualZero>,
    runir::kr::ps::ConcreteCondition<runir::kr::BaseFamilyTag, runir::kr::DlTag, runir::kr::ps::dl::NumericalFeature, runir::kr::ps::dl::GreaterZero>>;

using EffectTypes =
    tyr::TypeList<runir::kr::ps::EffectVariant<runir::kr::BaseFamilyTag>,
                  runir::kr::ps::ConcreteEffectVariant<runir::kr::BaseFamilyTag, runir::kr::DlTag>,
                  runir::kr::ps::ConcreteEffect<runir::kr::BaseFamilyTag, runir::kr::DlTag, runir::kr::ps::dl::BooleanFeature, runir::kr::ps::dl::Positive>,
                  runir::kr::ps::ConcreteEffect<runir::kr::BaseFamilyTag, runir::kr::DlTag, runir::kr::ps::dl::BooleanFeature, runir::kr::ps::dl::Negative>,
                  runir::kr::ps::ConcreteEffect<runir::kr::BaseFamilyTag, runir::kr::DlTag, runir::kr::ps::dl::BooleanFeature, runir::kr::ps::dl::Unchanged>,
                  runir::kr::ps::ConcreteEffect<runir::kr::BaseFamilyTag, runir::kr::DlTag, runir::kr::ps::dl::NumericalFeature, runir::kr::ps::dl::Increases>,
                  runir::kr::ps::ConcreteEffect<runir::kr::BaseFamilyTag, runir::kr::DlTag, runir::kr::ps::dl::NumericalFeature, runir::kr::ps::dl::Decreases>,
                  runir::kr::ps::ConcreteEffect<runir::kr::BaseFamilyTag, runir::kr::DlTag, runir::kr::ps::dl::NumericalFeature, runir::kr::ps::dl::Unchanged>>;

using SketchTypes = tyr::TypeList<runir::kr::ps::base::Rule, runir::kr::ps::base::Sketch>;
using RepositoryTypes = tyr::ConcatTypeListsT<FeatureTypes, ConditionTypes, EffectTypes, SketchTypes>;
using Repository = runir::kr::ps::BasicRepository<runir::kr::BaseFamilyTag, RepositoryTypes, runir::kr::dl::base::ConstructorRepositoryPtr>;
using RepositoryPtr = std::shared_ptr<Repository>;
using RepositoryFactory = runir::kr::ps::BasicRepositoryFactory<runir::kr::BaseFamilyTag, RepositoryTypes, runir::kr::dl::base::ConstructorRepositoryPtr>;

using SketchView = tyr::View<tyr::Index<runir::kr::ps::base::Sketch>, Repository>;
using RuleView = tyr::View<tyr::Index<runir::kr::ps::base::Rule>, Repository>;

}  // namespace runir::kr::ps::base

#endif
