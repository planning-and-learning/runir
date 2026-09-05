#ifndef RUNIR_KR_UNS_DECLARATIONS_HPP_
#define RUNIR_KR_UNS_DECLARATIONS_HPP_

#include "runir/kr/declarations.hpp"
#include "runir/kr/dl/declarations.hpp"
#include "runir/kr/ps/declarations.hpp"
#include "runir/kr/ps/dl/declarations.hpp"

#include <memory>
#include <yggdrasil/core/type_list.hpp>
#include <yggdrasil/core/types.hpp>

namespace runir::kr::uns
{

// A literal of the classifier DNF: a boolean feature together with a polarity.
struct ClassifierLiteral
{
};

// A clause of the classifier DNF: a conjunction of literals.
struct ClassifierClause
{
};

// A classifier: named boolean features + a DNF (disjunction of clauses) over those features.
struct Classifier
{
    static constexpr auto keyword = "classifier";
};

using RepositoryTypes = ygg::TypeList<runir::kr::ps::ConcreteFeature<runir::kr::UnsFamilyTag, runir::kr::DlTag, runir::kr::ps::dl::BooleanFeature>,
                                      runir::kr::ps::Feature<runir::kr::UnsFamilyTag, runir::kr::ps::dl::BooleanFeature>,
                                      runir::kr::uns::ClassifierLiteral,
                                      runir::kr::uns::ClassifierClause,
                                      runir::kr::uns::Classifier>;
using Repository =
    runir::kr::ps::BasicRepository<runir::kr::UnsFamilyTag, RepositoryTypes, runir::kr::dl::ConstructorRepositoryPtrFor<runir::kr::UnsFamilyTag>>;
using RepositoryPtr = std::shared_ptr<Repository>;
using RepositoryFactory =
    runir::kr::ps::BasicRepositoryFactory<runir::kr::UnsFamilyTag, RepositoryTypes, runir::kr::dl::ConstructorRepositoryPtrFor<runir::kr::UnsFamilyTag>>;

using ClassifierLiteralView = ygg::View<ygg::Index<runir::kr::uns::ClassifierLiteral>, Repository>;
using ClassifierClauseView = ygg::View<ygg::Index<runir::kr::uns::ClassifierClause>, Repository>;
using ClassifierView = ygg::View<ygg::Index<runir::kr::uns::Classifier>, Repository>;

}  // namespace runir::kr::uns

namespace runir::kr::uns::dl
{

using BooleanFeatureView = ygg::View<ygg::Index<runir::kr::ps::Feature<runir::kr::UnsFamilyTag, runir::kr::ps::dl::BooleanFeature>>, Repository>;
using ConcreteBooleanFeatureView =
    ygg::View<ygg::Index<runir::kr::ps::ConcreteFeature<runir::kr::UnsFamilyTag, runir::kr::DlTag, runir::kr::ps::dl::BooleanFeature>>, Repository>;

}  // namespace runir::kr::uns::dl

#endif
