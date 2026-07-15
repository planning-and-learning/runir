#ifndef RUNIR_KR_PS_BASE_DECLARATIONS_HPP_
#define RUNIR_KR_PS_BASE_DECLARATIONS_HPP_

#include "runir/kr/dl/declarations.hpp"
#include "runir/kr/ps/declarations.hpp"
#include "runir/kr/ps/family_traits.hpp"

#include <memory>
#include <yggdrasil/core/type_list.hpp>
#include <yggdrasil/core/types.hpp>

namespace runir::kr::ps::base
{

// Rule

struct Rule
{
};

// Sketch

struct Sketch
{
};

using FeatureTypes = runir::kr::ps::PsFeatureTypes<runir::kr::BaseFamilyTag>;
using ConditionTypes = runir::kr::ps::PsConditionTypes<runir::kr::BaseFamilyTag>;
using EffectTypes = runir::kr::ps::PsEffectTypes<runir::kr::BaseFamilyTag>;
using SketchTypes = ygg::TypeList<runir::kr::ps::base::Rule, runir::kr::ps::base::Sketch>;
using RepositoryTypes = ygg::ConcatTypeListsT<FeatureTypes, ConditionTypes, EffectTypes, SketchTypes>;
using Repository =
    runir::kr::ps::BasicRepository<runir::kr::BaseFamilyTag, RepositoryTypes, runir::kr::dl::ConstructorRepositoryPtrFor<runir::kr::BaseFamilyTag>>;
using RepositoryPtr = std::shared_ptr<Repository>;
using RepositoryFactory =
    runir::kr::ps::BasicRepositoryFactory<runir::kr::BaseFamilyTag, RepositoryTypes, runir::kr::dl::ConstructorRepositoryPtrFor<runir::kr::BaseFamilyTag>>;

using SketchView = ygg::View<ygg::Index<runir::kr::ps::base::Sketch>, Repository>;
using RuleView = ygg::View<ygg::Index<runir::kr::ps::base::Rule>, Repository>;

}  // namespace runir::kr::ps::base

namespace runir::kr::ps::base::dl
{

using BooleanFeatureView = ygg::View<ygg::Index<runir::kr::ps::Feature<runir::kr::BaseFamilyTag, runir::kr::ps::dl::BooleanFeature>>, Repository>;
using NumericalFeatureView = ygg::View<ygg::Index<runir::kr::ps::Feature<runir::kr::BaseFamilyTag, runir::kr::ps::dl::NumericalFeature>>, Repository>;

}  // namespace runir::kr::ps::base::dl

#endif
