#include <concepts>
#include <gtest/gtest.h>
#include <runir/kr/dl/semantics/denotation_repository.hpp>
#include <string>
#include <tyr/formalism/object_data.hpp>
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
    return requires(ygg::Data<T>& data, const ygg::Builder<T>& builder, const ygg::View<ygg::Index<T>, kr::dl::semantics::DenotationRepository>& view) {
        data.value;
        builder.get();
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
static_assert(requires(ygg::Data<Concept>& data, const ygg::Builder<Concept>& builder, const ConceptView& view) {
    data.num_objects;
    data.vec_index;
    builder.get();
    view.get();
    view.begin();
    view.end();
});
using Role = Denotation<kr::dl::RoleTag>;
using RoleView = kr::dl::semantics::DenotationView<kr::dl::RoleTag>;
static_assert(requires(ygg::Data<Role>& data, const ygg::Builder<Role>& builder, const RoleView& view, ygg::Index<tyr::formalism::Object> object) {
    data.num_objects;
    data.vec_index;
    builder.get(object);
    builder.get(ygg::uint_t {});
    builder.get_num_objects();
    builder.any();
    builder.count();
    view.get(object);
    view.get(ygg::uint_t {});
    view.get_num_objects();
    view.any();
    view.count();
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

TEST(RunirKrDlSemanticsDenotation, CountsRolePairs)
{
    namespace dl = kr::dl;
    namespace semantics = dl::semantics;

    auto builder = ygg::Builder<semantics::Denotation<dl::RoleTag>>(3);
    EXPECT_FALSE(builder.any());
    EXPECT_EQ(builder.count(), 0);

    builder.get(0).set(1);
    builder.get(2).set(0);
    EXPECT_TRUE(builder.any());
    EXPECT_EQ(builder.count(), 2);

    auto planning_repository = tyr::formalism::planning::RepositoryFactory().create_shared();
    auto repository = semantics::DenotationRepositoryFactory().create(planning_repository);
    const auto vec_index = repository.get_vector_repository().insert(builder.blocks);
    auto data = ygg::Data<semantics::Denotation<dl::RoleTag>>(builder.get_num_objects(), vec_index);
    const auto view = repository.get_or_create(data).first;

    EXPECT_TRUE(view.any());
    EXPECT_EQ(view.count(), 2);
}

TEST(RunirKrDlSemanticsDenotation, IteratorsOutliveViewWrappers)
{
    namespace dl = kr::dl;
    namespace semantics = dl::semantics;
    auto planning_repository = tyr::formalism::planning::RepositoryFactory().create_shared();
    for (const auto* name : { "a", "b", "c" })
    {
        auto data = ygg::Data<tyr::formalism::Object>(std::string(name));
        (void) planning_repository->get_or_create(data);
    }
    auto repository = semantics::DenotationRepositoryFactory().create(planning_repository);

    auto concept_builder = ygg::Builder<semantics::Denotation<dl::ConceptTag>>(3);
    concept_builder.get().set(0);
    concept_builder.get().set(2);
    auto concept_data = ygg::Data<semantics::Denotation<dl::ConceptTag>>(3, repository.get_vector_repository().insert(concept_builder.blocks));
    const auto concept_view = repository.get_or_create(concept_data).first;
    const auto concept_copy = concept_view;
    EXPECT_TRUE(concept_view.begin() == concept_copy.begin());
    auto concept_iterator = ConceptView(concept_view).begin();  // The temporary wrapper is gone before iteration.
    const auto concept_end = ConceptView(concept_view).end();
    ASSERT_TRUE(concept_iterator != concept_end);
    EXPECT_EQ((*concept_iterator).get_index(), ygg::Index<tyr::formalism::Object>(0));
    ASSERT_TRUE(++concept_iterator != concept_end);
    EXPECT_EQ((*concept_iterator).get_index(), ygg::Index<tyr::formalism::Object>(2));
    EXPECT_TRUE(++concept_iterator == concept_end);

    auto role_builder = ygg::Builder<semantics::Denotation<dl::RoleTag>>(3);
    role_builder.get(0).set(1);
    role_builder.get(2).set(0);
    auto role_data = ygg::Data<semantics::Denotation<dl::RoleTag>>(3, repository.get_vector_repository().insert(role_builder.blocks));
    const auto role_view = repository.get_or_create(role_data).first;
    const auto role_copy = role_view;
    EXPECT_TRUE(role_view.begin() == role_copy.begin());
    auto role_iterator = RoleView(role_view).begin();
    const auto role_end = RoleView(role_view).end();
    ASSERT_TRUE(role_iterator != role_end);
    EXPECT_EQ((*role_iterator).first.get_index(), ygg::Index<tyr::formalism::Object>(0));
    EXPECT_EQ((*role_iterator).second.get_index(), ygg::Index<tyr::formalism::Object>(1));
    ASSERT_TRUE(++role_iterator != role_end);
    EXPECT_EQ((*role_iterator).first.get_index(), ygg::Index<tyr::formalism::Object>(2));
    EXPECT_EQ((*role_iterator).second.get_index(), ygg::Index<tyr::formalism::Object>(0));
    EXPECT_TRUE(++role_iterator == role_end);
}

}
