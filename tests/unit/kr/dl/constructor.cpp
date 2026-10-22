#include <concepts>
#include <gtest/gtest.h>
#include <runir/kr/dl/detail/repository_instantiations_ext.hpp>
#include <runir/kr/dl/repository.hpp>
#include <tyr/formalism/planning/repository.hpp>
#include <utility>
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
           data.variant;
           data.clear();
           view.get_index();
           view.get_variant();
       };

template<typename... Ts>
consteval bool indexed_data_views(ygg::TypeList<Ts...>)
{
    return (IndexedDataView<Ts, kr::dl::ConstructorRepositoryFor> && ...);
}

template<typename Category>
consteval bool view_alias()
{
    using Entity = kr::dl::Constructor<Category>;
    using View = ygg::View<ygg::Index<Entity>, kr::dl::ConstructorRepositoryFor>;
    static_assert(std::same_as<View, kr::dl::FamilyConstructorView<Category>>);
    if constexpr (false)
        return std::same_as<View, kr::dl::BaseConstructorView<Category>>;
    else if constexpr (true)
        return std::same_as<View, kr::dl::ExtConstructorView<Category>>;
    else
        return std::same_as<View, kr::dl::UnsConstructorView<Category>>;
}

template<typename... Categories>
consteval bool view_aliases(ygg::TypeList<Categories...>)
{
    return (view_alias<Categories>() && ...);
}

template<typename Category>
consteval bool query_projection_contract()
{
    using Entity = kr::dl::QueryProjection<Category>;
    using Repository = kr::dl::ConstructorRepositoryFor;
    using View = ygg::View<ygg::Index<Entity>, Repository>;
    return ygg::ViewConcept<ygg::Index<Entity>, Repository> && requires(ygg::Data<Entity>& data, const View& view) {
        data.arg;
        data.columns;
        view.get_arg();
        view.get_columns();
    } && !requires(const View& view) { view.get_plan(); };
}

template<typename... Tags>
consteval bool query_view_contracts(ygg::TypeList<Tags...>)
{
    using Repository = kr::dl::ConstructorRepositoryFor;
    return ygg::ViewConcept<ygg::Index<kr::dl::QueryColumn>, Repository> && ygg::ViewConcept<ygg::Index<kr::dl::Query<>>, Repository>
           && query_projection_contract<kr::dl::ConceptTag>() && query_projection_contract<kr::dl::RoleTag>()
           && (ygg::ViewConcept<ygg::Index<kr::dl::Query<Tags>>, Repository> && ...);
}

static_assert(query_view_contracts<>(kr::dl::QueryConstructorTags {}));

static_assert(indexed_data_views<>(kr::dl::FamilyConstructorTypes {}));
static_assert(view_aliases<>(kr::dl::CategoryTags {}));

}  // namespace

TEST(RunirKrDlConstructor, PreservesConcreteVariant)
{
    using Concrete = kr::dl::Concept<kr::dl::TopTag>;
    using Constructor = kr::dl::Constructor<kr::dl::ConceptTag>;
    auto planning_repository = tyr::formalism::planning::RepositoryFactory().create_shared();
    auto repository = kr::dl::ExtConstructorRepositoryFactory().create(planning_repository);
    auto data = ygg::Data<Constructor>(ygg::Index<Concrete>(6));
    const auto constructor = repository->insert(data).first;

    ASSERT_TRUE(constructor.get_variant().template is<ygg::Index<Concrete>>());
    EXPECT_EQ(constructor.get_variant().template get<ygg::Index<Concrete>>().get_index(), ygg::Index<Concrete>(6));
}

TEST(RunirKrDlConstructor, SharedInterningCanonicalizesAndPrepares)
{
    const auto check = []()
    {
        auto planning_repository = tyr::formalism::planning::RepositoryFactory().create_shared();
        auto repository = kr::dl::ConstructorRepositoryFactoryFor().create(planning_repository);
        auto top = ygg::Data<kr::dl::Concept<kr::dl::TopTag>>();
        auto bot = ygg::Data<kr::dl::Concept<kr::dl::BotTag>>();
        auto first_data = ygg::Data<kr::dl::Constructor<kr::dl::ConceptTag>>(ygg::formalism::insert(*repository, top).first.get_index());
        auto second_data = ygg::Data<kr::dl::Constructor<kr::dl::ConceptTag>>(ygg::formalism::insert(*repository, bot).first.get_index());
        const auto first = ygg::formalism::insert(*repository, first_data).first;
        const auto second = ygg::formalism::insert(*repository, second_data).first;
        EXPECT_TRUE(first.is_static());
        EXPECT_TRUE(second.is_static());

        auto data = ygg::Data<kr::dl::Concept<kr::dl::IntersectionTag>>(second.get_index(), first.get_index());
        ASSERT_FALSE(kr::dl::is_canonical(data));
        const auto [intersection, created] = ygg::formalism::insert(*repository, data);
        EXPECT_TRUE(created);
        EXPECT_TRUE(kr::dl::is_canonical(data));
        EXPECT_EQ(data.lhs, first.get_index());
        std::swap(data.lhs, data.rhs);
        const auto [same, duplicate_created] = kr::dl::insert(*repository, data);
        EXPECT_FALSE(duplicate_created);
        EXPECT_EQ(same.get_index(), intersection.get_index());
    };
    check();
}

}
