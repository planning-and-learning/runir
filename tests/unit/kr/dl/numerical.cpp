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

template<typename Family, typename... Ts>
consteval bool indexed_data_views(ygg::TypeList<Ts...>)
{
    return (IndexedDataView<Ts, kr::dl::ConstructorRepositoryFor<Family>> && ...);
}

static_assert(indexed_data_views<kr::BaseFamilyTag>(kr::dl::FamilyNumericalTypes<kr::BaseFamilyTag> {}));
static_assert(indexed_data_views<kr::ExtFamilyTag>(kr::dl::FamilyNumericalTypes<kr::ExtFamilyTag> {}));
static_assert(indexed_data_views<kr::UnsFamilyTag>(kr::dl::FamilyNumericalTypes<kr::UnsFamilyTag> {}));

template<typename Family, typename Tag>
consteval bool numerical_data_view()
{
    using Entity = kr::dl::Numerical<Family, Tag>;
    using Data = ygg::Data<Entity>;
    using View = ygg::View<ygg::Index<Entity>, kr::dl::ConstructorRepositoryFor<Family>>;

    static_assert(std::same_as<View, kr::dl::FamilyNumericalView<Family, Tag>>);
    if constexpr (std::same_as<Family, kr::BaseFamilyTag>)
        static_assert(std::same_as<View, kr::dl::BaseNumericalView<Tag>>);
    else if constexpr (std::same_as<Family, kr::ExtFamilyTag>)
        static_assert(std::same_as<View, kr::dl::ExtNumericalView<Tag>>);
    else
        static_assert(std::same_as<View, kr::dl::UnsNumericalView<Tag>>);

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
    else if constexpr (kr::dl::TypeListContains<Tag, kr::dl::UnsNumericalBinaryConstructorTags>::value)
        return requires(Data& data, const View& view) {
            data.lhs;
            data.rhs;
            view.get_lhs();
            view.get_rhs();
        };
    else
        return false;
}

template<typename Family, typename... Tags>
consteval bool numerical_data_views(ygg::TypeList<Tags...>)
{
    return (numerical_data_view<Family, Tags>() && ...);
}

static_assert(numerical_data_views<kr::BaseFamilyTag>(kr::dl::FamilyNumericalConstructorTags<kr::BaseFamilyTag> {}));
static_assert(numerical_data_views<kr::ExtFamilyTag>(kr::dl::FamilyNumericalConstructorTags<kr::ExtFamilyTag> {}));
static_assert(numerical_data_views<kr::UnsFamilyTag>(kr::dl::FamilyNumericalConstructorTags<kr::UnsFamilyTag> {}));

}  // namespace

TEST(RunirKrDlNumerical, RoutesValueBinaryAndDistanceData)
{
    namespace dl = kr::dl;
    using Family = kr::UnsFamilyTag;
    using Concept = dl::Constructor<Family, dl::ConceptTag>;
    using Role = dl::Constructor<Family, dl::RoleTag>;
    using Numerical = dl::Constructor<Family, dl::NumericalTag>;
    auto planning_repository = tyr::formalism::planning::RepositoryFactory().create_shared();
    auto repository = dl::UnsConstructorRepositoryFactory().create(planning_repository);

    auto constant_data = ygg::Data<dl::Numerical<Family, dl::NumericalConstantTag>>(7);
    EXPECT_EQ(repository->get_or_create(constant_data).first.get_value(), 7);

    auto binary_data = ygg::Data<dl::Numerical<Family, dl::AddTag>>(ygg::Index<Numerical>(1), ygg::Index<Numerical>(2));
    const auto binary = repository->get_or_create(binary_data).first;
    EXPECT_EQ(binary.get_lhs().get_index(), ygg::Index<Numerical>(1));
    EXPECT_EQ(binary.get_rhs().get_index(), ygg::Index<Numerical>(2));

    auto distance_data = ygg::Data<dl::Numerical<Family, dl::DistanceTag>>(ygg::Index<Concept>(3), ygg::Index<Role>(4), ygg::Index<Concept>(5));
    const auto distance = repository->get_or_create(distance_data).first;
    EXPECT_EQ(distance.get_lhs().get_index(), ygg::Index<Concept>(3));
    EXPECT_EQ(distance.get_mid().get_index(), ygg::Index<Role>(4));
    EXPECT_EQ(distance.get_rhs().get_index(), ygg::Index<Concept>(5));
}

}
