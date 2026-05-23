#ifndef RUNIR_KR_PS_BASE_REPOSITORY_HPP_
#define RUNIR_KR_PS_BASE_REPOSITORY_HPP_

#include "runir/kr/ps/base/declarations.hpp"
#include "runir/kr/ps/repository.hpp"

namespace runir::kr::ps::base
{

using FeatureTypes = runir::kr::ps::FeatureTypes;
using ConditionTypes = runir::kr::ps::ConditionTypes;
using EffectTypes = runir::kr::ps::EffectTypes;
using SketchTypes = runir::kr::ps::SketchTypes;
using RepositoryTypes = runir::kr::ps::RepositoryTypes;
using SymbolRepository = runir::kr::ps::SymbolRepository;

using Repository = runir::kr::ps::Repository;
using RepositoryPtr = runir::kr::ps::RepositoryPtr;
using RepositoryFactory = runir::kr::ps::RepositoryFactory;
using RepositoryFactoryPtr = runir::kr::ps::RepositoryFactoryPtr;

using SketchView = runir::kr::ps::SketchView;
using RuleView = runir::kr::ps::RuleView;

}  // namespace runir::kr::ps::base

#endif
