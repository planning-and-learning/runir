#include <concepts>
#include <gtest/gtest.h>
#include <runir/kr/dl/repository.hpp>
#include <tyr/formalism/planning/repository.hpp>

namespace runir::tests
{

namespace
{

template<typename T, typename Repository>
concept IndexedDataView =
    std::constructible_from<urd::index<T>, ygg::uint_t> && urd::Identifiable<urd::index<T>> && urd::Identifiable<urd::data<T>>
    && urd::Identifiable<urd::view<T, Repository>> && std::totally_ordered<urd::index<T>> && std::totally_ordered<urd::data<T>>
    && std::totally_ordered<urd::view<T, Repository>> && requires(urd::data<T>& data, const urd::view<T, Repository>& view) {
           data.index;
           view.index();
       };

template<typename... Ts>
consteval bool indexed_data_views(boost::mp11::mp_list<Ts...>)
{ return (IndexedDataView<Ts, kr::dl::ConstructorRepository> && ...); }

static_assert(indexed_data_views<>(kr::dl::RoleTypes {}));

using PredicateTags = boost::mp11::mp_list<kr::dl::AtomicStateTag<tyr::formalism::StaticTag>,
                                    kr::dl::AtomicStateTag<tyr::formalism::FluentTag>,
                                    kr::dl::AtomicStateTag<tyr::formalism::DerivedTag>,
                                    kr::dl::AtomicGoalTag<tyr::formalism::StaticTag>,
                                    kr::dl::AtomicGoalTag<tyr::formalism::FluentTag>,
                                    kr::dl::AtomicGoalTag<tyr::formalism::DerivedTag>>;
using BinaryTags = boost::mp11::mp_list<kr::dl::IntersectionTag, kr::dl::UnionTag, kr::dl::CompositionTag, kr::dl::RestrictionTag>;
using UnaryTags =
    boost::mp11::mp_list<kr::dl::ComplementTag, kr::dl::InverseTag, kr::dl::TransitiveClosureTag, kr::dl::ReflexiveTransitiveClosureTag, kr::dl::IdentityTag>;

template<typename Tag>
consteval bool role_data_view()
{
    using Entity = kr::dl::Role<Tag>;
    using Data = urd::data<Entity>;
    using View = urd::view<Entity, kr::dl::ConstructorRepository>;

    static_assert(!requires(const View& view) { view.get_columns(); });
    static_assert(!requires(const View& view) { view.get_plan(); });
    static_assert(std::same_as<View, kr::dl::RoleView<Tag>>);
    if constexpr (false)
        static_assert(std::same_as<View, kr::dl::RoleView<Tag>>);
    else if constexpr (true)
        static_assert(std::same_as<View, kr::dl::RoleView<Tag>>);
    else
        static_assert(std::same_as<View, kr::dl::RoleView<Tag>>);

    if constexpr (std::same_as<Tag, kr::dl::UniversalTag>)
        return requires(Data& data, const View& view) {
            data.index;
            view.index();
        };
    else if constexpr (boost::mp11::mp_contains<PredicateTags, Tag>::value)
        return requires(Data& data, const View& view) {
            data.predicate;
            data.polarity;
            view.get_predicate();
            view.get_polarity();
        };
    else if constexpr (boost::mp11::mp_contains<BinaryTags, Tag>::value)
        return requires(Data& data, const View& view) {
            data.lhs;
            data.rhs;
            view.get_lhs();
            view.get_rhs();
        };
    else if constexpr (boost::mp11::mp_contains<UnaryTags, Tag>::value)
        return requires(Data& data, const View& view) {
            data.arg;
            view.get_arg();
        };
    else if constexpr (std::same_as<Tag, kr::dl::RegisterTag>)
        return requires(Data& data, const View& view) {
            data.reference;
            view.get_register();
        };
    else if constexpr (std::same_as<Tag, kr::dl::ArgumentTag<kr::dl::RoleTag>>)
        return requires(Data& data, const View& view) {
            data.reference;
            view.get_argument();
        };
    else
        return false;
}

template<typename... Tags>
consteval bool role_data_views(boost::mp11::mp_list<Tags...>)
{ return (role_data_view<Tags>() && ...); }

static_assert(role_data_views<>(kr::dl::RoleConstructorTags {}));

}  // namespace

TEST(RunirKrDlRole, RoutesUnaryAndMixedBinaryData)
{
    using Concept = kr::dl::Constructor<kr::dl::ConceptTag>;
    using Role = kr::dl::Constructor<kr::dl::RoleTag>;
    auto planning_repository = tyr::formalism::planning::RepositoryFactory().create_shared();
    auto repository = kr::dl::ConstructorRepositoryFactory().create(planning_repository);

    auto unary_data = urd::data<kr::dl::Role<kr::dl::InverseTag>>(urd::index<Role>(3));
    const auto unary = repository->insert(unary_data).first;
    EXPECT_EQ(unary.get_arg().index(), urd::index<Role>(3));

    auto binary_data = urd::data<kr::dl::Role<kr::dl::RestrictionTag>>(urd::index<Role>(1), urd::index<Concept>(2));
    const auto binary = repository->insert(binary_data).first;
    EXPECT_EQ(binary.get_lhs().index(), urd::index<Role>(1));
    EXPECT_EQ(binary.get_rhs().index(), urd::index<Concept>(2));
}
}
