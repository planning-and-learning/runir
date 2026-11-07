#include "runir/kr/dl/grammar/declarations.hpp"

#include <concepts>
#include <runir/kr/dl/grammar/constructor_repository.hpp>
#include <runir/kr/dl/grammar/role_data.hpp>
#include <runir/kr/dl/grammar/role_view.hpp>
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
    && urd::Identifiable<urd::index_view<T, Repository>> && std::totally_ordered<urd::index<T>> && std::totally_ordered<urd::data<T>>
    && std::totally_ordered<urd::index_view<T, Repository>> && requires(urd::data<T>& data, const urd::index_view<T, Repository>& view) {
           data.index;
           view.index();
       };

template<typename... Ts>
consteval bool indexed_data_views(boost::mp11::mp_list<Ts...>)
{ return (IndexedDataView<Ts, kr::dl::grammar::ConstructorRepository> && ...); }

static_assert(indexed_data_views<>(kr::dl::grammar::RoleTypes {}));

using PredicateTags = boost::mp11::mp_list<kr::dl::AtomicStateTag<tyr::formalism::StaticTag>,
                                    kr::dl::AtomicStateTag<tyr::formalism::FluentTag>,
                                    kr::dl::AtomicStateTag<tyr::formalism::DerivedTag>,
                                    kr::dl::AtomicGoalTag<tyr::formalism::StaticTag>,
                                    kr::dl::AtomicGoalTag<tyr::formalism::FluentTag>,
                                    kr::dl::AtomicGoalTag<tyr::formalism::DerivedTag>>;
using BinaryTags = boost::mp11::mp_list<kr::dl::IntersectionTag, kr::dl::UnionTag, kr::dl::CompositionTag, kr::dl::RestrictionTag>;
using UnaryTags =
    boost::mp11::mp_list<kr::dl::ComplementTag, kr::dl::InverseTag, kr::dl::TransitiveClosureTag, kr::dl::ReflexiveTransitiveClosureTag, kr::dl::IdentityTag>;

template<typename Tag>
consteval bool role_data_view()
{
    using Entity = kr::dl::grammar::Role<Tag>;
    using Data = urd::data<Entity>;
    using View = urd::index_view<Entity, kr::dl::grammar::ConstructorRepository>;

    static_assert(std::same_as<View, kr::dl::grammar::RoleView<Tag>>);
    if constexpr (false)
        static_assert(std::same_as<View, kr::dl::grammar::RoleView<Tag>>);

    if constexpr (std::same_as<Tag, kr::dl::UniversalTag>)
        return requires(Data& data, const View& view) {
            data.index;
            view.index();
        };
    else if constexpr (boost::mp11::mp_contains<PredicateTags, Tag>::value)
        return requires(Data& data, const View& view) {
            data.predicate;
            data.polarity;
            view.get_predicate();
            view.get_polarity();
        };
    else if constexpr (boost::mp11::mp_contains<BinaryTags, Tag>::value)
        return requires(Data& data, const View& view) {
            data.lhs;
            data.rhs;
            view.get_lhs();
            view.get_rhs();
        };
    else if constexpr (boost::mp11::mp_contains<UnaryTags, Tag>::value)
        return requires(Data& data, const View& view) {
            data.arg;
            view.get_arg();
        };
    else if constexpr (std::same_as<Tag, kr::dl::RegisterTag> || std::same_as<Tag, kr::dl::ArgumentTag<kr::dl::RoleTag>>)
        return requires(Data& data, const View& view) {
            data.identifier;
            view.get_identifier();
        };
    else
        return false;
}

template<typename... Tags>
consteval bool role_data_views(boost::mp11::mp_list<Tags...>)
{ return (role_data_view<Tags>() && ...); }

static_assert(role_data_views(kr::dl::GenerationRoleConstructorTags {}));

}  // namespace

}
