#include "runir/kr/dl/cnf_grammar/declarations.hpp"

#include <concepts>
#include <runir/kr/dl/cnf_grammar/boolean_data.hpp>
#include <runir/kr/dl/cnf_grammar/boolean_view.hpp>
#include <runir/kr/dl/cnf_grammar/constructor_repository.hpp>
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

static_assert(indexed_data_views<>(kr::dl::cnf_grammar::BooleanTypes {}));

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
    using Entity = kr::dl::cnf_grammar::Boolean<Tag>;
    using Data = urd::data<Entity>;
    using View = urd::view<Entity, kr::dl::cnf_grammar::ConstructorRepository>;

    static_assert(std::same_as<View, kr::dl::cnf_grammar::BooleanView<Tag>>);
    if constexpr (false)
        static_assert(std::same_as<View, kr::dl::cnf_grammar::BooleanView<Tag>>);

    if constexpr (boost::mp11::mp_contains<PredicateTags, Tag>::value)
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

static_assert(boolean_data_views(kr::dl::GenerationBooleanConstructorTags {}));

}  // namespace

}
