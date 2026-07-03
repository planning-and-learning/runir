#ifndef RUNIR_KR_PS_BASE_REPOSITORY_HPP_
#define RUNIR_KR_PS_BASE_REPOSITORY_HPP_

#include "runir/kr/dl/repository.hpp"
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
#include "runir/kr/ps/family_traits.hpp"
#include "runir/kr/ps/feature_view.hpp"
#include "runir/kr/ps/repository.hpp"

#include <memory>
#include <yggdrasil/core/type_list.hpp>

namespace runir::kr::ps::base
{

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

#endif
