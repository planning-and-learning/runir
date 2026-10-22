#include <concepts>
#include <gtest/gtest.h>
#include <runir/kr/dl/repository.hpp>
#include <tyr/formalism/planning/repository.hpp>
#include <yggdrasil/core/concepts.hpp>

namespace runir::tests
{

namespace
{

template<typename T, typename Repository>
concept IndexedDataView =
    std::constructible_from<ygg::Index<T>, ygg::uint_t> && ygg::Identifiable<ygg::Index<T>> && ygg::Identifiable<ygg::Data<T>>
    && ygg::Identifiable<ygg::View<ygg::Index<T>, Repository>> && std::totally_ordered<ygg::Index<T>> && std::totally_ordered<ygg::Data<T>>
    && std::totally_ordered<ygg::View<ygg::Index<T>, Repository>> && requires(ygg::Data<T>& data, const ygg::View<ygg::Index<T>, Repository>& view) {
           data.index;
           data.clear();
           view.get_index();
       };

template<typename... Ts>
consteval bool indexed_data_views(ygg::TypeList<Ts...>)
{
    return (IndexedDataView<Ts, kr::dl::ConstructorRepositoryFor> && ...);
}

static_assert(indexed_data_views<>(kr::dl::FamilyRoleTypes {}));

using PredicateTags = ygg::TypeList<kr::dl::AtomicStateTag<tyr::formalism::StaticTag>,
                                    kr::dl::AtomicStateTag<tyr::formalism::FluentTag>,
                                    kr::dl::AtomicStateTag<tyr::formalism::DerivedTag>,
                                    kr::dl::AtomicGoalTag<tyr::formalism::StaticTag>,
                                    kr::dl::AtomicGoalTag<tyr::formalism::FluentTag>,
                                    kr::dl::AtomicGoalTag<tyr::formalism::DerivedTag>>;
using BinaryTags = ygg::TypeList<kr::dl::IntersectionTag, kr::dl::UnionTag, kr::dl::CompositionTag, kr::dl::RestrictionTag>;
using UnaryTags =
    ygg::TypeList<kr::dl::ComplementTag, kr::dl::InverseTag, kr::dl::TransitiveClosureTag, kr::dl::ReflexiveTransitiveClosureTag, kr::dl::IdentityTag>;

template<typename Tag>
consteval bool role_data_view()
{
    using Entity = kr::dl::Role<Tag>;
    using Data = ygg::Data<Entity>;
    using View = ygg::View<ygg::Index<Entity>, kr::dl::ConstructorRepositoryFor>;

    static_assert(!requires(const View& view) { view.get_columns(); });
    static_assert(!requires(const View& view) { view.get_plan(); });
    static_assert(std::same_as<View, kr::dl::FamilyRoleView<Tag>>);
    if constexpr (false)
        static_assert(std::same_as<View, kr::dl::BaseRoleView<Tag>>);
    else if constexpr (true)
        static_assert(std::same_as<View, kr::dl::ExtRoleView<Tag>>);
    else
        static_assert(std::same_as<View, kr::dl::UnsRoleView<Tag>>);

    if constexpr (std::same_as<Tag, kr::dl::UniversalTag>)
        return requires(Data& data, const View& view) {
            data.index;
            view.get_index();
        };
    else if constexpr (ygg::InTypeList<Tag, PredicateTags>)
        return requires(Data& data, const View& view) {
            data.predicate;
            data.polarity;
            view.get_predicate();
            view.get_polarity();
        };
    else if constexpr (ygg::InTypeList<Tag, BinaryTags>)
        return requires(Data& data, const View& view) {
            data.lhs;
            data.rhs;
            view.get_lhs();
            view.get_rhs();
        };
    else if constexpr (ygg::InTypeList<Tag, UnaryTags>)
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
consteval bool role_data_views(ygg::TypeList<Tags...>)
{
    return (role_data_view<Tags>() && ...);
}

static_assert(role_data_views<>(kr::dl::FamilyRoleConstructorTags {}));

}  // namespace

TEST(RunirKrDlRole, RoutesUnaryAndMixedBinaryData)
{
    using Concept = kr::dl::Constructor<kr::dl::ConceptTag>;
    using Role = kr::dl::Constructor<kr::dl::RoleTag>;
    auto planning_repository = tyr::formalism::planning::RepositoryFactory().create_shared();
    auto repository = kr::dl::ExtConstructorRepositoryFactory().create(planning_repository);

    auto unary_data = ygg::Data<kr::dl::Role<kr::dl::InverseTag>>(ygg::Index<Role>(3));
    const auto unary = repository->insert(unary_data).first;
    EXPECT_EQ(unary.get_arg().get_index(), ygg::Index<Role>(3));

    auto binary_data = ygg::Data<kr::dl::Role<kr::dl::RestrictionTag>>(ygg::Index<Role>(1), ygg::Index<Concept>(2));
    const auto binary = repository->insert(binary_data).first;
    EXPECT_EQ(binary.get_lhs().get_index(), ygg::Index<Role>(1));
    EXPECT_EQ(binary.get_rhs().get_index(), ygg::Index<Concept>(2));
}
}
