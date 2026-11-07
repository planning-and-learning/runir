#ifndef RUNIR_KR_UNS_DECLARATIONS_HPP_
#define RUNIR_KR_UNS_DECLARATIONS_HPP_

#include "runir/kr/declarations.hpp"
#include "runir/kr/dl/declarations.hpp"
#include "runir/kr/ps/declarations.hpp"
#include "runir/kr/ps/dl/declarations.hpp"
#include "runir/kr/ps/family_traits.hpp"

#include <memory>
#include <boost/mp11.hpp>
#include <urd/elements/flat.hpp>

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

using RepositoryTypes =
    boost::mp11::mp_append<runir::kr::ps::PsFeatureTypes<runir::kr::UnsFamilyTag>, boost::mp11::mp_list<ClassifierLiteral, ClassifierClause, Classifier>>;
using Repository = runir::kr::ps::BasicRepository<runir::kr::UnsFamilyTag, RepositoryTypes>;
using RepositoryPtr = std::shared_ptr<Repository>;
using RepositoryFactory = runir::kr::ps::BasicRepositoryFactory<runir::kr::UnsFamilyTag, RepositoryTypes>;

using ClassifierLiteralView = urd::index_view<runir::kr::uns::ClassifierLiteral, Repository>;
using ClassifierClauseView = urd::index_view<runir::kr::uns::ClassifierClause, Repository>;
using ClassifierView = urd::index_view<runir::kr::uns::Classifier, Repository>;

}  // namespace runir::kr::uns

namespace runir::kr::uns::dl
{

using BooleanFeatureView = urd::index_view<runir::kr::ps::Feature<runir::kr::UnsFamilyTag, runir::kr::dl::BooleanTag>, Repository>;
using ConcreteBooleanFeatureView =
    urd::index_view<runir::kr::ps::ConcreteFeature<runir::kr::UnsFamilyTag, runir::kr::DlTag, runir::kr::dl::BooleanTag>, Repository>;

}  // namespace runir::kr::uns::dl

#endif
