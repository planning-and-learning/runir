#include "runir/kr/dl/grammar/declarations.hpp"

#include <concepts>
#include <gtest/gtest.h>
#include <runir/kr/dl/grammar/constructor_repository.hpp>
#include <runir/kr/dl/grammar/numerical_data.hpp>
#include <runir/kr/dl/grammar/numerical_view.hpp>
#include <yggdrasil/core/concepts.hpp>
#include <yggdrasil/core/types.hpp>

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
    return (IndexedDataView<Ts, kr::dl::grammar::ConstructorRepositoryFor<Family>> && ...);
}

static_assert(indexed_data_views<kr::BaseFamilyTag>(kr::dl::grammar::FamilyNumericalTypes<kr::BaseFamilyTag> {}));
static_assert(indexed_data_views<kr::ExtFamilyTag>(kr::dl::grammar::FamilyNumericalTypes<kr::ExtFamilyTag> {}));
static_assert(indexed_data_views<kr::UnsFamilyTag>(kr::dl::grammar::FamilyNumericalTypes<kr::UnsFamilyTag> {}));

template<typename Family, typename Tag>
consteval bool numerical_data_view()
{
    using Entity = kr::dl::grammar::Numerical<Family, Tag>;
    using Data = ygg::Data<Entity>;
    using View = ygg::View<ygg::Index<Entity>, kr::dl::grammar::ConstructorRepositoryFor<Family>>;

    static_assert(std::same_as<View, kr::dl::grammar::FamilyNumericalView<Family, Tag>>);
    if constexpr (std::same_as<Family, kr::BaseFamilyTag>)
        static_assert(std::same_as<View, kr::dl::grammar::BaseNumericalView<Tag>>);

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
            data.identifier;
            view.get_identifier();
        };
    else if constexpr (std::same_as<Tag, kr::dl::NumericalConstantTag>)
        return requires(Data& data, const View& view) {
            data.identifier;
            view.get_value();
        };
    else if constexpr (ygg::InTypeList<Tag, kr::dl::NumericalBinaryConstructorTags>)
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

TEST(RunirKrDlGrammarNumerical, ExposesArithmeticOperandsAndConstants)
{
    namespace dl = kr::dl;
    namespace grammar = dl::grammar;
    using Family = kr::ExtFamilyTag;
    auto planning_repository = tyr::formalism::planning::RepositoryFactory().create_shared();
    auto repository = grammar::ConstructorRepositoryFactoryFor<Family>().create(planning_repository);

    auto lhs_data = ygg::Data<grammar::NonTerminal<Family, dl::NumericalTag>>(std::string("left"));
    auto rhs_data = ygg::Data<grammar::NonTerminal<Family, dl::NumericalTag>>(std::string("right"));
    const auto lhs = repository->insert(lhs_data).first;
    const auto rhs = repository->insert(rhs_data).first;
    auto lhs_choice_data = ygg::Data<grammar::ConstructorOrNonTerminal<Family, dl::NumericalTag>>(lhs.get_index());
    auto rhs_choice_data = ygg::Data<grammar::ConstructorOrNonTerminal<Family, dl::NumericalTag>>(rhs.get_index());
    const auto lhs_choice = repository->insert(lhs_choice_data).first;
    const auto rhs_choice = repository->insert(rhs_choice_data).first;
    auto difference_data = ygg::Data<grammar::Numerical<Family, dl::SubTag>>(lhs_choice.get_index(), rhs_choice.get_index());
    const auto difference = repository->insert(difference_data).first;

    EXPECT_EQ(difference.get_lhs(), lhs_choice);
    EXPECT_EQ(difference.get_rhs(), rhs_choice);
    auto constant_data = ygg::Data<grammar::Numerical<Family, dl::NumericalConstantTag>>(42);
    EXPECT_EQ(repository->insert(constant_data).first.get_value(), 42);
}

}
