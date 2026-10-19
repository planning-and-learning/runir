#ifndef RUNIR_SEMANTICS_EVALUATION_DECL_HPP_
#define RUNIR_SEMANTICS_EVALUATION_DECL_HPP_

#include "runir/kr/dl/constructors.hpp"
#include "runir/kr/dl/declarations.hpp"
#include "runir/kr/dl/semantics/declarations.hpp"
#include "runir/kr/dl/semantics/state_evaluation_context.hpp"

#include <tyr/planning/declarations.hpp>
#include <yggdrasil/core/types.hpp>
#include <yggdrasil/database/declarations.hpp>

namespace runir::kr::dl::semantics
{

template<tyr::TaskKind Kind, FamilyTag Family, StateEvaluationContextConcept<Family, Kind> Context, typename C>
auto evaluate(ygg::View<ygg::Index<Query<Family>>, C> constructor, Context& context) -> QueryDenotationView;

namespace detail
{
/// Defined in distance_evaluation.hpp.
template<tyr::TaskKind Kind, FamilyTag Family, StateEvaluationContextConcept<Family, Kind> Context>
ygg::uint_t on_demand_distance(FamilyNumericalView<Family, DistanceTag> constructor, Context& context);
/// Defined in distance_evaluation.hpp.
template<tyr::TaskKind Kind, FamilyTag Family, StateEvaluationContextConcept<Family, Kind> Context, typename C>
void on_demand_must_pass(ygg::View<ygg::Index<Query<Family, QueryMustPassTag>>, C> constructor,
                         Context& context,
                         ygg::Builder<ygg::database::Relation<QueryValues>>& result);
}

template<tyr::TaskKind Kind, FamilyTag Family, ConceptOrRoleTag Category, StateEvaluationContextConcept<Family, Kind> Context, typename C>
auto evaluate_impl(ygg::View<ygg::Index<QueryProjection<Family, Category>>, C> constructor, Context& context) -> DenotationView<Category>;

template<tyr::TaskKind Kind, FamilyTag Family, CategoryTag Category, StateEvaluationContextConcept<Family, Kind> Context>
auto evaluate_impl(FamilyConstructorView<Family, Category> constructor, Context& context) -> DenotationView<Category>;

template<tyr::TaskKind Kind, FamilyTag Family, typename Tag, StateEvaluationContextConcept<Family, Kind> Context, typename C>
    requires FamilyConceptConstructorTag<Family, Tag>
auto evaluate_impl(ygg::View<ygg::Index<FamilyConcept<Family, Tag>>, C> constructor, Context& context) -> DenotationView<ConceptTag>;

template<tyr::TaskKind Kind, FamilyTag Family, typename Tag, StateEvaluationContextConcept<Family, Kind> Context, typename C>
    requires FamilyRoleConstructorTag<Family, Tag>
auto evaluate_impl(ygg::View<ygg::Index<FamilyRole<Family, Tag>>, C> constructor, Context& context) -> DenotationView<RoleTag>;

template<tyr::TaskKind Kind, FamilyTag Family, typename Tag, StateEvaluationContextConcept<Family, Kind> Context, typename C>
    requires FamilyBooleanConstructorTag<Family, Tag>
auto evaluate_impl(ygg::View<ygg::Index<FamilyBoolean<Family, Tag>>, C> constructor, Context& context) -> DenotationView<BooleanTag>;

template<tyr::TaskKind Kind, FamilyTag Family, typename Tag, StateEvaluationContextConcept<Family, Kind> Context, typename C>
    requires FamilyNumericalConstructorTag<Family, Tag>
auto evaluate_impl(ygg::View<ygg::Index<FamilyNumerical<Family, Tag>>, C> constructor, Context& context) -> DenotationView<NumericalTag>;

template<tyr::TaskKind Kind, FamilyTag Family, CategoryTag Category, StateEvaluationContextConcept<Family, Kind> Context>
auto evaluate(FamilyConstructorView<Family, Category> constructor, Context& context) -> DenotationView<Category>;

}

#endif
