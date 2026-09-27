#include <concepts>
#include <runir/kr/ps/base/repository.hpp>
#include <runir/kr/ps/compatibility.hpp>
#include <runir/kr/ps/ext/repository.hpp>
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
    using DlFamily = typename kr::ps::PsFamilyTraits<Family>::DlFamily;
    using StateContext = kr::dl::semantics::StateEvaluationContext<DlFamily, Kind>;
    return kr::ps::IsTransitionEvaluationContext<Family, kr::DlTag, Context>
           && std::same_as<decltype(std::declval<Context&>().get_source_context()), StateContext&>
           && std::same_as<decltype(std::declval<Context&>().get_target_context()), StateContext&>;
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

static_assert(!kr::ps::IsTransitionEvaluationContext<kr::BaseFamilyTag, kr::DlTag, NameOnlyContext>);
static_assert(!kr::ps::IsTransitionEvaluationContext<kr::BaseFamilyTag, int, BaseContext>);
static_assert(!kr::ps::IsTransitionEvaluationContext<kr::BaseFamilyTag, kr::DlTag, ExtContext>);
static_assert(!kr::ps::IsTransitionEvaluationContext<kr::ExtFamilyTag, kr::DlTag, IcpContext>);

using BaseCondition = ygg::View<ygg::Index<kr::ps::ConditionVariant<kr::BaseFamilyTag>>, kr::ps::base::Repository>;
using ExtCondition = ygg::View<ygg::Index<kr::ps::ConditionVariant<kr::ExtFamilyTag>>, kr::ps::ext::Repository>;
using IcpCondition = ygg::View<ygg::Index<kr::ps::ConditionVariant<kr::IcpFamilyTag>>, kr::ps::icp::Repository>;

template<typename Condition, typename Context>
concept CanCheckCondition = requires(Condition condition, Context& context) {
    { kr::ps::is_compatible_with(condition, context) } -> std::same_as<bool>;
};

static_assert(CanCheckCondition<BaseCondition, BaseContext> && CanCheckCondition<BaseCondition, BaseStateContext>);
static_assert(CanCheckCondition<ExtCondition, ExtContext> && CanCheckCondition<ExtCondition, ExtStateContext>);
static_assert(CanCheckCondition<IcpCondition, IcpContext> && CanCheckCondition<IcpCondition, ExtStateContext>);
static_assert(!CanCheckCondition<BaseCondition, ExtStateContext>);
static_assert(!CanCheckCondition<ExtCondition, BaseStateContext>);
static_assert(!CanCheckCondition<IcpCondition, ExtContext>);
static_assert(!CanCheckCondition<ExtCondition, IcpContext>);
static_assert(!CanCheckCondition<BaseCondition, NameOnlyContext>);

}  // namespace
}  // namespace runir::tests
