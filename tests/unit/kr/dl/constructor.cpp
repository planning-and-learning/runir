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
           data.value;
           data.clear();
           view.get_index();
           view.get_variant();
       };

template<typename Family, typename... Ts>
consteval bool indexed_data_views(ygg::TypeList<Ts...>)
{
    return (IndexedDataView<Ts, kr::dl::ConstructorRepositoryFor<Family>> && ...);
}

template<typename Family, typename Category>
consteval bool view_alias()
{
    using Entity = kr::dl::Constructor<Family, Category>;
    using View = ygg::View<ygg::Index<Entity>, kr::dl::ConstructorRepositoryFor<Family>>;
    static_assert(std::same_as<View, kr::dl::FamilyConstructorView<Family, Category>>);
    if constexpr (std::same_as<Family, kr::BaseFamilyTag>)
        return std::same_as<View, kr::dl::BaseConstructorView<Category>>;
    else if constexpr (std::same_as<Family, kr::ExtFamilyTag>)
        return std::same_as<View, kr::dl::ExtConstructorView<Category>>;
    else
        return std::same_as<View, kr::dl::UnsConstructorView<Category>>;
}

template<typename Family, typename... Categories>
consteval bool view_aliases(ygg::TypeList<Categories...>)
{
    return (view_alias<Family, Categories>() && ...);
}

static_assert(indexed_data_views<kr::BaseFamilyTag>(kr::dl::FamilyConstructorTypes<kr::BaseFamilyTag> {}));
static_assert(indexed_data_views<kr::ExtFamilyTag>(kr::dl::FamilyConstructorTypes<kr::ExtFamilyTag> {}));
static_assert(indexed_data_views<kr::UnsFamilyTag>(kr::dl::FamilyConstructorTypes<kr::UnsFamilyTag> {}));
static_assert(view_aliases<kr::BaseFamilyTag>(kr::dl::CategoryTags {}));
static_assert(view_aliases<kr::ExtFamilyTag>(kr::dl::CategoryTags {}));
static_assert(view_aliases<kr::UnsFamilyTag>(kr::dl::CategoryTags {}));

}  // namespace

TEST(RunirKrDlConstructor, PreservesConcreteVariant)
{
    namespace dl = kr::dl;
    using Family = kr::BaseFamilyTag;
    using Concrete = dl::Concept<Family, dl::TopTag>;
    using Constructor = dl::Constructor<Family, dl::ConceptTag>;
    auto planning_repository = tyr::formalism::planning::RepositoryFactory().create_shared();
    auto repository = dl::BaseConstructorRepositoryFactory().create(planning_repository);
    auto data = ygg::Data<Constructor>(ygg::Index<Concrete>(6));
    const auto constructor = repository->get_or_create(data).first;

    ASSERT_TRUE(constructor.get_variant().template is<ygg::Index<Concrete>>());
    EXPECT_EQ(constructor.get_variant().template get<ygg::Index<Concrete>>().get_index(), ygg::Index<Concrete>(6));
}

}
