#ifndef RUNIR_KR_UNS_DECLARATIONS_HPP_
#define RUNIR_KR_UNS_DECLARATIONS_HPP_

#include "runir/kr/declarations.hpp"

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

}  // namespace runir::kr::uns

#endif
