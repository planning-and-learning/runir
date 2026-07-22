#include <concepts>
#include <runir/kr/dl/cnf_grammar/constructor_repository.hpp>
#include <runir/kr/dl/cnf_grammar/numerical_data.hpp>
#include <runir/kr/dl/cnf_grammar/numerical_index.hpp>
#include <runir/kr/dl/cnf_grammar/numerical_view.hpp>
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
    return (IndexedDataView<Ts, kr::dl::cnf_grammar::ConstructorRepositoryFor<Family>> && ...);
}

static_assert(indexed_data_views<kr::BaseFamilyTag>(kr::dl::cnf_grammar::FamilyNumericalTypes<kr::BaseFamilyTag> {}));
static_assert(indexed_data_views<kr::ExtFamilyTag>(kr::dl::cnf_grammar::FamilyNumericalTypes<kr::ExtFamilyTag> {}));
static_assert(indexed_data_views<kr::UnsFamilyTag>(kr::dl::cnf_grammar::FamilyNumericalTypes<kr::UnsFamilyTag> {}));

template<typename Family, typename Tag>
consteval bool numerical_data_view()
{
    using Entity = kr::dl::cnf_grammar::Numerical<Family, Tag>;
    using Data = ygg::Data<Entity>;
    using View = ygg::View<ygg::Index<Entity>, kr::dl::cnf_grammar::ConstructorRepositoryFor<Family>>;

    static_assert(std::same_as<View, kr::dl::cnf_grammar::FamilyNumericalView<Family, Tag>>);
    if constexpr (std::same_as<Family, kr::BaseFamilyTag>)
        static_assert(std::same_as<View, kr::dl::cnf_grammar::BaseNumericalView<Tag>>);

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
        return requires(Data& data) { data.identifier; };
    else if constexpr (kr::dl::TypeListContains<Tag, kr::dl::UnsNumericalBinaryConstructorTags>::value)
        return requires(Data& data) {
            data.lhs;
            data.rhs;
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

}
