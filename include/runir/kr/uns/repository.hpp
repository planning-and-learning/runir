#ifndef RUNIR_KR_UNS_REPOSITORY_HPP_
#define RUNIR_KR_UNS_REPOSITORY_HPP_

#include "runir/kr/dl/uns/repository.hpp"
#include "runir/kr/ps/repository.hpp"
#include "runir/kr/uns/classifier_data.hpp"
#include "runir/kr/uns/classifier_view.hpp"
#include "runir/kr/uns/declarations.hpp"
#include "runir/kr/uns/dl/feature_data.hpp"
#include "runir/kr/uns/dl/feature_view.hpp"
#include "runir/kr/uns/feature_data.hpp"
#include "runir/kr/uns/feature_view.hpp"

#include <memory>
#include <yggdrasil/core/type_list.hpp>

namespace runir::kr::uns
{

// The repository is keyed by the feature-language family (UnsFamilyTag); it stores the DL feature
// language types as well as the classifier (DNF) structure types.
using RepositoryTypes = ygg::TypeList<runir::kr::uns::dl::Feature,
                                      runir::kr::uns::Feature,
                                      runir::kr::uns::ClassifierLiteral,
                                      runir::kr::uns::ClassifierClause,
                                      runir::kr::uns::Classifier>;
using Repository = runir::kr::ps::BasicRepository<runir::kr::UnsFamilyTag, RepositoryTypes, runir::kr::dl::uns::ConstructorRepositoryPtr>;
using RepositoryPtr = std::shared_ptr<Repository>;
using RepositoryFactory = runir::kr::ps::BasicRepositoryFactory<runir::kr::UnsFamilyTag, RepositoryTypes, runir::kr::dl::uns::ConstructorRepositoryPtr>;

using ConcreteFeatureView = ygg::View<ygg::Index<runir::kr::uns::dl::Feature>, Repository>;
using FeatureView = ygg::View<ygg::Index<runir::kr::uns::Feature>, Repository>;
using ClassifierLiteralView = ygg::View<ygg::Index<runir::kr::uns::ClassifierLiteral>, Repository>;
using ClassifierClauseView = ygg::View<ygg::Index<runir::kr::uns::ClassifierClause>, Repository>;
using ClassifierView = ygg::View<ygg::Index<runir::kr::uns::Classifier>, Repository>;

}  // namespace runir::kr::uns

#endif
