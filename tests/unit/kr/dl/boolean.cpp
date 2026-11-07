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
    && urd::Identifiable<urd::index_view<T, Repository>> && std::totally_ordered<urd::index<T>> && std::totally_ordered<urd::data<T>>
    && std::totally_ordered<urd::index_view<T, Repository>> && requires(urd::data<T>& data, const urd::index_view<T, Repository>& view) {
           data.index;
           view.index();
       };

template<typename... Ts>
consteval bool indexed_data_views(boost::mp11::mp_list<Ts...>)
{ return (IndexedDataView<Ts, kr::dl::ConstructorRepository> && ...); }

static_assert(indexed_data_views<>(kr::dl::BooleanTypes {}));

using PredicateTags = boost::mp11::mp_list<kr::dl::AtomicStateTag<tyr::formalism::StaticTag>,
                                    kr::dl::AtomicStateTag<tyr::formalism::FluentTag>,
                                    kr::dl::AtomicStateTag<tyr::formalism::DerivedTag>,
                                    kr::dl::AtomicGoalTag<tyr::formalism::StaticTag>,
                                    kr::dl::AtomicGoalTag<tyr::formalism::FluentTag>,
                                    kr::dl::AtomicGoalTag<tyr::formalism::DerivedTag>>;
using LogicalBinaryTags = boost::mp11::mp_list<kr::dl::AndTag, kr::dl::OrTag>;

template<typename Tag>
consteval bool boolean_data_view()
{
    using Entity = kr::dl::Boolean<Tag>;
    using Data = urd::data<Entity>;
    using View = urd::index_view<Entity, kr::dl::ConstructorRepository>;

    static_assert(std::same_as<View, kr::dl::BooleanView<Tag>>);
    if constexpr (false)
        static_assert(std::same_as<View, kr::dl::BooleanView<Tag>>);
    else if constexpr (true)
        static_assert(std::same_as<View, kr::dl::BooleanView<Tag>>);
    else
        static_assert(std::same_as<View, kr::dl::BooleanView<Tag>>);

    if constexpr (boost::mp11::mp_contains<PredicateTags, Tag>::value)
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
    else if constexpr (boost::mp11::mp_contains<kr::dl::ComparisonConstructorTags, Tag>::value || boost::mp11::mp_contains<LogicalBinaryTags, Tag>::value)
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

template<typename... Tags>
consteval bool boolean_data_views(boost::mp11::mp_list<Tags...>)
{ return (boolean_data_view<Tags>() && ...); }

static_assert(boolean_data_views<>(kr::dl::BooleanConstructorTags {}));

}  // namespace

TEST(RunirKrDlBoolean, RoutesConstantUnaryAndBinaryData)
{
    using Constructor = kr::dl::Constructor<kr::dl::BooleanTag>;
    auto planning_repository = tyr::formalism::planning::RepositoryFactory().create_shared();
    auto repository = kr::dl::ConstructorRepositoryFactory().create(planning_repository);

    auto constant_data = urd::data<kr::dl::Boolean<kr::dl::BooleanConstantTag>>(true);
    const auto constant = repository->insert(constant_data).first;
    EXPECT_TRUE(constant.get_value());

    auto unary_data = urd::data<kr::dl::Boolean<kr::dl::NotTag>>(urd::index<Constructor>(4));
    const auto unary = repository->insert(unary_data).first;
    EXPECT_EQ(unary.get_arg().index(), urd::index<Constructor>(4));

    auto binary_data = urd::data<kr::dl::Boolean<kr::dl::AndTag>>(urd::index<Constructor>(1), urd::index<Constructor>(2));
    const auto binary = repository->insert(binary_data).first;
    EXPECT_EQ(binary.get_lhs().index(), urd::index<Constructor>(1));
    EXPECT_EQ(binary.get_rhs().index(), urd::index<Constructor>(2));
}
}
