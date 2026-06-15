#ifndef RUNIR_KR_PS_UNS_REPOSITORY_HPP_
#define RUNIR_KR_PS_UNS_REPOSITORY_HPP_

#include "runir/kr/dl/uns/repository.hpp"
#include "runir/kr/ps/uns/canonicalization.hpp"
#include "runir/kr/ps/uns/condition_data.hpp"
#include "runir/kr/ps/uns/declarations.hpp"
#include "runir/kr/ps/uns/dl/condition_data.hpp"
#include "runir/kr/ps/uns/dl/condition_view.hpp"
#include "runir/kr/ps/uns/dl/effect_data.hpp"
#include "runir/kr/ps/uns/dl/effect_view.hpp"
#include "runir/kr/ps/uns/dl/feature_data.hpp"
#include "runir/kr/ps/uns/dl/feature_view.hpp"
#include "runir/kr/ps/uns/effect_data.hpp"
#include "runir/kr/ps/uns/rule_data.hpp"
#include "runir/kr/ps/uns/rule_view.hpp"
#include "runir/kr/ps/uns/sketch_data.hpp"
#include "runir/kr/ps/uns/sketch_view.hpp"
#include "runir/kr/ps/condition_view.hpp"
#include "runir/kr/ps/declarations.hpp"
#include "runir/kr/ps/effect_view.hpp"
#include "runir/kr/ps/feature_view.hpp"
#include "runir/kr/ps/repository.hpp"

#include <memory>
#include <yggdrasil/core/type_list.hpp>

namespace runir::kr::ps::uns
{

using FeatureTypes = ygg::TypeList<runir::kr::ps::Feature<runir::kr::UnsFamilyTag, runir::kr::ps::dl::BooleanFeature>,
                                   runir::kr::ps::Feature<runir::kr::UnsFamilyTag, runir::kr::ps::dl::NumericalFeature>,
                                   runir::kr::ps::ConcreteFeature<runir::kr::UnsFamilyTag, runir::kr::DlTag, runir::kr::ps::dl::BooleanFeature>,
                                   runir::kr::ps::ConcreteFeature<runir::kr::UnsFamilyTag, runir::kr::DlTag, runir::kr::ps::dl::NumericalFeature>>;

using ConditionTypes = ygg::TypeList<
    runir::kr::ps::ConditionVariant<runir::kr::UnsFamilyTag>,
    runir::kr::ps::ConcreteConditionVariant<runir::kr::UnsFamilyTag, runir::kr::DlTag>,
    runir::kr::ps::ConcreteCondition<runir::kr::UnsFamilyTag, runir::kr::DlTag, runir::kr::ps::dl::BooleanFeature, runir::kr::ps::dl::Positive>,
    runir::kr::ps::ConcreteCondition<runir::kr::UnsFamilyTag, runir::kr::DlTag, runir::kr::ps::dl::BooleanFeature, runir::kr::ps::dl::Negative>,
    runir::kr::ps::ConcreteCondition<runir::kr::UnsFamilyTag, runir::kr::DlTag, runir::kr::ps::dl::NumericalFeature, runir::kr::ps::dl::EqualZero>,
    runir::kr::ps::ConcreteCondition<runir::kr::UnsFamilyTag, runir::kr::DlTag, runir::kr::ps::dl::NumericalFeature, runir::kr::ps::dl::GreaterZero>>;

using EffectTypes =
    ygg::TypeList<runir::kr::ps::EffectVariant<runir::kr::UnsFamilyTag>,
                  runir::kr::ps::ConcreteEffectVariant<runir::kr::UnsFamilyTag, runir::kr::DlTag>,
                  runir::kr::ps::ConcreteEffect<runir::kr::UnsFamilyTag, runir::kr::DlTag, runir::kr::ps::dl::BooleanFeature, runir::kr::ps::dl::Positive>,
                  runir::kr::ps::ConcreteEffect<runir::kr::UnsFamilyTag, runir::kr::DlTag, runir::kr::ps::dl::BooleanFeature, runir::kr::ps::dl::Negative>,
                  runir::kr::ps::ConcreteEffect<runir::kr::UnsFamilyTag, runir::kr::DlTag, runir::kr::ps::dl::BooleanFeature, runir::kr::ps::dl::Unchanged>,
                  runir::kr::ps::ConcreteEffect<runir::kr::UnsFamilyTag, runir::kr::DlTag, runir::kr::ps::dl::NumericalFeature, runir::kr::ps::dl::Increases>,
                  runir::kr::ps::ConcreteEffect<runir::kr::UnsFamilyTag, runir::kr::DlTag, runir::kr::ps::dl::NumericalFeature, runir::kr::ps::dl::Decreases>,
                  runir::kr::ps::ConcreteEffect<runir::kr::UnsFamilyTag, runir::kr::DlTag, runir::kr::ps::dl::NumericalFeature, runir::kr::ps::dl::Unchanged>>;

using SketchTypes = ygg::TypeList<runir::kr::ps::uns::Rule, runir::kr::ps::uns::Sketch>;
using RepositoryTypes = ygg::ConcatTypeListsT<FeatureTypes, ConditionTypes, EffectTypes, SketchTypes>;
using Repository = runir::kr::ps::BasicRepository<runir::kr::UnsFamilyTag, RepositoryTypes, runir::kr::dl::uns::ConstructorRepositoryPtr>;
using RepositoryPtr = std::shared_ptr<Repository>;
using RepositoryFactory = runir::kr::ps::BasicRepositoryFactory<runir::kr::UnsFamilyTag, RepositoryTypes, runir::kr::dl::uns::ConstructorRepositoryPtr>;

using SketchView = ygg::View<ygg::Index<runir::kr::ps::uns::Sketch>, Repository>;
using RuleView = ygg::View<ygg::Index<runir::kr::ps::uns::Rule>, Repository>;

}  // namespace runir::kr::ps::uns

#endif
