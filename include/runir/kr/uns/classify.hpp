#ifndef RUNIR_KR_UNS_CLASSIFY_HPP_
#define RUNIR_KR_UNS_CLASSIFY_HPP_

#include "runir/kr/dl/semantics/evaluation_context.hpp"
#include "runir/kr/ps/dl/evaluation.hpp"
#include "runir/kr/uns/classifier_view.hpp"

#include <tyr/planning/declarations.hpp>
#include <urd/elements/flat.hpp>

namespace runir::kr::uns
{

// Classify a state by evaluating each literal's boolean feature, then the DNF over the literals.
// The DNF is an OR over clauses of an AND over literals: an empty OR evaluates to false and an
// empty AND evaluates to true.
template<tyr::TaskKind Kind, typename C, runir::kr::dl::semantics::EvaluationContextConcept<Kind> Context>
bool classify(urd::view<runir::kr::uns::Classifier, C> classifier, Context& context)
{
    for (auto clause : classifier.get_clauses())
    {
        bool satisfied = true;
        for (auto literal : clause.get_literals())
        {
            const bool value = std::visit([&](auto feature) { return runir::kr::ps::evaluate<Kind>(feature, context).value(); }, literal.get_feature());
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
