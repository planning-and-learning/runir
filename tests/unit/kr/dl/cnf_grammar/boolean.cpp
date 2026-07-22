#include <concepts>
#include <runir/kr/dl/cnf_grammar/boolean_data.hpp>
#include <runir/kr/dl/cnf_grammar/boolean_index.hpp>
#include <runir/kr/dl/cnf_grammar/boolean_view.hpp>
#include <runir/kr/dl/cnf_grammar/constructor_repository.hpp>
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

static_assert(indexed_data_views<kr::BaseFamilyTag>(kr::dl::cnf_grammar::FamilyBooleanTypes<kr::BaseFamilyTag> {}));
static_assert(indexed_data_views<kr::ExtFamilyTag>(kr::dl::cnf_grammar::FamilyBooleanTypes<kr::ExtFamilyTag> {}));
static_assert(indexed_data_views<kr::UnsFamilyTag>(kr::dl::cnf_grammar::FamilyBooleanTypes<kr::UnsFamilyTag> {}));

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
    using Entity = kr::dl::cnf_grammar::Boolean<Family, Tag>;
    using Data = ygg::Data<Entity>;
    using View = ygg::View<ygg::Index<Entity>, kr::dl::cnf_grammar::ConstructorRepositoryFor<Family>>;

    static_assert(std::same_as<View, kr::dl::cnf_grammar::FamilyBooleanView<Family, Tag>>);
    if constexpr (std::same_as<Family, kr::BaseFamilyTag>)
        static_assert(std::same_as<View, kr::dl::cnf_grammar::BaseBooleanView<Tag>>);

    if constexpr (kr::dl::TypeListContains<Tag, PredicateTags>::value)
        return requires(Data& data, const View& view) {
            data.predicate;
            data.polarity;
            view.get_predicate();
            view.get_polarity();
        };
    else if constexpr (std::same_as<Tag, kr::dl::ArgumentTag<kr::dl::BooleanTag>>)
        return requires(Data& data, const View& view) {
            data.identifier;
            view.get_identifier();
        };
    else if constexpr (std::same_as<Tag, kr::dl::NonemptyTag>)
        return requires(Data& data, const View& view) {
            data.arg;
            view.get_arg();
        };
    else if constexpr (kr::dl::TypeListContains<Tag, kr::dl::UnsComparisonConstructorTags>::value || kr::dl::TypeListContains<Tag, LogicalBinaryTags>::value)
        return requires(Data& data) {
            data.lhs;
            data.rhs;
        };
    else if constexpr (std::same_as<Tag, kr::dl::BooleanConstantTag>)
        return requires(Data& data) { data.identifier; };
    else if constexpr (std::same_as<Tag, kr::dl::NotTag>)
        return requires(Data& data) { data.arg; };
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

}
