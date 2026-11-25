#include <concepts>
#include <runir/kr/dl/repository.hpp>
#include <runir/kr/dl/semantics/denotation_repository.hpp>
#include <runir/kr/dl/semantics/evaluation_storage.hpp>
#include <runir/kr/dl/semantics/register_values_view.hpp>
#include <runir/kr/ps/base/repository.hpp>
#include <runir/kr/ps/compatibility.hpp>
#include <runir/kr/ps/ext/repository.hpp>
#include <runir/kr/ps/icp/repository.hpp>
#include <runir/kr/uns/repository.hpp>
#include <type_traits>
#include <tyr/planning/state_repository.hpp>
#include <utility>

namespace runir::tests::ps_family_traits
{
namespace
{

using ExtConstructors = kr::dl::ConstructorRepositoryPtr;
using UnsConstructors = kr::dl::ConstructorRepositoryPtr;

// ICP shares Ext expressions, while policy indices remain distinct.
static_assert(std::same_as<kr::ps::icp::Repository::DlRepositoryPtr, ExtConstructors>);
static_assert(std::same_as<kr::ps::base::Repository::DlRepositoryPtr, ExtConstructors>);
static_assert(std::same_as<kr::uns::Repository::DlRepositoryPtr, UnsConstructors>);
static_assert(std::is_constructible_v<kr::ps::icp::Repository, ExtConstructors>);
static_assert(
    !std::same_as<urd::index<kr::ps::Feature<kr::IcpFamilyTag, kr::dl::ConceptTag>>, urd::index<kr::ps::Feature<kr::ExtFamilyTag, kr::dl::ConceptTag>>>);

using IcpQuery = kr::ps::ConcreteFeature<kr::IcpFamilyTag, kr::ps::dl::QueryFeature>;
static_assert(std::same_as<decltype(urd::data<IcpQuery>::feature), urd::index<kr::dl::Query<>>>);

// Classifiers expose Boolean features, without acquiring policy conditions or effects.
static_assert(std::same_as<kr::ps::PsConditionTypes<kr::UnsFamilyTag>, boost::mp11::mp_list<>>);
static_assert(std::same_as<kr::ps::PsEffectTypes<kr::UnsFamilyTag>, boost::mp11::mp_list<>>);

template<typename T>
concept StoredType = requires {
    sizeof(urd::index<T>);
    sizeof(urd::data<T>);
};

template<typename T, typename Repository>
concept ViewableType = StoredType<T> && requires { sizeof(urd::index_view<T, Repository>); };

template<typename Repository, typename... Types>
consteval bool usable_inventory(boost::mp11::mp_list<Types...>)
{ return (ViewableType<Types, Repository> && ...); }

static_assert(usable_inventory<kr::ps::base::Repository>(kr::ps::PsCoreTypes<kr::BaseFamilyTag> {}));
static_assert(usable_inventory<kr::ps::ext::Repository>(kr::ps::PsCoreTypes<kr::ExtFamilyTag> {}));
static_assert(usable_inventory<kr::ps::icp::Repository>(kr::ps::PsCoreTypes<kr::IcpFamilyTag> {}));
static_assert(usable_inventory<kr::uns::Repository>(kr::ps::PsCoreTypes<kr::UnsFamilyTag> {}));

// Default repository inventories do not restrict the reusable semantic types.
using BaseRole = kr::ps::ConcreteFeature<kr::BaseFamilyTag, kr::dl::RoleTag>;
using BaseQuery = kr::ps::ConcreteFeature<kr::BaseFamilyTag, kr::ps::dl::QueryFeature>;
using UnsNumerical = kr::ps::ConcreteFeature<kr::UnsFamilyTag, kr::dl::NumericalTag>;
using UnsPositive = kr::ps::ConcreteCondition<kr::UnsFamilyTag, kr::dl::BooleanTag, kr::ps::dl::Positive>;
using BaseCustomTypes = boost::mp11::mp_append<
    kr::ps::PsCoreTypes<kr::BaseFamilyTag>,
    boost::mp11::
        mp_list<BaseRole, BaseQuery, kr::ps::Feature<kr::BaseFamilyTag, kr::dl::RoleTag>, kr::ps::Feature<kr::BaseFamilyTag, kr::ps::dl::QueryFeature>>>;
using UnsCustomTypes = boost::mp11::mp_append<kr::ps::PsCoreTypes<kr::UnsFamilyTag>,
                                              boost::mp11::mp_list<UnsNumerical, kr::ps::Feature<kr::UnsFamilyTag, kr::dl::NumericalTag>>,
                                              kr::ps::detail::PsConcreteConditionTypes<kr::UnsFamilyTag>,
                                              kr::ps::detail::PsConcreteEffectTypes<kr::UnsFamilyTag>>;

}  // namespace

// Outside the anonymous namespace: yggdrasil's parts over a repository are declared in its headers and defined where a
// library instantiates them, which a class without linkage forbids (clang: used but never defined).
// NOLINTNEXTLINE(misc-use-internal-linkage): yggdrasil's parts over it are defined where a library instantiates them
class BaseCustomRepository final : public kr::ps::BasicRepository<BaseCustomRepository, kr::BaseFamilyTag, BaseCustomTypes>
{
public:
    explicit BaseCustomRepository(DlRepositoryPtr dl_repository) :
        kr::ps::BasicRepository<BaseCustomRepository, kr::BaseFamilyTag, BaseCustomTypes>(std::move(dl_repository))
    {
    }
};

// NOLINTNEXTLINE(misc-use-internal-linkage): yggdrasil's parts over it are defined where a library instantiates them
class UnsCustomRepository final : public kr::ps::BasicRepository<UnsCustomRepository, kr::UnsFamilyTag, UnsCustomTypes>
{
public:
    explicit UnsCustomRepository(DlRepositoryPtr dl_repository) :
        kr::ps::BasicRepository<UnsCustomRepository, kr::UnsFamilyTag, UnsCustomTypes>(std::move(dl_repository))
    {
    }
};

namespace
{
static_assert(usable_inventory<BaseCustomRepository>(BaseCustomTypes {}));
static_assert(usable_inventory<UnsCustomRepository>(UnsCustomTypes {}));
static_assert(usable_inventory<kr::ps::ext::Repository>(kr::ps::PsCoreTypes<kr::BaseFamilyTag> {}));

using BaseFeatureInExt = urd::index_view<kr::ps::Feature<kr::BaseFamilyTag, kr::dl::BooleanTag>, kr::ps::ext::Repository>;
static_assert(requires(BaseFeatureInExt feature) { feature.get_variant(); });

// Storage contexts remain structural; no exact repository type is required.
struct BaseFeatureStorage
{
    const BaseCustomRepository* repository;

    [[nodiscard]] const auto& get_dl_repository() const { return repository->get<runir::kr::dl::ConstructorRepository>(); }

    [[nodiscard]] std::size_t layer() const { return repository->layer(); }

    template<typename T>
    [[nodiscard]] decltype(auto) operator[](urd::index<T> index) const
    { return (*repository)[index]; }

    template<typename T>
    [[nodiscard]] urd::index_view<T, BaseFeatureStorage> view(urd::index<T> index) const
    { return { index, this }; }

    /// A DL record as its view in the DL repository, which holds it.
    template<typename T>
        requires ygg::formalism::SupportsSymbol<kr::dl::ConstructorRepository, T>
    [[nodiscard]] auto view(urd::index<T> index) const
    { return get_dl_repository().view(index); }
};

template<typename Type, typename Repository, typename Context>
concept CanEvaluate =
    StoredType<Type> && requires(urd::index_view<Type, Repository>& view, Context& context) { kr::ps::evaluate<tyr::GroundTag>(view, context); };

using BaseStateContext = kr::dl::semantics::StateEvaluationContext<tyr::GroundTag>;
using UnsStateContext = kr::dl::semantics::StateEvaluationContext<tyr::GroundTag>;
using ExtStateContext = kr::dl::semantics::StateEvaluationContext<tyr::GroundTag>;
using BaseBoolean = kr::ps::ConcreteFeature<kr::BaseFamilyTag, kr::dl::BooleanTag>;
using UnsBoolean = kr::ps::ConcreteFeature<kr::UnsFamilyTag, kr::dl::BooleanTag>;
static_assert(CanEvaluate<BaseBoolean, kr::ps::base::Repository, BaseStateContext>);
static_assert(CanEvaluate<UnsBoolean, kr::uns::Repository, UnsStateContext>);
static_assert(CanEvaluate<BaseRole, BaseCustomRepository, BaseStateContext>);
static_assert(CanEvaluate<BaseQuery, BaseCustomRepository, BaseStateContext>);
static_assert(CanEvaluate<BaseRole, BaseFeatureStorage, BaseStateContext>);
static_assert(CanEvaluate<kr::ps::Feature<kr::BaseFamilyTag, kr::dl::RoleTag>, BaseFeatureStorage, BaseStateContext>);
static_assert(CanEvaluate<UnsNumerical, UnsCustomRepository, UnsStateContext>);
static_assert(CanEvaluate<IcpQuery, kr::ps::icp::Repository, ExtStateContext>);

// Invalid semantic categories and observation pairs still fail substitution.
using InvalidFeature = kr::ps::ConcreteFeature<kr::BaseFamilyTag, int>;
using NumericalPositive = kr::ps::ConcreteCondition<kr::BaseFamilyTag, kr::dl::NumericalTag, kr::ps::dl::Positive>;
using BooleanDecreases = kr::ps::ConcreteEffect<kr::ExtFamilyTag, kr::dl::BooleanTag, kr::ps::dl::Decreases>;
static_assert(!StoredType<InvalidFeature>);
static_assert(!StoredType<kr::ps::Feature<kr::BaseFamilyTag, int>>);
static_assert(!StoredType<NumericalPositive>);
static_assert(!StoredType<BooleanDecreases>);
static_assert(!CanEvaluate<InvalidFeature, BaseCustomRepository, BaseStateContext>);

template<typename Kind, typename Type, typename Repository, typename Context>
concept CanCheckCompatibility = requires(urd::index_view<Type, Repository> view, Context& context) {
    { kr::ps::is_compatible_with<Kind>(view, context) } -> std::same_as<bool>;
};

template<typename Family, typename Repository>
consteval bool observation_contracts()
{
    using Context = kr::ps::dl::TransitionEvaluationContext<Family, tyr::GroundTag>;
    using Boolean = kr::dl::BooleanTag;
    using Numerical = kr::dl::NumericalTag;
    using Condition = kr::ps::ConcreteCondition<Family, Boolean, kr::ps::dl::Positive>;
    using Effect = kr::ps::ConcreteEffect<Family, Numerical, kr::ps::dl::Decreases>;
    static_assert(CanCheckCompatibility<tyr::GroundTag, Condition, Repository, Context>);
    static_assert(CanCheckCompatibility<tyr::GroundTag, Effect, Repository, Context>);
    static_assert(!CanCheckCompatibility<tyr::LiftedTag, Condition, Repository, Context>);
    static_assert(!CanCheckCompatibility<tyr::LiftedTag, Effect, Repository, Context>);
    static_assert(!CanCheckCompatibility<tyr::GroundTag, kr::ps::ConcreteCondition<Family, Numerical, kr::ps::dl::Positive>, Repository, Context>);
    static_assert(!CanCheckCompatibility<tyr::GroundTag, kr::ps::ConcreteEffect<Family, Boolean, kr::ps::dl::Decreases>, Repository, Context>);

    // The variant alternatives and repository inventory share one ordered list.
    using Conditions = urd::index_variant<kr::ps::detail::PsConcreteConditionTypes<Family>>;
    using Effects = urd::index_variant<kr::ps::detail::PsConcreteEffectTypes<Family>>;
    static_assert(std::same_as<kr::ps::ConditionVariant<Family>, Conditions>);
    static_assert(std::same_as<kr::ps::EffectVariant<Family>, Effects>);
    return true;
}

static_assert(observation_contracts<kr::BaseFamilyTag, kr::ps::base::Repository>());
static_assert(observation_contracts<kr::ExtFamilyTag, kr::ps::ext::Repository>());
static_assert(observation_contracts<kr::IcpFamilyTag, kr::ps::icp::Repository>());
static_assert(observation_contracts<kr::UnsFamilyTag, UnsCustomRepository>());

// Compile the implementation body as well as checking its callable signature.
[[maybe_unused]] bool check_custom_uns_condition(urd::index_view<UnsPositive, UnsCustomRepository> condition, UnsStateContext& context)
{ return kr::ps::is_compatible_with<tyr::GroundTag>(condition, context); }

}  // namespace
}  // namespace runir::tests::ps_family_traits
