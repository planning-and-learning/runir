#include "runir/kr/dl/cnf_grammar/declarations.hpp"

#include <concepts>
#include <gtest/gtest.h>
#include <runir/kr/dl/cnf_grammar/constructor_repository.hpp>
#include <runir/kr/dl/cnf_grammar/numerical_data.hpp>
#include <runir/kr/dl/cnf_grammar/numerical_view.hpp>
#include <tyr/formalism/planning/repository.hpp>
#include <urd/elements/flat.hpp>
#include <boost/mp11.hpp>
#include <yggdrasil/core/config.hpp>

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
{ return (IndexedDataView<Ts, kr::dl::cnf_grammar::ConstructorRepository> && ...); }

static_assert(indexed_data_views<>(kr::dl::cnf_grammar::NumericalTypes {}));

template<typename Tag>
consteval bool numerical_data_view()
{
    using Entity = kr::dl::cnf_grammar::Numerical<Tag>;
    using Data = urd::data<Entity>;
    using View = urd::view<Entity, kr::dl::cnf_grammar::ConstructorRepository>;

    static_assert(std::same_as<View, kr::dl::cnf_grammar::NumericalView<Tag>>);
    if constexpr (false)
        static_assert(std::same_as<View, kr::dl::cnf_grammar::NumericalView<Tag>>);

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

static_assert(numerical_data_views(kr::dl::GenerationNumericalConstructorTags {}));

}  // namespace

TEST(RunirKrDlCnfGrammarNumerical, ExposesDistanceOperands)
{
    auto planning_repository = tyr::formalism::planning::RepositoryFactory().create_shared();
    auto repository = kr::dl::cnf_grammar::ConstructorRepositoryFactory().create(planning_repository);

    auto source_data = urd::data<kr::dl::cnf_grammar::NonTerminal<kr::dl::ConceptTag>>(std::string("source"));
    auto role_data = urd::data<kr::dl::cnf_grammar::NonTerminal<kr::dl::RoleTag>>(std::string("role"));
    auto target_data = urd::data<kr::dl::cnf_grammar::NonTerminal<kr::dl::ConceptTag>>(std::string("target"));
    const auto source = repository->insert(source_data).first;
    const auto role = repository->insert(role_data).first;
    const auto target = repository->insert(target_data).first;
    auto distance_data = urd::data<kr::dl::cnf_grammar::Numerical<kr::dl::DistanceTag>>(source.index(), role.index(), target.index());
    const auto distance = repository->insert(distance_data).first;

    EXPECT_EQ(distance.get_lhs(), source);
    EXPECT_EQ(distance.get_mid(), role);
    EXPECT_EQ(distance.get_rhs(), target);
}
}
