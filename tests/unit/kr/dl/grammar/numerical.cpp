#include "runir/kr/dl/grammar/declarations.hpp"

#include <concepts>
#include <gtest/gtest.h>
#include <runir/kr/dl/grammar/constructor_repository.hpp>
#include <runir/kr/dl/grammar/numerical_data.hpp>
#include <runir/kr/dl/grammar/numerical_view.hpp>
#include <tyr/formalism/planning/repository.hpp>
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

TEST(RunirKrDlGrammarNumerical, ExposesDistanceOperands)
{
    using Family = kr::BaseFamilyTag;
    auto planning_repository = tyr::formalism::planning::RepositoryFactory().create_shared();
    auto repository = kr::dl::grammar::ConstructorRepositoryFactoryFor<Family>().create(planning_repository);

    auto source_data = ygg::Data<kr::dl::grammar::NonTerminal<Family, kr::dl::ConceptTag>>(std::string("source"));
    auto role_data = ygg::Data<kr::dl::grammar::NonTerminal<Family, kr::dl::RoleTag>>(std::string("role"));
    auto target_data = ygg::Data<kr::dl::grammar::NonTerminal<Family, kr::dl::ConceptTag>>(std::string("target"));
    auto source_choice_data =
        ygg::Data<kr::dl::grammar::ConstructorOrNonTerminal<Family, kr::dl::ConceptTag>>(repository->insert(source_data).first.get_index());
    auto role_choice_data = ygg::Data<kr::dl::grammar::ConstructorOrNonTerminal<Family, kr::dl::RoleTag>>(repository->insert(role_data).first.get_index());
    auto target_choice_data =
        ygg::Data<kr::dl::grammar::ConstructorOrNonTerminal<Family, kr::dl::ConceptTag>>(repository->insert(target_data).first.get_index());
    const auto source = repository->insert(source_choice_data).first;
    const auto role = repository->insert(role_choice_data).first;
    const auto target = repository->insert(target_choice_data).first;
    auto distance_data = ygg::Data<kr::dl::grammar::Numerical<Family, kr::dl::DistanceTag>>(source.get_index(), role.get_index(), target.get_index());
    const auto distance = repository->insert(distance_data).first;

    EXPECT_EQ(distance.get_lhs(), source);
    EXPECT_EQ(distance.get_mid(), role);
    EXPECT_EQ(distance.get_rhs(), target);
}
}
