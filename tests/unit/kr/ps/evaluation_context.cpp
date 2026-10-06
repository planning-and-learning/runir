#include <concepts>
#include <runir/kr/ps/base/compatibility.hpp>
#include <runir/kr/ps/base/repository.hpp>
#include <runir/kr/ps/compatibility.hpp>
#include <runir/kr/ps/ext/repository.hpp>
#include <runir/kr/ps/family_traits.hpp>
#include <runir/kr/ps/icp/repository.hpp>
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
    return kr::ps::dl::TransitionEvaluationContextConcept<Context, Family, Kind>
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
template<kr::FamilyTag Family>
struct ResourceContext
{
    using FamilyType = Family;

    kr::ps::dl::TransitionEvaluationContext<Family, tyr::GroundTag>& transition;
    auto& get_source_context() { return transition.get_source_context(); }
    auto& get_target_context() { return transition.get_target_context(); }
};
using IcpResourceContext = ResourceContext<kr::IcpFamilyTag>;
using BaseResourceContext = ResourceContext<kr::BaseFamilyTag>;

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

using BaseCondition = ygg::View<ygg::Index<kr::ps::ConditionVariant<kr::BaseFamilyTag>>, kr::ps::base::Repository>;
using ExtCondition = ygg::View<ygg::Index<kr::ps::ConditionVariant<kr::ExtFamilyTag>>, kr::ps::ext::Repository>;
using IcpCondition = ygg::View<ygg::Index<kr::ps::ConditionVariant<kr::IcpFamilyTag>>, kr::ps::icp::Repository>;
using IcpEffect = ygg::View<ygg::Index<kr::ps::EffectVariant<kr::IcpFamilyTag>>, kr::ps::icp::Repository>;

using kr::ps::is_compatible_with;
using kr::ps::base::is_compatible_with;

template<typename Kind, typename Value, typename Context>
concept CanCheckCompatibility = requires(Value value, Context& context) {
    { is_compatible_with<Kind>(value, context) } -> std::same_as<bool>;
};

static_assert(CanCheckCompatibility<tyr::GroundTag, BaseCondition, BaseContext> && CanCheckCompatibility<tyr::GroundTag, BaseCondition, BaseStateContext>);
static_assert(CanCheckCompatibility<tyr::GroundTag, ExtCondition, ExtContext> && CanCheckCompatibility<tyr::GroundTag, ExtCondition, ExtStateContext>);
static_assert(CanCheckCompatibility<tyr::GroundTag, IcpCondition, IcpContext> && CanCheckCompatibility<tyr::GroundTag, IcpCondition, ExtStateContext>);
static_assert(!CanCheckCompatibility<tyr::GroundTag, BaseCondition, ExtStateContext>);
static_assert(!CanCheckCompatibility<tyr::GroundTag, ExtCondition, BaseStateContext>);
static_assert(!CanCheckCompatibility<tyr::GroundTag, IcpCondition, ExtContext>);
static_assert(!CanCheckCompatibility<tyr::GroundTag, ExtCondition, IcpContext>);
static_assert(!CanCheckCompatibility<tyr::GroundTag, BaseCondition, NameOnlyContext>);
static_assert(!CanCheckCompatibility<tyr::LiftedTag, BaseCondition, BaseStateContext>);
static_assert(!CanCheckCompatibility<tyr::LiftedTag, BaseCondition, BaseContext>);

static_assert(CanCheckCompatibility<tyr::GroundTag, IcpCondition, IcpResourceContext>);
static_assert(CanCheckCompatibility<tyr::GroundTag, IcpEffect, IcpResourceContext>);
static_assert(!CanCheckCompatibility<tyr::LiftedTag, IcpCondition, IcpResourceContext>);
static_assert(!CanCheckCompatibility<tyr::LiftedTag, IcpEffect, IcpResourceContext>);
static_assert(!CanCheckCompatibility<tyr::GroundTag, ExtCondition, IcpResourceContext>);
static_assert(!CanCheckCompatibility<tyr::GroundTag, IcpEffect, ExtContext>);
static_assert(!CanCheckCompatibility<tyr::GroundTag, IcpEffect, ExtStateContext>);

static_assert(CanCheckCompatibility<tyr::GroundTag, kr::ps::base::RuleView, BaseResourceContext>);
static_assert(CanCheckCompatibility<tyr::GroundTag, kr::ps::base::SketchView, BaseResourceContext>);
static_assert(!CanCheckCompatibility<tyr::LiftedTag, kr::ps::base::RuleView, BaseResourceContext>);
static_assert(!CanCheckCompatibility<tyr::LiftedTag, kr::ps::base::SketchView, BaseResourceContext>);
static_assert(!CanCheckCompatibility<tyr::GroundTag, kr::ps::base::RuleView, IcpResourceContext>);
static_assert(!CanCheckCompatibility<tyr::GroundTag, kr::ps::base::SketchView, IcpResourceContext>);

// Instantiate the compatibility bodies through a structural transition context.
[[maybe_unused]] bool check_structural_compatibility(IcpCondition condition,
                                                     IcpEffect effect,
                                                     IcpResourceContext& context,
                                                     kr::ps::base::RuleView rule,
                                                     kr::ps::base::SketchView sketch,
                                                     BaseResourceContext& base_context)
{
    return kr::ps::is_compatible_with<tyr::GroundTag>(condition, context) && kr::ps::is_compatible_with<tyr::GroundTag>(effect, context)
           && kr::ps::base::is_compatible_with<tyr::GroundTag>(rule, base_context) && kr::ps::base::is_compatible_with<tyr::GroundTag>(sketch, base_context);
}

}  // namespace
}  // namespace runir::tests
