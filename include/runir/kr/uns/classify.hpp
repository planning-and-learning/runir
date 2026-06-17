#ifndef RUNIR_KR_UNS_CLASSIFY_HPP_
#define RUNIR_KR_UNS_CLASSIFY_HPP_

#include "runir/kr/dl/semantics/uns/evaluation_context.hpp"
#include "runir/kr/uns/classifier_view.hpp"
#include "runir/kr/uns/evaluation.hpp"

#include <tyr/planning/declarations.hpp>
#include <yggdrasil/containers/variant.hpp>
#include <yggdrasil/core/types.hpp>

namespace runir::kr::uns
{

// Classify a state by evaluating each literal's boolean feature, then the DNF over the literals.
// The DNF is an OR over clauses of an AND over literals: an empty OR evaluates to false and an
// empty AND evaluates to true.
template<typename C, tyr::planning::TaskKind Kind>
bool classify(ygg::View<ygg::Index<runir::kr::uns::Classifier>, C> classifier,
              runir::kr::dl::semantics::EvaluationContext<runir::kr::UnsFamilyTag, Kind>& context)
{
    for (auto clause : classifier.get_clauses())
    {
        bool satisfied = true;
        for (auto literal : clause.get_literals())
        {
            const bool value = ygg::visit([&](auto feature) { return runir::kr::uns::evaluate(feature, context); }, literal.get_feature());
            if (literal.get_polarity() ? !value : value)
            {
                satisfied = false;
                break;
            }
        }
        if (satisfied)
            return true;
    }

    return false;
}

}  // namespace runir::kr::uns

#endif
