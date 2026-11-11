#include "runir/kr/dl/semantics/evaluation.hpp"

#include "runir/kr/dl/datas.hpp"
#include "runir/kr/dl/declarations.hpp"
#include "runir/kr/dl/semantics/denotation_repository.hpp"
#include "runir/kr/dl/semantics/detail/evaluation_impl.hpp"
#include "runir/kr/dl/semantics/detail/repository_instantiations.hpp"
#include "runir/kr/dl/semantics/incremental/detail/evaluation_impl.hpp"
#include "runir/kr/dl/semantics/state_evaluation_context.hpp"

#include <boost/mp11/algorithm.hpp>
#include <boost/preprocessor/repetition/repeat.hpp>
#include <boost/preprocessor/tuple/elem.hpp>
#include <cstddef>
#include <tyr/datalog/lifted/database/detail/repository_instantiations.hpp>
#include <tyr/datalog/lifted/database/syntax/query_repository.hpp>

namespace runir::kr::dl::semantics
{
namespace
{
/// The I-th type of a list of category tags, and an evaluation context over builder states of a task kind.
template<typename Categories, std::size_t I>
using At = boost::mp11::mp_at_c<Categories, I>;
template<tyr::TaskKind Kind>
using BuilderContext = StateEvaluationContext<Kind, tyr::planning::BuilderStateView<Kind>>;
}

static_assert(boost::mp11::mp_size<CategoryTags>::value == 4, "the instantiations below cover the 4 CategoryTags");
static_assert(boost::mp11::mp_size<ConceptOrRoleTags>::value == 2, "the instantiations below cover the 2 ConceptOrRoleTags");
static_assert(boost::mp11::mp_size<BooleanOrNumericalTags>::value == 2, "the instantiations below cover the 2 BooleanOrNumericalTags");

/// evaluate for the I-th category of CategoryTags; ARGS = (task kind, evaluation context).
#define RUNIR_EVALUATE_AT(z, I, ARGS)                                                                                                \
    template auto evaluate<BOOST_PP_TUPLE_ELEM(2, 0, ARGS)>(ConstructorView<At<CategoryTags, I>>, BOOST_PP_TUPLE_ELEM(2, 1, ARGS) &) \
        -> DenotationView<At<CategoryTags, I>>;
/// evaluate for every category and for queries.
#define RUNIR_EVALUATE(Kind, Context)                      \
    BOOST_PP_REPEAT(4, RUNIR_EVALUATE_AT, (Kind, Context)) \
    template auto evaluate<Kind>(QueryView<>, Context&) -> QueryDenotationView;

RUNIR_EVALUATE(tyr::GroundTag, StateEvaluationContext<tyr::GroundTag>)
RUNIR_EVALUATE(tyr::LiftedTag, StateEvaluationContext<tyr::LiftedTag>)
/// Queries over builder states with interned register values, which the successor expander tests evaluate.
RUNIR_EVALUATE(tyr::GroundTag, BuilderContext<tyr::GroundTag>)
RUNIR_EVALUATE(tyr::LiftedTag, BuilderContext<tyr::LiftedTag>)

}  // namespace runir::kr::dl::semantics

namespace runir::kr::dl::semantics::incremental
{

/// EvaluationGraph's members for the I-th category of CategoryTags (prepare, get_index, result), of ConceptOrRoleTags
/// (change, cardinality) and IncrementalEvaluation's for the I-th of BooleanOrNumericalTags (changed); Kind: the task kind.
#define RUNIR_EVALUATION_GRAPH_AT(z, I, Kind)                                                                                                        \
    template EvaluationIndex<At<CategoryTags, I>> EvaluationGraph<Kind>::prepare<At<CategoryTags, I>>(ConstructorView<At<CategoryTags, I>>);         \
    template EvaluationIndex<At<CategoryTags, I>> EvaluationGraph<Kind>::get_index<At<CategoryTags, I>>(ConstructorView<At<CategoryTags, I>>) const; \
    template BorrowedDenotationView<At<CategoryTags, I>> EvaluationGraph<Kind>::result<At<CategoryTags, I>>(EvaluationIndex<At<CategoryTags, I>>) const;
#define RUNIR_CONCEPT_OR_ROLE_AT(z, I, Kind)                                                                           \
    template const DenotationDelta<At<ConceptOrRoleTags, I>>& EvaluationGraph<Kind>::change<At<ConceptOrRoleTags, I>>( \
        EvaluationIndex<At<ConceptOrRoleTags, I>>) const;                                                              \
    template size_t EvaluationGraph<Kind>::cardinality<At<ConceptOrRoleTags, I>>(EvaluationIndex<At<ConceptOrRoleTags, I>>) const;
#define RUNIR_BOOLEAN_OR_NUMERICAL_AT(z, I, Kind) \
    template bool IncrementalEvaluation<Kind>::changed<At<BooleanOrNumericalTags, I>>(EvaluationIndex<At<BooleanOrNumericalTags, I>>) const;
/// The evaluation classes of a task kind, their members per category, and demand over registered and builder states.
#define RUNIR_INCREMENTAL_EVALUATION(Kind)                                                                               \
    template class EvaluationGraph<Kind>;                                                                                \
    template class IncrementalEvaluation<Kind>;                                                                          \
    BOOST_PP_REPEAT(4, RUNIR_EVALUATION_GRAPH_AT, Kind)                                                                  \
    BOOST_PP_REPEAT(2, RUNIR_CONCEPT_OR_ROLE_AT, Kind)                                                                   \
    BOOST_PP_REPEAT(2, RUNIR_BOOLEAN_OR_NUMERICAL_AT, Kind)                                                              \
    template void IncrementalEvaluation<Kind>::demand<StateEvaluationContext<Kind>>(ygg::uint_t,                         \
                                                                                    StateEvaluationContext<Kind>&,       \
                                                                                    const Delta*,                        \
                                                                                    tyr::datalog::database::Workspace*); \
    template void IncrementalEvaluation<Kind>::demand<BuilderContext<Kind>>(ygg::uint_t,                                 \
                                                                            BuilderContext<Kind>&,                       \
                                                                            const Delta*,                                \
                                                                            tyr::datalog::database::Workspace*);

RUNIR_INCREMENTAL_EVALUATION(tyr::GroundTag)
RUNIR_INCREMENTAL_EVALUATION(tyr::LiftedTag)

}  // namespace runir::kr::dl::semantics::incremental
