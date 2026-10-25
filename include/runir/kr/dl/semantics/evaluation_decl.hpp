#ifndef RUNIR_SEMANTICS_EVALUATION_DECL_HPP_
#define RUNIR_SEMANTICS_EVALUATION_DECL_HPP_

#include "runir/kr/dl/declarations.hpp"
#include "runir/kr/dl/semantics/declarations.hpp"
#include "runir/kr/dl/semantics/state_evaluation_context.hpp"

#include <tyr/planning/declarations.hpp>
#include <yggdrasil/core/types.hpp>
#include <yggdrasil/database/declarations.hpp>

namespace runir::kr::dl::semantics
{

template<tyr::TaskKind Kind, StateEvaluationContextConcept<Kind> Context, typename C>
auto evaluate(ygg::View<ygg::Index<Query<>>, C> constructor, Context& context) -> QueryDenotationView;

namespace detail
{
/// Defined in distance_evaluation.hpp.
template<tyr::TaskKind Kind, StateEvaluationContextConcept<Kind> Context>
ygg::uint_t on_demand_distance(NumericalView<DistanceTag> constructor, Context& context);
/// Defined in distance_evaluation.hpp.
template<tyr::TaskKind Kind, StateEvaluationContextConcept<Kind> Context, typename C>
void on_demand_must_pass(ygg::View<ygg::Index<Query<QueryMustPassTag>>, C> constructor, Context& context, ygg::Builder<ygg::database::Relation>& result);
/// Defined in datalog_evaluation.hpp.
template<tyr::TaskKind Kind, StateEvaluationContextConcept<Kind> Context, typename C>
void on_demand_head(ygg::View<ygg::Index<Query<QueryHeadTag>>, C> constructor, Context& context, ygg::Builder<ygg::database::Relation>& result);
}

template<tyr::TaskKind Kind, ConceptOrRoleTag Category, StateEvaluationContextConcept<Kind> Context, typename C>
auto evaluate_impl(ygg::View<ygg::Index<QueryProjection<Category>>, C> constructor, Context& context) -> DenotationView<Category>;

template<tyr::TaskKind Kind, CategoryTag Category, StateEvaluationContextConcept<Kind> Context>
auto evaluate_impl(ConstructorView<Category> constructor, Context& context) -> DenotationView<Category>;

template<tyr::TaskKind Kind, typename Tag, StateEvaluationContextConcept<Kind> Context, typename C>
    requires ConceptConstructorTag<Tag>
auto evaluate_impl(ygg::View<ygg::Index<Concept<Tag>>, C> constructor, Context& context) -> DenotationView<ConceptTag>;

template<tyr::TaskKind Kind, typename Tag, StateEvaluationContextConcept<Kind> Context, typename C>
    requires RoleConstructorTag<Tag>
auto evaluate_impl(ygg::View<ygg::Index<Role<Tag>>, C> constructor, Context& context) -> DenotationView<RoleTag>;

template<tyr::TaskKind Kind, typename Tag, StateEvaluationContextConcept<Kind> Context, typename C>
    requires BooleanConstructorTag<Tag>
auto evaluate_impl(ygg::View<ygg::Index<Boolean<Tag>>, C> constructor, Context& context) -> DenotationView<BooleanTag>;

template<tyr::TaskKind Kind, typename Tag, StateEvaluationContextConcept<Kind> Context, typename C>
    requires NumericalConstructorTag<Tag>
auto evaluate_impl(ygg::View<ygg::Index<Numerical<Tag>>, C> constructor, Context& context) -> DenotationView<NumericalTag>;

template<tyr::TaskKind Kind, CategoryTag Category, StateEvaluationContextConcept<Kind> Context>
auto evaluate(ConstructorView<Category> constructor, Context& context) -> DenotationView<Category>;
}

#endif
