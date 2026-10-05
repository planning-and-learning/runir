#include <concepts>
#include <runir/kr/ps/base/repository.hpp>
#include <runir/kr/ps/compatibility.hpp>
#include <runir/kr/ps/ext/repository.hpp>
#include <runir/kr/ps/family_traits.hpp>
#include <runir/kr/ps/icp/repository.hpp>
#include <runir/kr/ps/rule_evaluator_concepts.hpp>
#include <utility>

namespace runir::tests
{
namespace
{

template<kr::FamilyTag Family, tyr::TaskKind Kind>
constexpr bool mapped_context()
{
    using Context = kr::ps::dl::TransitionEvaluationContext<Family, Kind>;
    using DlFamily = kr::ps::DlFamilyFor<Family>;
    using StateContext = kr::dl::semantics::StateEvaluationContext<DlFamily, Kind>;
    using ResultContext = decltype(std::declval<const StateContext&>().for_result(false));
    using ChildContext = decltype(std::declval<const StateContext&>().child_context());
    return kr::ps::IsTransitionEvaluationContext<Family, kr::DlTag, Context>
           && std::same_as<decltype(std::declval<Context&>().get_source_context()), StateContext&>
           && std::same_as<decltype(std::declval<Context&>().get_target_context()), StateContext&>
           && kr::dl::semantics::StateEvaluationContextConcept<StateContext&, DlFamily, Kind>
           && kr::dl::semantics::StateEvaluationContextConcept<ResultContext, DlFamily, Kind>
           && kr::dl::semantics::StateEvaluationContextConcept<ChildContext, DlFamily, Kind>
           && (!std::same_as<DlFamily, kr::ExtFamilyTag> || (std::same_as<ResultContext, StateContext> && std::same_as<ChildContext, StateContext>) );
}

static_assert(mapped_context<kr::BaseFamilyTag, tyr::GroundTag>());
static_assert(mapped_context<kr::BaseFamilyTag, tyr::LiftedTag>());
static_assert(mapped_context<kr::ExtFamilyTag, tyr::GroundTag>());
static_assert(mapped_context<kr::ExtFamilyTag, tyr::LiftedTag>());
static_assert(mapped_context<kr::IcpFamilyTag, tyr::GroundTag>());
static_assert(mapped_context<kr::IcpFamilyTag, tyr::LiftedTag>());
static_assert(mapped_context<kr::UnsFamilyTag, tyr::GroundTag>());
static_assert(mapped_context<kr::UnsFamilyTag, tyr::LiftedTag>());

struct NameOnlyContext
{
    void get_source_state() const;
    void get_target_state() const;
};

using BaseContext = kr::ps::dl::TransitionEvaluationContext<kr::BaseFamilyTag, tyr::GroundTag>;
using ExtContext = kr::ps::dl::TransitionEvaluationContext<kr::ExtFamilyTag, tyr::GroundTag>;
using IcpContext = kr::ps::dl::TransitionEvaluationContext<kr::IcpFamilyTag, tyr::GroundTag>;
using BaseStateContext = kr::dl::semantics::StateEvaluationContext<kr::BaseFamilyTag, tyr::GroundTag>;
using ExtStateContext = kr::dl::semantics::StateEvaluationContext<kr::ExtFamilyTag, tyr::GroundTag>;

// The returned resources identify the task kind; the PS family still distinguishes ICP from Ext.
struct IcpResourceContext
{
    using FamilyType = kr::IcpFamilyTag;

    ExtStateContext make_dl_context(tyr::planning::StateView<tyr::GroundTag>);
    ExtStateContext& get_source_context();
    ExtStateContext& get_target_context();
};

static_assert(kr::ps::RuleEvaluationContextConcept<IcpResourceContext, kr::IcpFamilyTag, tyr::GroundTag, tyr::planning::StateView<tyr::GroundTag>>);
static_assert(!kr::ps::RuleEvaluationContextConcept<IcpResourceContext, kr::IcpFamilyTag, tyr::LiftedTag, tyr::planning::StateView<tyr::GroundTag>>);
static_assert(!kr::ps::RuleEvaluationContextConcept<IcpResourceContext, kr::ExtFamilyTag, tyr::GroundTag, tyr::planning::StateView<tyr::GroundTag>>);
static_assert(kr::ps::dl::TransitionEvaluationContextConcept<IcpResourceContext, kr::IcpFamilyTag, tyr::GroundTag>);
static_assert(!kr::ps::dl::TransitionEvaluationContextConcept<IcpResourceContext, kr::IcpFamilyTag, tyr::LiftedTag>);
static_assert(!kr::ps::dl::TransitionEvaluationContextConcept<IcpResourceContext, kr::ExtFamilyTag, tyr::GroundTag>);

static_assert(kr::dl::semantics::StateEvaluationContextConcept<ExtStateContext, kr::ps::DlFamilyFor<kr::IcpFamilyTag>, tyr::GroundTag>);
static_assert(!kr::dl::semantics::StateEvaluationContextConcept<BaseStateContext, kr::ps::DlFamilyFor<kr::IcpFamilyTag>, tyr::GroundTag>);
static_assert(!kr::dl::semantics::StateEvaluationContextConcept<ExtStateContext, kr::BaseFamilyTag, tyr::GroundTag>);
static_assert(!kr::dl::semantics::StateEvaluationContextConcept<ExtStateContext, kr::ExtFamilyTag, tyr::LiftedTag>);
static_assert(!kr::dl::semantics::StateEvaluationContextConcept<const ExtStateContext&, kr::ExtFamilyTag, tyr::GroundTag>);
static_assert(!kr::dl::semantics::StateEvaluationContextConcept<ExtStateContext, int, tyr::GroundTag>);
static_assert(!kr::dl::semantics::StateEvaluationContextConcept<ExtStateContext, kr::ExtFamilyTag, int>);
static_assert(!kr::dl::semantics::StateEvaluationContextConcept<NameOnlyContext, kr::ExtFamilyTag, tyr::GroundTag>);

static_assert(kr::dl::semantics::StateEvaluationContextConcept<BaseStateContext&, kr::BaseFamilyTag, tyr::GroundTag>);
static_assert(!kr::dl::semantics::StateEvaluationContextConcept<const BaseStateContext&, kr::BaseFamilyTag, tyr::GroundTag>);
static_assert(!kr::dl::semantics::StateEvaluationContextConcept<BaseStateContext, kr::BaseFamilyTag, tyr::LiftedTag>);
static_assert(!kr::dl::semantics::StateEvaluationContextConcept<BaseStateContext, kr::ExtFamilyTag, tyr::GroundTag>);
static_assert(kr::ps::dl::TransitionEvaluationContextConcept<IcpContext, kr::IcpFamilyTag, tyr::GroundTag>);
static_assert(!kr::ps::dl::TransitionEvaluationContextConcept<IcpContext, kr::IcpFamilyTag, tyr::LiftedTag>);

static_assert(!kr::ps::IsTransitionEvaluationContext<kr::BaseFamilyTag, kr::DlTag, NameOnlyContext>);
static_assert(!kr::ps::IsTransitionEvaluationContext<kr::BaseFamilyTag, int, BaseContext>);
static_assert(!kr::ps::IsTransitionEvaluationContext<kr::BaseFamilyTag, kr::DlTag, ExtContext>);
static_assert(!kr::ps::IsTransitionEvaluationContext<kr::ExtFamilyTag, kr::DlTag, IcpContext>);

using BaseCondition = ygg::View<ygg::Index<kr::ps::ConditionVariant<kr::BaseFamilyTag>>, kr::ps::base::Repository>;
using ExtCondition = ygg::View<ygg::Index<kr::ps::ConditionVariant<kr::ExtFamilyTag>>, kr::ps::ext::Repository>;
using IcpCondition = ygg::View<ygg::Index<kr::ps::ConditionVariant<kr::IcpFamilyTag>>, kr::ps::icp::Repository>;

template<typename Kind, typename Condition, typename Context>
concept CanCheckCondition = requires(Condition condition, Context& context) {
    { kr::ps::is_compatible_with<Kind>(condition, context) } -> std::same_as<bool>;
};

static_assert(CanCheckCondition<tyr::GroundTag, BaseCondition, BaseContext> && CanCheckCondition<tyr::GroundTag, BaseCondition, BaseStateContext>);
static_assert(CanCheckCondition<tyr::GroundTag, ExtCondition, ExtContext> && CanCheckCondition<tyr::GroundTag, ExtCondition, ExtStateContext>);
static_assert(CanCheckCondition<tyr::GroundTag, IcpCondition, IcpContext> && CanCheckCondition<tyr::GroundTag, IcpCondition, ExtStateContext>);
static_assert(!CanCheckCondition<tyr::GroundTag, BaseCondition, ExtStateContext>);
static_assert(!CanCheckCondition<tyr::GroundTag, ExtCondition, BaseStateContext>);
static_assert(!CanCheckCondition<tyr::GroundTag, IcpCondition, ExtContext>);
static_assert(!CanCheckCondition<tyr::GroundTag, ExtCondition, IcpContext>);
static_assert(!CanCheckCondition<tyr::GroundTag, BaseCondition, NameOnlyContext>);
static_assert(!CanCheckCondition<tyr::LiftedTag, BaseCondition, BaseStateContext>);
static_assert(!CanCheckCondition<tyr::LiftedTag, BaseCondition, BaseContext>);

}  // namespace
}  // namespace runir::tests
