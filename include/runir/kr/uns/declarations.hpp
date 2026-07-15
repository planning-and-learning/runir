#ifndef RUNIR_KR_UNS_DECLARATIONS_HPP_
#define RUNIR_KR_UNS_DECLARATIONS_HPP_

#include "runir/kr/declarations.hpp"
#include "runir/kr/dl/declarations.hpp"
#include "runir/kr/ps/declarations.hpp"
#include "runir/kr/uns/dl/declarations.hpp"

#include <memory>
#include <yggdrasil/core/type_list.hpp>
#include <yggdrasil/core/types.hpp>

namespace runir::kr::uns
{

// Language-agnostic wrapper around a concrete feature specialization. The wrapper's variant lists the
// available feature languages (currently only the DL one in kr/uns/dl), so different feature languages
// can be used without templating the classifier on a feature-language family.
struct Feature
{
};

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

using RepositoryTypes = ygg::TypeList<runir::kr::uns::dl::Feature,
                                      runir::kr::uns::Feature,
                                      runir::kr::uns::ClassifierLiteral,
                                      runir::kr::uns::ClassifierClause,
                                      runir::kr::uns::Classifier>;
using Repository =
    runir::kr::ps::BasicRepository<runir::kr::UnsFamilyTag, RepositoryTypes, runir::kr::dl::ConstructorRepositoryPtrFor<runir::kr::UnsFamilyTag>>;
using RepositoryPtr = std::shared_ptr<Repository>;
using RepositoryFactory =
    runir::kr::ps::BasicRepositoryFactory<runir::kr::UnsFamilyTag, RepositoryTypes, runir::kr::dl::ConstructorRepositoryPtrFor<runir::kr::UnsFamilyTag>>;

using ConcreteFeatureView = ygg::View<ygg::Index<runir::kr::uns::dl::Feature>, Repository>;
using FeatureView = ygg::View<ygg::Index<runir::kr::uns::Feature>, Repository>;
using ClassifierLiteralView = ygg::View<ygg::Index<runir::kr::uns::ClassifierLiteral>, Repository>;
using ClassifierClauseView = ygg::View<ygg::Index<runir::kr::uns::ClassifierClause>, Repository>;
using ClassifierView = ygg::View<ygg::Index<runir::kr::uns::Classifier>, Repository>;

}  // namespace runir::kr::uns

#endif
