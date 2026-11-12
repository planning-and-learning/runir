#ifndef RUNIR_SEMANTICS_EVALUATION_DECL_HPP_
#define RUNIR_SEMANTICS_EVALUATION_DECL_HPP_

#include "runir/kr/dl/declarations.hpp"
#include "runir/kr/dl/semantics/declarations.hpp"
#include "runir/kr/dl/semantics/state_evaluation_context.hpp"

#include <tyr/datalog/lifted/database/declarations.hpp>
#include <tyr/planning/declarations.hpp>
#include <urd/elements/flat.hpp>
#include <yggdrasil/core/config.hpp>

namespace runir::kr::dl::semantics
{

template<tyr::TaskKind Kind, StateEvaluationContextConcept<Kind> Context, typename C>
auto evaluate(urd::index_view<Query<>, C> constructor, Context& context) -> QueryDenotationView;

namespace detail
{
/// Defined in datalog_evaluation.hpp.
template<tyr::TaskKind Kind, StateEvaluationContextConcept<Kind> Context, typename C>
void on_demand_head(urd::index_view<Query<QueryHeadTag>, C> constructor, Context& context, urd::builder<ygg::Relation<ygg::Schema>>& result);
}

template<tyr::TaskKind Kind, ConceptOrRoleTag Category, StateEvaluationContextConcept<Kind> Context, typename C>
auto evaluate_impl(urd::index_view<QueryProjection<Category>, C> constructor, Context& context) -> DenotationView<Category>;

template<tyr::TaskKind Kind, CategoryTag Category, StateEvaluationContextConcept<Kind> Context>
auto evaluate_impl(ConstructorView<Category> constructor, Context& context) -> DenotationView<Category>;

template<tyr::TaskKind Kind, typename Tag, StateEvaluationContextConcept<Kind> Context, typename C>
    requires ConceptConstructorTag<Tag>
auto evaluate_impl(urd::index_view<Concept<SemanticFamily, Tag>, C> constructor, Context& context) -> DenotationView<ConceptTag>;

template<tyr::TaskKind Kind, typename Tag, StateEvaluationContextConcept<Kind> Context, typename C>
    requires RoleConstructorTag<Tag>
auto evaluate_impl(urd::index_view<Role<SemanticFamily, Tag>, C> constructor, Context& context) -> DenotationView<RoleTag>;

template<tyr::TaskKind Kind, typename Tag, StateEvaluationContextConcept<Kind> Context, typename C>
    requires BooleanConstructorTag<Tag>
auto evaluate_impl(urd::index_view<Boolean<SemanticFamily, Tag>, C> constructor, Context& context) -> DenotationView<BooleanTag>;

template<tyr::TaskKind Kind, typename Tag, StateEvaluationContextConcept<Kind> Context, typename C>
    requires NumericalConstructorTag<Tag>
auto evaluate_impl(urd::index_view<Numerical<SemanticFamily, Tag>, C> constructor, Context& context) -> DenotationView<NumericalTag>;

template<tyr::TaskKind Kind, CategoryTag Category, StateEvaluationContextConcept<Kind> Context>
auto evaluate(ConstructorView<Category> constructor, Context& context) -> DenotationView<Category>;
}

#endif
