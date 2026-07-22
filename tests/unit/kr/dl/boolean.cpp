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

static_assert(indexed_data_views<kr::BaseFamilyTag>(kr::dl::FamilyBooleanTypes<kr::BaseFamilyTag> {}));
static_assert(indexed_data_views<kr::ExtFamilyTag>(kr::dl::FamilyBooleanTypes<kr::ExtFamilyTag> {}));
static_assert(indexed_data_views<kr::UnsFamilyTag>(kr::dl::FamilyBooleanTypes<kr::UnsFamilyTag> {}));

using PredicateTags = ygg::TypeList<kr::dl::AtomicStateTag<tyr::formalism::StaticTag>,
                                    kr::dl::AtomicStateTag<tyr::formalism::FluentTag>,
                                    kr::dl::AtomicStateTag<tyr::formalism::DerivedTag>,
                                    kr::dl::AtomicGoalTag<tyr::formalism::StaticTag>,
                                    kr::dl::AtomicGoalTag<tyr::formalism::FluentTag>,
                                    kr::dl::AtomicGoalTag<tyr::formalism::DerivedTag>>;
using LogicalBinaryTags = ygg::TypeList<kr::dl::AndTag, kr::dl::OrTag>;

template<typename Family, typename Tag>
consteval bool boolean_data_view()
{
    using Entity = kr::dl::Boolean<Family, Tag>;
    using Data = ygg::Data<Entity>;
    using View = ygg::View<ygg::Index<Entity>, kr::dl::ConstructorRepositoryFor<Family>>;

    static_assert(std::same_as<View, kr::dl::FamilyBooleanView<Family, Tag>>);
    if constexpr (std::same_as<Family, kr::BaseFamilyTag>)
        static_assert(std::same_as<View, kr::dl::BaseBooleanView<Tag>>);
    else if constexpr (std::same_as<Family, kr::ExtFamilyTag>)
        static_assert(std::same_as<View, kr::dl::ExtBooleanView<Tag>>);
    else
        static_assert(std::same_as<View, kr::dl::UnsBooleanView<Tag>>);

    if constexpr (kr::dl::TypeListContains<Tag, PredicateTags>::value)
        return requires(Data& data, const View& view) {
            data.predicate;
            data.polarity;
            view.get_predicate();
            view.get_polarity();
        };
    else if constexpr (std::same_as<Tag, kr::dl::ArgumentTag<kr::dl::BooleanTag>>)
        return requires(Data& data, const View& view) {
            data.reference;
            view.get_argument();
        };
    else if constexpr (std::same_as<Tag, kr::dl::NonemptyTag> || std::same_as<Tag, kr::dl::NotTag>)
        return requires(Data& data, const View& view) {
            data.arg;
            view.get_arg();
        };
    else if constexpr (kr::dl::TypeListContains<Tag, kr::dl::UnsComparisonConstructorTags>::value || kr::dl::TypeListContains<Tag, LogicalBinaryTags>::value)
        return requires(Data& data, const View& view) {
            data.lhs;
            data.rhs;
            view.get_lhs();
            view.get_rhs();
        };
    else if constexpr (std::same_as<Tag, kr::dl::BooleanConstantTag>)
        return requires(Data& data, const View& view) {
            data.identifier;
            view.get_value();
        };
    else
        return false;
}

template<typename Family, typename... Tags>
consteval bool boolean_data_views(ygg::TypeList<Tags...>)
{
    return (boolean_data_view<Family, Tags>() && ...);
}

static_assert(boolean_data_views<kr::BaseFamilyTag>(kr::dl::FamilyBooleanConstructorTags<kr::BaseFamilyTag> {}));
static_assert(boolean_data_views<kr::ExtFamilyTag>(kr::dl::FamilyBooleanConstructorTags<kr::ExtFamilyTag> {}));
static_assert(boolean_data_views<kr::UnsFamilyTag>(kr::dl::FamilyBooleanConstructorTags<kr::UnsFamilyTag> {}));

}  // namespace

TEST(RunirKrDlBoolean, RoutesConstantUnaryAndBinaryData)
{
    namespace dl = kr::dl;
    using Family = kr::UnsFamilyTag;
    using Constructor = dl::Constructor<Family, dl::BooleanTag>;
    auto planning_repository = tyr::formalism::planning::RepositoryFactory().create_shared();
    auto repository = dl::UnsConstructorRepositoryFactory().create(planning_repository);

    auto constant_data = ygg::Data<dl::Boolean<Family, dl::BooleanConstantTag>>(true);
    const auto constant = repository->get_or_create(constant_data).first;
    EXPECT_TRUE(constant.get_value());

    auto unary_data = ygg::Data<dl::Boolean<Family, dl::NotTag>>(ygg::Index<Constructor>(4));
    const auto unary = repository->get_or_create(unary_data).first;
    EXPECT_EQ(unary.get_arg().get_index(), ygg::Index<Constructor>(4));

    auto binary_data = ygg::Data<dl::Boolean<Family, dl::AndTag>>(ygg::Index<Constructor>(1), ygg::Index<Constructor>(2));
    const auto binary = repository->get_or_create(binary_data).first;
    EXPECT_EQ(binary.get_lhs().get_index(), ygg::Index<Constructor>(1));
    EXPECT_EQ(binary.get_rhs().get_index(), ygg::Index<Constructor>(2));
}

}
