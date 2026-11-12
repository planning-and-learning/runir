#include <concepts>
#include <gtest/gtest.h>
#include <runir/kr/dl/repository.hpp>
#include <tyr/formalism/planning/repository.hpp>
#include <utility>

namespace runir::tests
{

namespace
{

template<typename T, typename Repository>
concept IndexedDataView = std::constructible_from<urd::index<T>, ygg::uint_t> && urd::Identifiable<urd::index<T>> && urd::Identifiable<urd::data<T>>
                          && urd::Identifiable<urd::index_view<T, Repository>> && std::totally_ordered<urd::index<T>> && std::totally_ordered<urd::data<T>>
                          && std::totally_ordered<urd::index_view<T, Repository>> && requires(urd::data<T>& data, const urd::index_view<T, Repository>& view) {
                                 data.index;
                                 data.variant;
                                 view.index();
                                 view.get_variant();
                             };

template<typename... Ts>
consteval bool indexed_data_views(boost::mp11::mp_list<Ts...>)
{ return (IndexedDataView<Ts, kr::dl::ConstructorRepository> && ...); }

template<typename Category>
consteval bool view_alias()
{
    using Entity = kr::dl::Constructor<Category>;
    using View = urd::index_view<Entity, kr::dl::ConstructorRepository>;
    static_assert(std::same_as<View, kr::dl::ConstructorView<Category>>);
    if constexpr (false)
        return std::same_as<View, kr::dl::ConstructorView<Category>>;
    else if constexpr (true)
        return std::same_as<View, kr::dl::ConstructorView<Category>>;
    else
        return std::same_as<View, kr::dl::ConstructorView<Category>>;
}

template<typename... Categories>
consteval bool view_aliases(boost::mp11::mp_list<Categories...>)
{ return (view_alias<Categories>() && ...); }

template<typename Category>
consteval bool query_projection_contract()
{
    using Entity = kr::dl::QueryProjection<Category>;
    using Repository = kr::dl::ConstructorRepository;
    using View = urd::index_view<Entity, Repository>;
    return urd::resolves<Repository, Entity>&&
        requires(urd::data<Entity> & data, const View& view)
    {
        data.arg;
        data.columns;
        view.get_arg();
        view.get_columns();
    }
    &&!requires(const View& view) { view.get_plan(); };
}

template<typename... Tags>
consteval bool query_view_contracts(boost::mp11::mp_list<Tags...>)
{
    using Repository = kr::dl::ConstructorRepository;
    return urd::resolves<Repository, kr::dl::QueryColumn> && urd::resolves<Repository, kr::dl::Query<>> && query_projection_contract<kr::dl::ConceptTag>()
           && query_projection_contract<kr::dl::RoleTag>() && (urd::resolves<Repository, kr::dl::Query<Tags>> && ...);
}

static_assert(query_view_contracts<>(kr::dl::QueryConstructorTags {}));

static_assert(indexed_data_views<>(kr::dl::ConstructorTypes {}));
static_assert(view_aliases<>(kr::dl::CategoryTags {}));

}  // namespace

TEST(RunirKrDlConstructor, PreservesConcreteVariant)
{
    using Concrete = kr::dl::Concept<kr::dl::SemanticFamily, kr::dl::TopTag>;
    using Constructor = kr::dl::Constructor<kr::dl::ConceptTag>;
    auto planning_repository = std::make_shared<tyr::formalism::planning::Repository>();
    auto repository = kr::dl::ConstructorRepositoryFactory().create(planning_repository);
    auto data = urd::data<Constructor>(urd::index<Concrete>(6));
    const auto constructor = repository->insert(data).first;

    ASSERT_TRUE((std::holds_alternative<urd::index_view<Concrete, runir::kr::dl::ConstructorRepository>>(constructor.get_variant())));
    EXPECT_EQ((std::get<urd::index_view<Concrete, runir::kr::dl::ConstructorRepository>>(constructor.get_variant()).index()), urd::index<Concrete>(6));
}

TEST(RunirKrDlConstructor, SharedInterningCanonicalizesAndPrepares)
{
    const auto check = []()
    {
        auto planning_repository = std::make_shared<tyr::formalism::planning::Repository>();
        auto repository = kr::dl::ConstructorRepositoryFactory().create(planning_repository);
        auto top = urd::data<kr::dl::Concept<kr::dl::SemanticFamily, kr::dl::TopTag>>();
        auto bot = urd::data<kr::dl::Concept<kr::dl::SemanticFamily, kr::dl::BotTag>>();
        auto first_data = urd::data<kr::dl::Constructor<kr::dl::ConceptTag>>(ygg::formalism::insert(*repository, top).first.index());
        auto second_data = urd::data<kr::dl::Constructor<kr::dl::ConceptTag>>(ygg::formalism::insert(*repository, bot).first.index());
        const auto first = ygg::formalism::insert(*repository, first_data).first;
        const auto second = ygg::formalism::insert(*repository, second_data).first;
        EXPECT_TRUE(first.is_static());
        EXPECT_TRUE(second.is_static());

        auto data = urd::data<kr::dl::Concept<kr::dl::SemanticFamily, kr::dl::IntersectionTag>>(second.index(), first.index());
        ASSERT_FALSE(kr::dl::is_canonical(data));
        const auto [intersection, created] = ygg::formalism::insert(*repository, data);
        EXPECT_TRUE(created);
        EXPECT_TRUE(kr::dl::is_canonical(data));
        EXPECT_EQ(data.lhs, first.index());
        std::swap(data.lhs, data.rhs);
        const auto [same, duplicate_created] = kr::dl::insert(*repository, data);
        EXPECT_FALSE(duplicate_created);
        EXPECT_EQ(same.index(), intersection.index());
    };
    check();
}

}
