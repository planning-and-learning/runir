#include <concepts>
#include <runir/kr/dl/repository.hpp>
#include <runir/kr/ps/base/repository.hpp>
#include <runir/kr/ps/compatibility.hpp>
#include <runir/kr/ps/ext/repository.hpp>
#include <runir/kr/ps/icp/repository.hpp>
#include <runir/kr/uns/repository.hpp>
#include <type_traits>

namespace runir::tests
{
namespace
{

using IcpFactory = kr::ps::icp::RepositoryFactory;
using ExtConstructors = kr::dl::ConstructorRepositoryPtrFor<kr::ExtFamilyTag>;
using BaseConstructors = kr::dl::ConstructorRepositoryPtrFor<kr::BaseFamilyTag>;
using UnsConstructors = kr::dl::ConstructorRepositoryPtrFor<kr::UnsFamilyTag>;

// ICP shares Ext expressions, while policy indices remain distinct.
static_assert(std::same_as<kr::ps::icp::Repository::DlRepositoryPtr, ExtConstructors>);
static_assert(std::same_as<kr::ps::base::Repository::DlRepositoryPtr, BaseConstructors>);
static_assert(std::same_as<kr::uns::Repository::DlRepositoryPtr, UnsConstructors>);
static_assert(std::is_invocable_v<decltype(&IcpFactory::create), IcpFactory&, ExtConstructors>);
static_assert(!std::is_invocable_v<decltype(&IcpFactory::create), IcpFactory&, BaseConstructors>);
static_assert(!std::is_invocable_v<decltype(&IcpFactory::create), IcpFactory&, UnsConstructors>);
static_assert(
    !std::same_as<ygg::Index<kr::ps::Feature<kr::IcpFamilyTag, kr::dl::ConceptTag>>, ygg::Index<kr::ps::Feature<kr::ExtFamilyTag, kr::dl::ConceptTag>>>);

using IcpQuery = kr::ps::ConcreteFeature<kr::IcpFamilyTag, kr::DlTag, kr::ps::dl::QueryFeature>;
static_assert(std::same_as<typename ygg::Data<IcpQuery>::Expression, kr::dl::Query<kr::ExtFamilyTag>>);

// Classifiers expose Boolean features, without acquiring policy conditions or effects.
static_assert(std::same_as<kr::ps::PsConditionTypes<kr::UnsFamilyTag>, ygg::TypeList<>>);
static_assert(std::same_as<kr::ps::PsEffectTypes<kr::UnsFamilyTag>, ygg::TypeList<>>);

template<typename T>
concept StoredType = requires {
    sizeof(ygg::Index<T>);
    sizeof(ygg::Data<T>);
};

template<typename T, typename Repository>
concept ViewableType = StoredType<T> && requires { sizeof(ygg::View<ygg::Index<T>, Repository>); };

template<typename Repository, typename... Types>
consteval bool usable_inventory(ygg::TypeList<Types...>)
{
    return (ViewableType<Types, Repository> && ...);
}

static_assert(usable_inventory<kr::ps::base::Repository>(kr::ps::PsCoreTypes<kr::BaseFamilyTag> {}));
static_assert(usable_inventory<kr::ps::ext::Repository>(kr::ps::PsCoreTypes<kr::ExtFamilyTag> {}));
static_assert(usable_inventory<kr::ps::icp::Repository>(kr::ps::PsCoreTypes<kr::IcpFamilyTag> {}));
static_assert(usable_inventory<kr::uns::Repository>(kr::ps::PsCoreTypes<kr::UnsFamilyTag> {}));

// Default repository inventories do not restrict the reusable semantic types.
using BaseRole = kr::ps::ConcreteFeature<kr::BaseFamilyTag, kr::DlTag, kr::dl::RoleTag>;
using BaseQuery = kr::ps::ConcreteFeature<kr::BaseFamilyTag, kr::DlTag, kr::ps::dl::QueryFeature>;
using UnsNumerical = kr::ps::ConcreteFeature<kr::UnsFamilyTag, kr::DlTag, kr::ps::dl::NumericalFeature>;
using UnsPositive = kr::ps::ConcreteCondition<kr::UnsFamilyTag, kr::DlTag, kr::ps::dl::BooleanFeature, kr::ps::dl::Positive>;
using BaseCustomTypes = ygg::ConcatTypeListsT<
    kr::ps::PsCoreTypes<kr::BaseFamilyTag>,
    ygg::TypeList<BaseRole, BaseQuery, kr::ps::Feature<kr::BaseFamilyTag, kr::dl::RoleTag>, kr::ps::Feature<kr::BaseFamilyTag, kr::ps::dl::QueryFeature>>>;
using UnsCustomTypes = ygg::ConcatTypeListsT<kr::ps::PsCoreTypes<kr::UnsFamilyTag>,
                                             ygg::TypeList<UnsNumerical, kr::ps::Feature<kr::UnsFamilyTag, kr::ps::dl::NumericalFeature>>,
                                             kr::ps::detail::PsConditionTypes<kr::UnsFamilyTag>,
                                             kr::ps::detail::PsEffectTypes<kr::UnsFamilyTag>>;
using BaseCustomRepository = kr::ps::BasicRepository<kr::BaseFamilyTag, BaseCustomTypes>;
using UnsCustomRepository = kr::ps::BasicRepository<kr::UnsFamilyTag, UnsCustomTypes>;
static_assert(usable_inventory<BaseCustomRepository>(BaseCustomTypes {}));
static_assert(usable_inventory<UnsCustomRepository>(UnsCustomTypes {}));
static_assert(usable_inventory<kr::ps::ext::Repository>(kr::ps::PsCoreTypes<kr::BaseFamilyTag> {}));

using BaseFeatureInExt = ygg::View<ygg::Index<kr::ps::Feature<kr::BaseFamilyTag, kr::ps::dl::BooleanFeature>>, kr::ps::ext::Repository>;
using BaseConditionInExt = ygg::View<ygg::Index<kr::ps::ConditionVariant<kr::BaseFamilyTag>>, kr::ps::ext::Repository>;
static_assert(requires(BaseFeatureInExt feature, BaseConditionInExt condition) {
    feature.get_symbol();
    feature.get_variant();
    condition.get_variant();
});

// Storage contexts remain structural; no exact repository type is required.
struct BaseFeatureStorage
{
    const BaseCustomRepository& repository;
    const auto& get_dl_repository() const { return repository.get_dl_repository(); }
    const auto& get_index() const { return repository.get_index(); }
    friend const BaseCustomRepository& get_repository(const BaseFeatureStorage& storage) { return storage.repository; }
};

template<typename Type, typename Repository, typename Context>
concept CanEvaluate = StoredType<Type> && requires(ygg::View<ygg::Index<Type>, Repository>& view, Context& context) { kr::ps::evaluate<tyr::GroundTag>(view, context); };

using BaseStateContext = kr::dl::semantics::StateEvaluationContext<kr::BaseFamilyTag, tyr::GroundTag>;
using UnsStateContext = kr::dl::semantics::StateEvaluationContext<kr::UnsFamilyTag, tyr::GroundTag>;
using ExtStateContext = kr::dl::semantics::StateEvaluationContext<kr::ExtFamilyTag, tyr::GroundTag>;
using BaseBoolean = kr::ps::ConcreteFeature<kr::BaseFamilyTag, kr::DlTag, kr::ps::dl::BooleanFeature>;
using UnsBoolean = kr::ps::ConcreteFeature<kr::UnsFamilyTag, kr::DlTag, kr::ps::dl::BooleanFeature>;
static_assert(CanEvaluate<BaseBoolean, kr::ps::base::Repository, BaseStateContext>);
static_assert(CanEvaluate<UnsBoolean, kr::uns::Repository, UnsStateContext>);
static_assert(CanEvaluate<BaseRole, BaseCustomRepository, BaseStateContext>);
static_assert(CanEvaluate<BaseQuery, BaseCustomRepository, BaseStateContext>);
static_assert(CanEvaluate<BaseRole, BaseFeatureStorage, BaseStateContext>);
static_assert(CanEvaluate<kr::ps::Feature<kr::BaseFamilyTag, kr::dl::RoleTag>, BaseFeatureStorage, BaseStateContext>);
static_assert(CanEvaluate<UnsNumerical, UnsCustomRepository, UnsStateContext>);
static_assert(CanEvaluate<IcpQuery, kr::ps::icp::Repository, ExtStateContext>);
static_assert(!CanEvaluate<BaseBoolean, kr::ps::base::Repository, UnsStateContext>);
static_assert(!CanEvaluate<UnsBoolean, kr::uns::Repository, BaseStateContext>);

// Invalid semantic categories and observation pairs still fail substitution.
using InvalidFeature = kr::ps::ConcreteFeature<kr::BaseFamilyTag, kr::DlTag, int>;
using NumericalPositive = kr::ps::ConcreteCondition<kr::BaseFamilyTag, kr::DlTag, kr::ps::dl::NumericalFeature, kr::ps::dl::Positive>;
using BooleanDecreases = kr::ps::ConcreteEffect<kr::ExtFamilyTag, kr::DlTag, kr::ps::dl::BooleanFeature, kr::ps::dl::Decreases>;
static_assert(!StoredType<InvalidFeature>);
static_assert(!StoredType<kr::ps::Feature<kr::BaseFamilyTag, int>>);
static_assert(!StoredType<kr::ps::ConcreteFeature<kr::ExtFamilyTag, void, kr::ps::dl::BooleanFeature>>);
static_assert(!StoredType<NumericalPositive>);
static_assert(!StoredType<BooleanDecreases>);
static_assert(!CanEvaluate<InvalidFeature, BaseCustomRepository, BaseStateContext>);

template<typename Kind, typename Type, typename Repository, typename Context>
concept CanCheckCompatibility = requires(ygg::View<ygg::Index<Type>, Repository> view, Context& context) {
    { kr::ps::is_compatible_with<Kind>(view, context) } -> std::same_as<bool>;
};

template<typename Family, typename Repository>
consteval bool observation_contracts()
{
    using Context = kr::ps::dl::TransitionEvaluationContext<Family, tyr::GroundTag>;
    using Boolean = kr::ps::dl::BooleanFeature;
    using Numerical = kr::ps::dl::NumericalFeature;
    using Condition = kr::ps::ConcreteCondition<Family, kr::DlTag, Boolean, kr::ps::dl::Positive>;
    using Effect = kr::ps::ConcreteEffect<Family, kr::DlTag, Numerical, kr::ps::dl::Decreases>;
    static_assert(CanCheckCompatibility<tyr::GroundTag, Condition, Repository, Context>);
    static_assert(CanCheckCompatibility<tyr::GroundTag, Effect, Repository, Context>);
    static_assert(!CanCheckCompatibility<tyr::LiftedTag, Condition, Repository, Context>);
    static_assert(!CanCheckCompatibility<tyr::LiftedTag, Effect, Repository, Context>);
    static_assert(!CanCheckCompatibility<tyr::GroundTag, kr::ps::ConcreteCondition<Family, kr::DlTag, Numerical, kr::ps::dl::Positive>, Repository, Context>);
    static_assert(!CanCheckCompatibility<tyr::GroundTag, kr::ps::ConcreteEffect<Family, kr::DlTag, Boolean, kr::ps::dl::Decreases>, Repository, Context>);
    static_assert(!CanCheckCompatibility<tyr::GroundTag, kr::ps::ConcreteCondition<Family, void, Boolean, kr::ps::dl::Positive>, Repository, Context>);
    static_assert(!CanCheckCompatibility<tyr::GroundTag, kr::ps::ConcreteEffect<Family, void, Numerical, kr::ps::dl::Decreases>, Repository, Context>);
    static_assert(!CanCheckCompatibility<tyr::GroundTag, kr::ps::ConcreteConditionVariant<Family, void>, Repository, Context>);
    static_assert(!CanCheckCompatibility<tyr::GroundTag, kr::ps::ConcreteEffectVariant<Family, void>, Repository, Context>);

    // The variant alternatives and repository inventory share one ordered list.
    using Conditions = ygg::ApplyTypeListT<::cista::offset::variant, ygg::MapTypeListT<ygg::Index, kr::ps::detail::PsConcreteConditionTypes<Family>>>;
    using Effects = ygg::ApplyTypeListT<::cista::offset::variant, ygg::MapTypeListT<ygg::Index, kr::ps::detail::PsConcreteEffectTypes<Family>>>;
    static_assert(std::same_as<typename ygg::Data<kr::ps::ConcreteConditionVariant<Family, kr::DlTag>>::Variant, Conditions>);
    static_assert(std::same_as<typename ygg::Data<kr::ps::ConcreteEffectVariant<Family, kr::DlTag>>::Variant, Effects>);
    return true;
}

static_assert(observation_contracts<kr::BaseFamilyTag, kr::ps::base::Repository>());
static_assert(observation_contracts<kr::ExtFamilyTag, kr::ps::ext::Repository>());
static_assert(observation_contracts<kr::IcpFamilyTag, kr::ps::icp::Repository>());
static_assert(observation_contracts<kr::UnsFamilyTag, UnsCustomRepository>());

// Compile the implementation body as well as checking its callable signature.
[[maybe_unused]] bool check_custom_uns_condition(ygg::View<ygg::Index<UnsPositive>, UnsCustomRepository> condition, UnsStateContext& context)
{
    return kr::ps::is_compatible_with<tyr::GroundTag>(condition, context);
}

}  // namespace
}  // namespace runir::tests
