#include <concepts>
#include <gtest/gtest.h>
#include <runir/kr/dl/semantics/denotation_repository.hpp>
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
           data.clear();
           view.get_index();
       };

template<typename Repository, typename... Ts>
consteval bool indexed_data_views(ygg::TypeList<Ts...>)
{
    return (IndexedDataView<Ts, Repository> && ...);
}

template<typename Category>
using Denotation = kr::dl::semantics::Denotation<Category>;

using DenotationTypes = ygg::MapTypeListT<Denotation, kr::dl::CategoryTags>;
static_assert(indexed_data_views<kr::dl::semantics::DenotationRepository>(DenotationTypes {}));

using ScalarDenotations = ygg::TypeList<Denotation<kr::dl::BooleanTag>, Denotation<kr::dl::NumericalTag>>;
template<typename T>
consteval bool scalar_denotation()
{
    return requires(ygg::Data<T>& data, const ygg::View<ygg::Index<T>, kr::dl::semantics::DenotationRepository>& view) {
        data.value;
        view.get();
    } && std::same_as<decltype(std::declval<ygg::Data<T>&>().get_data()), decltype((std::declval<ygg::Data<T>&>().value))>
           && std::same_as<decltype(std::declval<const ygg::Data<T>&>().get_data()), decltype((std::declval<const ygg::Data<T>&>().value))>;
}

template<typename... Ts>
consteval bool scalar_denotations(ygg::TypeList<Ts...>)
{
    return (scalar_denotation<Ts>() && ...);
}

template<typename... Categories>
consteval bool view_aliases(ygg::TypeList<Categories...>)
{
    return (std::same_as<kr::dl::semantics::DenotationView<Categories>, ygg::View<ygg::Index<Denotation<Categories>>, kr::dl::semantics::DenotationRepository>>
            && ...);
}

static_assert(scalar_denotations(ScalarDenotations {}));
using Concept = Denotation<kr::dl::ConceptTag>;
using ConceptView = kr::dl::semantics::DenotationView<kr::dl::ConceptTag>;
static_assert(requires(ygg::Data<Concept>& data, const ConceptView& view) {
    data.num_objects;
    data.vec_index;
    view.get();
    view.begin();
    view.end();
});
using Role = Denotation<kr::dl::RoleTag>;
using RoleView = kr::dl::semantics::DenotationView<kr::dl::RoleTag>;
static_assert(requires(ygg::Data<Role>& data, const RoleView& view, ygg::Index<tyr::formalism::Object> object) {
    data.num_objects;
    data.vec_index;
    view.get(object);
    view.begin();
    view.end();
});
static_assert(view_aliases(kr::dl::CategoryTags {}));
static_assert(
    std::same_as<kr::dl::semantics::ConceptDenotationView, ygg::View<ygg::Index<Denotation<kr::dl::ConceptTag>>, kr::dl::semantics::DenotationRepository>>);

}  // namespace

TEST(RunirKrDlSemanticsDenotation, ExposesScalarValues)
{
    namespace dl = kr::dl;
    namespace semantics = dl::semantics;
    auto planning_repository = tyr::formalism::planning::RepositoryFactory().create_shared();
    auto repository = semantics::DenotationRepositoryFactory().create(planning_repository);

    auto boolean_data = ygg::Data<semantics::Denotation<dl::BooleanTag>>(true);
    auto numerical_data = ygg::Data<semantics::Denotation<dl::NumericalTag>>(9);
    EXPECT_TRUE(repository.get_or_create(boolean_data).first.get());
    EXPECT_EQ(repository.get_or_create(numerical_data).first.get(), 9);
}

}
