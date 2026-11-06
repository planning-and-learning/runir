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

static_assert(indexed_data_views<>(kr::dl::NumericalTypes {}));

template<typename Tag>
consteval bool numerical_data_view()
{
    using Entity = kr::dl::Numerical<Tag>;
    using Data = urd::data<Entity>;
    using View = urd::view<Entity, kr::dl::ConstructorRepository>;

    static_assert(std::same_as<View, kr::dl::NumericalView<Tag>>);
    if constexpr (false)
        static_assert(std::same_as<View, kr::dl::NumericalView<Tag>>);
    else if constexpr (true)
        static_assert(std::same_as<View, kr::dl::NumericalView<Tag>>);
    else
        static_assert(std::same_as<View, kr::dl::NumericalView<Tag>>);

    if constexpr (std::same_as<Tag, kr::dl::CountTag>)
        return requires(Data& data, const View& view) {
            data.arg;
            view.get_arg();
        };
    else if constexpr (std::same_as<Tag, kr::dl::DistanceTag>)
        return requires(Data& data, const View& view) {
            data.lhs;
            data.mid;
            data.rhs;
            view.get_lhs();
            view.get_mid();
            view.get_rhs();
        };
    else if constexpr (std::same_as<Tag, kr::dl::ArgumentTag<kr::dl::NumericalTag>>)
        return requires(Data& data, const View& view) {
            data.reference;
            view.get_argument();
        };
    else if constexpr (std::same_as<Tag, kr::dl::NumericalConstantTag>)
        return requires(Data& data, const View& view) {
            data.identifier;
            view.get_value();
        };
    else if constexpr (boost::mp11::mp_contains<kr::dl::NumericalBinaryConstructorTags, Tag>::value)
        return requires(Data& data, const View& view) {
            data.lhs;
            data.rhs;
            view.get_lhs();
            view.get_rhs();
        };
    else
        return false;
}

template<typename... Tags>
consteval bool numerical_data_views(boost::mp11::mp_list<Tags...>)
{ return (numerical_data_view<Tags>() && ...); }

static_assert(numerical_data_views<>(kr::dl::NumericalConstructorTags {}));

}  // namespace

TEST(RunirKrDlNumerical, RoutesValueBinaryAndDistanceData)
{
    using Concept = kr::dl::Constructor<kr::dl::ConceptTag>;
    using Role = kr::dl::Constructor<kr::dl::RoleTag>;
    using Numerical = kr::dl::Constructor<kr::dl::NumericalTag>;
    auto planning_repository = tyr::formalism::planning::RepositoryFactory().create_shared();
    auto repository = kr::dl::ConstructorRepositoryFactory().create(planning_repository);

    auto constant_data = urd::data<kr::dl::Numerical<kr::dl::NumericalConstantTag>>(7);
    EXPECT_EQ(repository->insert(constant_data).first.get_value(), 7);

    auto binary_data = urd::data<kr::dl::Numerical<kr::dl::AddTag>>(urd::index<Numerical>(1), urd::index<Numerical>(2));
    const auto binary = repository->insert(binary_data).first;
    EXPECT_EQ(binary.get_lhs().index(), urd::index<Numerical>(1));
    EXPECT_EQ(binary.get_rhs().index(), urd::index<Numerical>(2));

    // Vertices are concept-or-query and edges role-or-query variants.
    using Query = kr::dl::Query<>;
    using DistanceData = urd::data<kr::dl::Numerical<kr::dl::DistanceTag>>;
    static_assert(std::same_as<DistanceData::Vertex, urd::index_variant<boost::mp11::mp_list<Concept, Query>>>);
    static_assert(std::same_as<DistanceData::Edge, urd::index_variant<boost::mp11::mp_list<Role, Query>>>);
    auto distance_data = DistanceData(urd::index<Concept>(3), urd::index<Query>(4), urd::index<Query>(5));
    const auto distance = repository->insert(distance_data).first;
    EXPECT_EQ((std::get<urd::view<Concept, runir::kr::dl::ConstructorRepository>>(distance.get_lhs()).index()), urd::index<Concept>(3));
    EXPECT_EQ((std::get<urd::view<Query, runir::kr::dl::ConstructorRepository>>(distance.get_mid()).index()), urd::index<Query>(4));
    EXPECT_EQ((std::get<urd::view<Query, runir::kr::dl::ConstructorRepository>>(distance.get_rhs()).index()), urd::index<Query>(5));
    EXPECT_TRUE((std::holds_alternative<urd::view<Query, runir::kr::dl::ConstructorRepository>>(distance.get_mid())));
}
}
